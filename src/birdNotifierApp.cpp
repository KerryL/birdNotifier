// File:  birdNotifierApp.cpp
// Date:  2/16/2021
// Auth:  K. Loux
// Desc:  Application for regularly notifying subscribers of rare bird sitings.

// Local headers
#include "birdNotifier.h"
#include "birdNotifierConfigFile.h"
#include "email/oAuth2Interface.h"
#include "logging/logger.h"
#include "logging/combinedLogger.h"

// Standard C++ headers
#include <string>
#include <iostream>
#include <memory>

static const UString::String oAuthTokenFileName(_T(".oAuthToken"));

void PrintUsage(const std::string& calledAs)
{
	std::cout << "Usage:  " << calledAs << " <config file name>" << std::endl;
}

bool SetupOAuth2Interface(const EmailConfig& email, UString::OStream& log)
{
	log << "Setting up OAuth2" << std::endl;

	OAuth2Interface::Get().SetLoggingTarget(log);

	OAuth2Interface::Get().SetClientID(UString::ToStringType(email.oAuth2ClientID));
	OAuth2Interface::Get().SetClientSecret(UString::ToStringType(email.oAuth2ClientSecret));
	OAuth2Interface::Get().SetVerboseOutput(false);
	if (!email.caCertificatePath.empty())
		OAuth2Interface::Get().SetCACertificatePath(UString::ToStringType(email.caCertificatePath));

	// Originally, this was for windows only, but device access does not
	// support full access to e-mail.  So the user has some typing to do...
#if 1//#ifdef _WIN32
	OAuth2Interface::Get().SetTokenURL(_T("https://accounts.google.com/o/oauth2/token"));
	OAuth2Interface::Get().SetAuthenticationURL(_T("https://accounts.google.com/o/oauth2/auth"));
	OAuth2Interface::Get().SetResponseType(_T("code"));
	OAuth2Interface::Get().SetRedirectURI(_T("urn:ietf:wg:oauth:2.0:oob"));
	OAuth2Interface::Get().SetLoginHint(UString::ToStringType(email.sender));
	OAuth2Interface::Get().SetGrantType(_T("authorization_code"));
	//OAuth2Interface::Get().SetScope(_T("https://www.googleapis.com/auth/gmail.send"));
	OAuth2Interface::Get().SetScope(_T("https://mail.google.com/"));
#else
	OAuth2Interface::Get().SetTokenURL(_T("https://www.googleapis.com/oauth2/v3/token"));
	OAuth2Interface::Get().SetAuthenticationURL(_T("https://accounts.google.com/o/oauth2/device/code"));
	OAuth2Interface::Get().SetAuthenticationPollURL(_T("https://oauth2.googleapis.com/token"));
	OAuth2Interface::Get().SetGrantType(_T("http://oauth.net/grant_type/device/1.0"));
	OAuth2Interface::Get().SetPollGrantType(_T("urn:ietf:params:oauth:grant-type:device_code"));
	OAuth2Interface::Get().SetScope(_T("email"));
#endif

	// Set the refresh token (one will be created, if this is the first login)
	UString::String oAuth2Token;
	{
		UString::IFStream tokenFile(oAuthTokenFileName);
		if (tokenFile.is_open())// If it's not found, no error since that just means we haven't logged in yet
			std::getline(tokenFile, oAuth2Token);
		else
			log << "Could not open '" << oAuthTokenFileName << "' for input; will request new token..." << std::endl;
	}

	OAuth2Interface::Get().SetRefreshToken(UString::ToStringType(oAuth2Token));
	if (OAuth2Interface::Get().GetRefreshToken() != oAuth2Token)
	{
		oAuth2Token = OAuth2Interface::Get().GetRefreshToken();
		UString::OFStream tokenFile(oAuthTokenFileName);
		if (tokenFile.is_open())
		{
			tokenFile << oAuth2Token;
			log << "Updated OAuth2 refresh token written to " << oAuthTokenFileName << std::endl;
		}
		else
			log << "Failed to write updated OAuth2 refresh token to " << oAuthTokenFileName << std::endl;
	}

	if (OAuth2Interface::Get().GetRefreshToken().empty())
	{
		log << "Failed to obtain refresh token" << std::endl;
		return false;
	}

	return true;
}

static const UString::String logFileName(_T("birdNotifier.log"));

int main(int argc, char* argv[])
{
	UString::OFStream logFile(logFileName);
	CombinedLogger<UString::OStream> logger;
	logger.Add(std::make_unique<Logger>(logFile));
	logger.Add(std::make_unique<Logger>(Cout));
	
	if (argc != 2)
	{
		PrintUsage(argv[0]);
		return 1;
	}

	BirdNotifierConfigFile configFile(logger);
	if (!configFile.ReadConfiguration(UString::ToStringType(argv[1])))
		return 1;

	if (!SetupOAuth2Interface(configFile.GetConfig().emailInfo, logger))
		return 1;

	BirdNotifier birdNotifier(configFile.GetConfig(), logger);
	if (!birdNotifier.Run())
		return 1;

	return 0;
}
