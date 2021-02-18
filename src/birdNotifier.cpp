// File:  birdNotifier.cpp
// Date:  2/16/2021
// Auth:  K. Loux
// Desc:  Object for notifying subscribers of rare bird sitings.

// Local headers
#include "birdNotifier.h"
#include "email/oAuth2Interface.h"

// Standard C++ headers
#include <iostream>
#include <sstream>
#include <filesystem>
#include <iomanip>

bool BirdNotifier::Run()
{
	std::cout << "Reading previously processed observations..." << std::endl;
	std::vector<ReportedObservation> previouslyProcessedObservations;
	if (!ReadPreviousObservations(previouslyProcessedObservations))
		return false;

	std::cout << "Checking for recent observations..." << std::endl;
	EBirdInterface ebi(UString::ToStringType(config.eBirdAPIKey));
	std::vector<EBirdInterface::ObservationInfo> observations;
	if (!ebi.GetRecentNotableObservations(UString::ToStringType(config.regionCode), config.daysBack, observations))
		return false;

	// For some reason, the eBird list of notable sightings tends to include multiple instances of same observation
	observations.erase(std::unique(observations.begin(), observations.end()), observations.end());

	std::cout << "Tailoring observation list..." << std::endl;
	ExcludeSpecies(observations, config.excludeSpecies);
	ExcludeObservations(observations, previouslyProcessedObservations);
	std::cout << "There are " << observations.size() << " new observations" << std::endl;

	if (!observations.empty())
	{
		std::cout << "Sending notifications..." << std::endl;
		if (!SendNotification(observations))
			return false;
	}

	std::cout << "Updating list of previously processed observations..." << std::endl;
	UpdateProcessedObservations(previouslyProcessedObservations, observations);
	if (!WritePreviousObservations(previouslyProcessedObservations))
		return false;

	return true;
}

bool BirdNotifier::ReadPreviousObservations(std::vector<ReportedObservation>& observations)
{
	if (config.alreadyNotifiedFile.empty())
		return true;

	// If the file doesn't exist, don't treat is as an error because it wouldn't have been written yet on first execution of the application
	if (!std::experimental::filesystem::exists(config.alreadyNotifiedFile))
		return true;

	std::ifstream file(config.alreadyNotifiedFile);
	if (!file.is_open())
	{
		std::cerr << "Failed to open '" << config.alreadyNotifiedFile << "' for input\n";
		return false;
	}

	std::string line;
	while (std::getline(file, line))
	{
		ReportedObservation o;
		if (!ParseReportedObservationLine(line, o))
			return false;
		observations.push_back(o);
	}

	return true;
}

bool BirdNotifier::ParseReportedObservationLine(const std::string& line, ReportedObservation& o)
{
	std::istringstream ss(line);
	if (!std::getline(ss, o.observationId, ','))
	{
		std::cerr << "Failed to parse ID from observation line\n";
		return false;
	}

	if (!std::getline(ss, o.observationDate, ','))
	{
		std::cerr << "Failed to parse date from observation line\n";
		return false;
	}

	return true;
}

void BirdNotifier::UpdateProcessedObservations(std::vector<ReportedObservation>& processedObservations, const std::vector<EBirdInterface::ObservationInfo>& observations)
{
	// TODO:  Remove observations from the list if the date is 2*daysBack or greater

	for (const auto& newO : observations)
	{
		ReportedObservation ro;
		ro.observationId = UString::ToNarrowString(newO.observationID);
		ro.observationDate = UString::ToNarrowString(BuildTimeString(newO.observationDate, newO.dateIncludesTimeInfo));
		processedObservations.push_back(ro);
	}
}

bool BirdNotifier::WritePreviousObservations(const std::vector<ReportedObservation>& observations)
{
	if (config.alreadyNotifiedFile.empty())
		return true;

	std::ofstream file(config.alreadyNotifiedFile);
	if (!file.is_open())
	{
		std::cerr << "Failed to open '" << config.alreadyNotifiedFile << "' for output\n";
		return false;
	}

	for (const auto& o : observations)
		file << o.observationId << "," << o.observationDate << '\n';

	return true;
}

bool BirdNotifier::SendNotification(const std::vector<EBirdInterface::ObservationInfo>& observations)
{
	EmailSender::LoginInfo loginInfo;
	std::vector<EmailSender::AddressInfo> recipients;
	BuildEmailEssentials(loginInfo, recipients);
	EmailSender sender("birdNotifier Message", BuildMessageBody(observations), std::string(), recipients, loginInfo, true, false, Cout);
	return sender.Send();
}

std::string BirdNotifier::BuildMessageBody(const std::vector<EBirdInterface::ObservationInfo>& observations)
{
	UString::OStringStream ss;
	for (const auto& o : observations)
		ss << "<p><b>" << o.commonName << "</b> (" << o.count << "), " << BuildTimeString(o.observationDate, o.dateIncludesTimeInfo) << ", " << o.locationName << ", " << o.userName << " -- https://ebird.org/checklist/" << o.checklistID << "</p>";

	return UString::ToNarrowString(ss.str());
}

void BirdNotifier::BuildEmailEssentials(EmailSender::LoginInfo& loginInfo, std::vector<EmailSender::AddressInfo>& recipients)
{
	loginInfo.smtpUrl = "smtp.gmail.com:587";
	loginInfo.localEmail = config.emailInfo.sender;
	loginInfo.oAuth2Token = UString::ToNarrowString(OAuth2Interface::Get().GetRefreshToken());
	loginInfo.useSSL = true;
	loginInfo.caCertificatePath = config.emailInfo.caCertificatePath;

	recipients.resize(config.emailInfo.recipients.size());
	for (unsigned int i = 0; i < recipients.size(); ++i)
	{
		recipients[i].address = config.emailInfo.recipients[i];
		recipients[i].displayName = config.emailInfo.recipients[i];
	}
}

void BirdNotifier::ExcludeSpecies(std::vector<EBirdInterface::ObservationInfo>& observations, const std::vector<std::string>& exclude)
{
	auto nameIsInList([&exclude](const EBirdInterface::ObservationInfo& o) {
		for (const auto& e : exclude)
		{
			if (o.commonName == UString::ToStringType(e))
				return true;
		}
		return false;
	});

	observations.erase(std::remove_if(observations.begin(), observations.end(), nameIsInList), observations.end());
}

void BirdNotifier::ExcludeObservations(std::vector<EBirdInterface::ObservationInfo>& observations, const std::vector<ReportedObservation>& exclude)
{
	auto observationIsInList([&exclude](const EBirdInterface::ObservationInfo& o) {
		for (const auto& e : exclude)
		{
			if (o.observationID == UString::ToStringType( e.observationDate))
				return true;
		}
		return false;
	});

	observations.erase(std::remove_if(observations.begin(), observations.end(), observationIsInList), observations.end());
}

UString::String BirdNotifier::BuildTimeString(const std::tm& dateTime, const bool& includeTime)
{
	UString::OStringStream ss;
	ss << dateTime.tm_mon + 1 << '/' << dateTime.tm_mday << '/' << dateTime.tm_year + 1900;
	if (includeTime)
		ss << ' ' << dateTime.tm_hour << ':' << std::setw(2) << std::setfill(UString::Char('0')) << dateTime.tm_min;
	return ss.str();
}
