// File:  birdNotifierConfigFile.h
// Date:  2/17/2021
// Auth:  K. Loux
// Desc:  Configuration file object.

#ifndef BIRD_NOTIFIER_CONFIG_FILE_H_
#define BIRD_NOTIFIER_CONFIG_FILE_H_

// Local headers
#include "utilities/configFile.h"
#include "utilities/uString.h"
#include "birdNotifierConfig.h"

class BirdNotifierConfigFile : public ConfigFile
{
public:
	BirdNotifierConfigFile(UString::OStream &outStream = Cout)
		: ConfigFile(outStream) {}

	BirdNotifierConfig& GetConfig() { return config; }

private:
	BirdNotifierConfig config;

	void BuildConfigItems() override;
	void AssignDefaults() override;
	bool ConfigIsOK() override;
};

#endif// BIRD_NOTIFIER_CONFIG_FILE_H_
