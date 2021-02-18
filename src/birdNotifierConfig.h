// File:  birdNotifierConfig.h
// Date:  2/16/2021
// Auth:  K. Loux
// Desc:  Configuration options.

#ifndef BIRD_NOTIFIER_CONFIG_H_
#define BIRD_NOTIFIER_CONFIG_H_

// Local headers
#include <string>
#include <vector>

struct EmailConfig
{
	std::string sender;
	std::vector<std::string> recipients;

	std::string oAuth2ClientID;
	std::string oAuth2ClientSecret;
	std::string caCertificatePath;
};

struct BirdNotifierConfig
{
	std::string alreadyNotifiedFile;

	std::string eBirdAPIKey;
	std::string regionCode;

	std::vector<std::string> excludeSpecies;
	unsigned int daysBack;

	EmailConfig emailInfo;
};

#endif// BIRD_NOTIFIER_CONFIG_H_
