// File:  eBirdInterface.h
// Date:  8/22/2017
// Auth:  K. Loux
// Desc:  Interface to eBird web API.  See https://confluence.cornell.edu/display/CLOISAPI/eBird+API+1.1.

#ifndef EBIRD_INTERFACE_H_
#define EBIRD_INTERFACE_H_

// Local headers
#include "utilities/uString.h"
#include "email/jsonInterface.h"

// Standard C++ headers
#include <vector>
#include <unordered_map>
#include <ctime>

class EBirdInterface : public JSONInterface
{
public:
	EBirdInterface(const UString::String& apiKey, UString::OStream& log = Cout) : tokenData(apiKey), log(log) {}

	struct ObservationInfo
	{
		UString::String speciesCode;
		UString::String commonName;
		UString::String scientificName;
		std::tm observationDate;
		bool presenceNoted;
		unsigned int count;
		UString::String locationID;
		bool isNotHotspot;
		UString::String locationName;
		double latitude;
		double longitude;
		bool observationReviewed;
		bool observationValid;
		bool locationPrivate;
		double distance = 0.0;// [km]
		unsigned int duration = 0;// [min]
		bool hasMedia;
		UString::String comments;
		UString::String observationID;
		UString::String checklistID;
		UString::String userName;

		bool dateIncludesTimeInfo = true;

		bool operator==(const ObservationInfo& o);
	};

	bool GetRecentNotableObservations(const UString::String& regionCode, const unsigned int& daysBack, std::vector<ObservationInfo>& observations);

private:
	static const UString::String apiRoot;
	static const UString::String observationDataPath;
	static const UString::String recentNotableEndPoint;

	static const UString::String speciesCodeTag;
	static const UString::String commonNameTag;
	static const UString::String scientificNameTag;
	static const UString::String locationIDTag;
	static const UString::String locationNameTag;
	static const UString::String howManyTag;
	static const UString::String presenceNotedTag;
	static const UString::String latitudeTag;
	static const UString::String longitudeTag;
	static const UString::String countryCodeTag;
	static const UString::String subnational1CodeTag;
	static const UString::String subnational2CodeTag;
	static const UString::String observationDateTag;
	static const UString::String isNotHotspotTag;
	static const UString::String isReviewedTag;
	static const UString::String isValidTag;
	static const UString::String locationPrivateTag;
	static const UString::String userDisplayNameTag;
	static const UString::String submissionIDTag;
	static const UString::String speciesCountTag;
	static const UString::String observationTimeTag;
	static const UString::String hasCommentsTag;
	static const UString::String commentsTag;
	static const UString::String hasMediaTag;
	static const UString::String observationIDTag;

	static const UString::String nameTag;
	static const UString::String codeTag;
	static const UString::String resultTag;

	static const UString::String errorTag;
	static const UString::String titleTag;
	static const UString::String statusTag;

	static const UString::String eBirdTokenHeader;

	bool ReadJSONObservationData(cJSON* item, ObservationInfo& info);

	struct TokenData : public ModificationData
	{
		TokenData(const UString::String& token) : token(token) {}
		UString::String token;
	};

	const TokenData tokenData;
	UString::OStream& log;

	static bool AddTokenToCurlHeader(CURL* curl, const ModificationData* data);// Expects TokenData

	struct ErrorInfo
	{
		UString::String title;
		UString::String code;
		UString::String status;
	};

	bool ResponseHasErrors(cJSON *root, std::vector<ErrorInfo>& errors);
	void PrintErrorInfo(const std::vector<ErrorInfo>& errors);
};

#endif// EBIRD_INTERFACE_H_
