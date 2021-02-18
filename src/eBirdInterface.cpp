// File:  eBirdInterface.cpp
// Date:  8/22/2017
// Auth:  K. Loux
// Desc:  Interface to eBird web API.  See https://confluence.cornell.edu/display/CLOISAPI/eBird+API+1.1.

// Local headers
#include "eBirdInterface.h"
#include "email/cJSON/cJSON.h"
#include "email/curlUtilities.h"

// Standard C++ headers
#include <cctype>
#include <algorithm>
#include <map>
#include <cassert>
#include <iomanip>

const UString::String EBirdInterface::apiRoot(_T("https://api.ebird.org/v2/"));
const UString::String EBirdInterface::observationDataPath(_T("data/obs/"));
const UString::String EBirdInterface::recentNotableEndPoint(_T("/recent/notable"));

const UString::String EBirdInterface::speciesCodeTag(_T("speciesCode"));
const UString::String EBirdInterface::commonNameTag(_T("comName"));
const UString::String EBirdInterface::scientificNameTag(_T("sciName"));
const UString::String EBirdInterface::locationNameTag(_T("locName"));
const UString::String EBirdInterface::userDisplayNameTag(_T("userDisplayName"));
const UString::String EBirdInterface::locationIDTag(_T("locID"));
const UString::String EBirdInterface::submissionIDTag(_T("subID"));
const UString::String EBirdInterface::latitudeTag(_T("lat"));
const UString::String EBirdInterface::longitudeTag(_T("lng"));
const UString::String EBirdInterface::howManyTag(_T("howMany"));
const UString::String EBirdInterface::countryCodeTag(_T("countryCode"));
const UString::String EBirdInterface::subnational1CodeTag(_T("subnational1Code"));
const UString::String EBirdInterface::subnational2CodeTag(_T("subnational2Code"));
const UString::String EBirdInterface::observationDateTag(_T("obsDt"));
const UString::String EBirdInterface::observationTimeTag(_T("obsTime"));
const UString::String EBirdInterface::isReviewedTag(_T("obsReviewed"));
const UString::String EBirdInterface::isValidTag(_T("obsValid"));
const UString::String EBirdInterface::locationPrivateTag(_T("locationPrivate"));
const UString::String EBirdInterface::hasCommentsTag(_T("hasComments"));
const UString::String EBirdInterface::commentsTag(_T("comments"));// TODO:  Unverified; seems to always return false even if comments were submitted (2/18/2021)
const UString::String EBirdInterface::hasMediaTag(_T("hasRichMedia"));
const UString::String EBirdInterface::observationIDTag(_T("obsId"));

const UString::String EBirdInterface::nameTag(_T("name"));
const UString::String EBirdInterface::codeTag(_T("code"));
const UString::String EBirdInterface::resultTag(_T("result"));

const UString::String EBirdInterface::errorTag(_T("errors"));
const UString::String EBirdInterface::titleTag(_T("title"));
const UString::String EBirdInterface::statusTag(_T("status"));

const UString::String EBirdInterface::eBirdTokenHeader(_T("X-eBirdApiToken: "));

bool EBirdInterface::GetRecentNotableObservations(const UString::String& regionCode, const unsigned int& daysBack, std::vector<ObservationInfo>& observations)
{
	UString::OStringStream request;
	request << apiRoot << observationDataPath << regionCode << recentNotableEndPoint << "?back=" << daysBack << "&detail=full";

	std::string response;
	if (!DoCURLGet(URLEncode(request.str()), response, AddTokenToCurlHeader, &tokenData))
		return false;

	cJSON *root(cJSON_Parse(response.c_str()));
	if (!root)
	{
		Cerr << "Failed to parse returned string (GetRecentNotableObservations())\n";
		Cerr << response.c_str() << '\n';
		return false;
	}

	std::vector<ErrorInfo> errorInfo;
	if (ResponseHasErrors(root, errorInfo))
	{
		PrintErrorInfo(errorInfo);
		return false;
	}

	observations.resize(cJSON_GetArraySize(root));
	unsigned int i(0);
	for (auto& o : observations)
	{
		cJSON* item(cJSON_GetArrayItem(root, i));
		if (!item)
		{
			Cerr << "Failed to get observation array item\n";
			cJSON_Delete(root);
			return false;
		}

		if (!ReadJSONObservationData(item, o))
		{
			cJSON_Delete(root);
			return false;
		}

		++i;
	}

	return true;
}

void EBirdInterface::PrintErrorInfo(const std::vector<ErrorInfo>& errors)
{
	for (const auto& e : errors)
		Cout << "Error " << e.code << " : " << e.title << " : " << e.status << std::endl;
}

bool EBirdInterface::ReadJSONObservationData(cJSON* item, ObservationInfo& info)
{
	if (!ReadJSON(item, speciesCodeTag, info.speciesCode))
	{
		Cerr << "Failed to get species code\n";
		return false;
	}

	if (!ReadJSON(item, commonNameTag, info.commonName))
	{
		Cerr << "Failed to get common name for item\n";
		return false;
	}

	if (!ReadJSON(item, scientificNameTag, info.scientificName))
	{
		Cerr << "Failed to get scientific name for item\n";
		return false;
	}

	if (!ReadJSON(item, locationIDTag, info.locationID))
	{
		Cerr << "Failed to get location id for item\n";
		return false;
	}

	if (!ReadJSON(item, locationNameTag, info.locationName))
	{
		Cerr << "Failed to get location name for item\n";
		return false;
	}

	if (!ReadJSON(item, observationDateTag, info.observationDate))
	{
		// Try without time info before declaring a failure
		UString::String dateString;
		if (!ReadJSON(item, observationDateTag, dateString))
		{
			Cerr << "Failed to get observation date and time for item\n";
			return false;
		}

		UString::IStringStream ss(dateString);
		if ((ss >> std::get_time(&info.observationDate, _T("%Y-%m-%d"))).fail())
		{
			Cerr << "Failed to get observation date for item\n";
			return false;
		}

		info.dateIncludesTimeInfo = false;
	}

	if (!ReadJSON(item, howManyTag, info.count))
	{
		Cerr << "Failed to get observation count\n";
		return false;
	}

	if (!ReadJSON(item, latitudeTag, info.latitude))
	{
		Cerr << "Failed to get location latitude for item\n";
		return false;
	}

	if (!ReadJSON(item, longitudeTag, info.longitude))
	{
		Cerr << "Failed to get location longitude for item\n";
		return false;
	}

	if (!ReadJSON(item, isValidTag, info.observationValid))
	{
		Cerr << "Failed to get observation valid flag for item\n";
		return false;
	}

	if (!ReadJSON(item, isReviewedTag, info.observationReviewed))
	{
		Cerr << "Failed to get observation reviewed flag for item\n";
		return false;
	}

	if (!ReadJSON(item, locationPrivateTag, info.locationPrivate))
	{
		Cerr << "Failed to get location private flag\n";
		return false;
	}

	if (!ReadJSON(item, submissionIDTag, info.checklistID))
	{
		Cerr << "Failed to get submission ID\n";
		return false;
	}

	if (!ReadJSON(item, userDisplayNameTag, info.userName))
	{
		Cerr << "Failed to get user name\n";
		return false;
	}

	if (!ReadJSON(item, observationIDTag, info.observationID))
	{
		Cerr << "Failed to get observation ID\n";
		return false;
	}

	bool hasComments;
	if (!ReadJSON(item, hasCommentsTag, hasComments))
	{
		Cerr << "Failed to get has comments flag\n";
		return false;
	}

	if (hasComments)
	{
		if (!ReadJSON(item, commentsTag, info.comments))
		{
			Cerr << "Failed to get comments\n";
			return false;
		}
	}

	if (!ReadJSON(item, hasMediaTag, info.hasMedia))
	{
		Cerr << "Failed to get has media flag\n";
		return false;
	}

	return true;
}

bool EBirdInterface::AddTokenToCurlHeader(CURL* curl, const ModificationData* data)
{
	curl_slist* headerList(nullptr);
	headerList = curl_slist_append(headerList, UString::ToNarrowString(UString::String(eBirdTokenHeader
		+ static_cast<const TokenData*>(data)->token)).c_str());
	if (!headerList)
	{
		Cerr << "Failed to append token to header in AddTokenToCurlHeader\n";
		return false;
	}

	headerList = curl_slist_append(headerList, "Content-Type: application/json");
	if (!headerList)
	{
		Cerr << "Failed to append content type to header in ListTables\n";
		return false;
	}

	if (CURLUtilities::CURLCallHasError(curl_easy_setopt(curl,
		CURLOPT_HTTPHEADER, headerList), _T("Failed to set header")))
		return false;

	return true;
}

bool EBirdInterface::ResponseHasErrors(cJSON *root, std::vector<ErrorInfo>& errors)
{
	cJSON* errorsNode(cJSON_GetObjectItem(root, UString::ToNarrowString(errorTag).c_str()));
	if (!errorsNode)
		return false;

	errors.resize(cJSON_GetArraySize(errorsNode));
	unsigned int i(0);
	for (auto& e : errors)
	{
		cJSON* item(cJSON_GetArrayItem(errorsNode, i++));
		if (!item)
		{
			Cerr << "Failed to read error " << i << '\n';
			return true;
		}

		if (!ReadJSON(item, codeTag, e.code))
		{
			Cerr << "Failed to read error code\n";
			break;
		}

		if (!ReadJSON(item, statusTag, e.status))
		{
			Cerr << "Failed to read error status\n";
			break;
		}

		if (!ReadJSON(item, titleTag, e.title))
		{
			Cerr << "Failed to read error title\n";
			break;
		}
	}

	return true;
}

bool EBirdInterface::ObservationInfo::operator==(const ObservationInfo& o)
{
	return observationID == o.observationID;
}
