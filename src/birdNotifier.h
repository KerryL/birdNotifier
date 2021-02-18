// File:  birdNotifier.h
// Date:  2/16/2021
// Auth:  K. Loux
// Desc:  Object for notifying subscribers of rare bird sitings.

#ifndef BIRD_NOTIFIER_H_
#define BIRD_NOTIFIER_H_

// Local headers
#include "birdNotifierConfig.h"
#include "eBirdInterface.h"
#include "email/emailSender.h"

class BirdNotifier
{
public:
	explicit BirdNotifier(const BirdNotifierConfig& config) : config(config) {}
	bool Run();

private:
	const BirdNotifierConfig config;

	struct ReportedObservation
	{
		std::string observationId;
		std::string observationDate;
	};

	bool ReadPreviousObservations(std::vector<ReportedObservation>& observations);
	bool ParseReportedObservationLine(const std::string& line, ReportedObservation& o);
	void UpdateProcessedObservations(std::vector<ReportedObservation>& processedObservations, const std::vector<EBirdInterface::ObservationInfo>& observations);
	bool WritePreviousObservations(const std::vector<ReportedObservation>& observations);

	bool SendNotification(const std::vector<EBirdInterface::ObservationInfo>& observations);
	void BuildEmailEssentials(EmailSender::LoginInfo& loginInfo, std::vector<EmailSender::AddressInfo>& recipients);
	std::string BuildMessageBody(const std::vector<EBirdInterface::ObservationInfo>& observations);

	static void ExcludeSpecies(std::vector<EBirdInterface::ObservationInfo>& observations, const std::vector<std::string>& exclude);
	static void ExcludeObservations(std::vector<EBirdInterface::ObservationInfo>& observations, const std::vector<ReportedObservation>& exclude);

	static UString::String BuildTimeString(const std::tm& dateTime, const bool& includeTime);
};

#endif// BIRD_NOTIFIER_H_
