// File:  birdNotifierConfigFile.cpp
// Date:  2/17/2021
// Auth:  K. Loux
// Desc:  Configuration file object.

// Local headers
#include "birdNotifierConfigFile.h"

void BirdNotifierConfigFile::BuildConfigItems()
{
	AddConfigItem(_T("PREVIOUS_NOTIFICATION_FILE"), config.alreadyNotifiedFile);

	AddConfigItem(_T("EBIRD_API_KEY"), config.eBirdAPIKey);
	AddConfigItem(_T("REGION_CODE"), config.regionCode);

	AddConfigItem(_T("EXCLUDE"), config.excludeSpecies);
	AddConfigItem(_T("DAYS_BACK"), config.daysBack);

	AddConfigItem(_T("SENDER"), config.emailInfo.sender);
	AddConfigItem(_T("RECIPIENT"), config.emailInfo.recipients);

	AddConfigItem(_T("OAUTH_CLIENT_ID"), config.emailInfo.oAuth2ClientID);
	AddConfigItem(_T("OAUTH_CLIENT_SECRET"), config.emailInfo.oAuth2ClientSecret);
	AddConfigItem(_T("CA_CERT_PATH"), config.emailInfo.caCertificatePath);
}

void BirdNotifierConfigFile::AssignDefaults()
{
	config.alreadyNotifiedFile = ".previouslyNotified";
	config.daysBack = 2;
}

bool BirdNotifierConfigFile::ConfigIsOK()
{
	bool configurationOK(true);

	if (config.eBirdAPIKey.empty())
	{
		Cerr << GetKey(config.eBirdAPIKey) << " must be specified" << '\n';
		configurationOK = false;
	}

	if (config.regionCode.empty())
	{
		Cerr << GetKey(config.regionCode) << " must be specified" << '\n';
		configurationOK = false;
	}

	if (config.daysBack == 0)
	{
		Cerr << GetKey(config.daysBack) << " must be strictly positive" << '\n';
		configurationOK = false;
	}

	if (config.emailInfo.sender.empty())
	{
		Cerr << GetKey(config.emailInfo.sender) << " must be specified" << '\n';
		configurationOK = false;
	}

	if (config.emailInfo.recipients.empty())
	{
		Cerr << GetKey(config.emailInfo.recipients) << " must be specified" << '\n';
		configurationOK = false;
	}

	if (config.emailInfo.oAuth2ClientID.empty())
	{
		Cerr << GetKey(config.emailInfo.oAuth2ClientID) << " must be specified" << '\n';
		configurationOK = false;
	}

	if (config.emailInfo.oAuth2ClientSecret.empty())
	{
		Cerr << GetKey(config.emailInfo.oAuth2ClientSecret) << " must be specified" << '\n';
		configurationOK = false;
	}

	return configurationOK;
}
