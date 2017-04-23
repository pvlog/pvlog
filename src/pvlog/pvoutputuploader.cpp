#define PVLOG_LOG_MODULE "pvoutputuploader"

#include "pvoutputuploader.h"

#include <exception>

#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPMessage.h>
#include <Poco/Net/HTMLForm.h>

#include "log.h"
#include "models/configservice.h"

using Poco::Net::HTTPRequest;
using Poco::Net::HTTPMessage;
using Poco::Net::HTMLForm;
using Poco::Net::HTTPClientSession;
using Poco::Net::HTTPResponse;

namespace bg = boost::gregorian;
namespace pt = boost::posix_time;

using model::SpotData;
using model::DayData;

static const std::string LIVE_DATA_PATH = "/r2/addstatus.jsp";
static const std::string DAY_DATA_PATH = "/service/r2/addoutput.jsp";
static const std::string PVOUTPUT_URI = "https://pvoutput.org";
static const int PVOUTPUT_PORT = 80;

void readIdApiKey(odb::database* db, std::string& id, std::string& apiKey) {
	try {
		id     = readConfig(db, "pvoutputId");
		apiKey = readConfig(db, "pvoutputApikey");
	} catch (const std::exception& ex) {
		LOG(Debug) << "uploadLiveData to pvoutput.ord disabled!";
		id = "";
		apiKey = "";
	}
}

PvoutputUploader::PvoutputUploader(odb::database* db) : db(db) {
	//nothing to do
}

void PvoutputUploader::uploadSpotDataSum(pt::ptime datetime, int32_t power,
		const std::string& systemId, const std::string& apiKey) {
	LOG(Debug) << "Uploading live data. Time: " << datetime << " power: " << power << "W";

	HTTPRequest request(HTTPRequest::HTTP_POST, LIVE_DATA_PATH, HTTPMessage::HTTP_1_1);

	request.add("X-Pvoutput-SystemId", systemId);
	request.add("X-Pvoutput-Apikey", apiKey);

	HTMLForm form;
	form.add("d", bg::to_iso_string(datetime.date()));
	int hours   = datetime.time_of_day().hours();
	int minutes = datetime.time_of_day().minutes();
	form.add("t", std::to_string(hours) + ":" + std::to_string(minutes));
	form.add("v2", std::to_string(power));
	form.prepareSubmit(request);

	HTTPClientSession session(PVOUTPUT_URI, PVOUTPUT_PORT);
	std::ostream& send = session.sendRequest(request);
	form.write(send);

	HTTPResponse response;
	session.receiveResponse(response);

	if (response.getStatus() != HTTPResponse::HTTP_OK) {
		LOG(Error) << "Error sending live power data. HTTP error: " << response.getStatus();
	}
}

void PvoutputUploader::uploadDayDataSum(bg::date date, int32_t yield,
		const std::string& systemId, const std::string& apiKey) {
	LOG(Debug) << "Uploading day energy data. Date: " << date<< " yield: " << yield << "Wh";

	HTTPRequest request(HTTPRequest::HTTP_POST, DAY_DATA_PATH, HTTPMessage::HTTP_1_1);

	request.add("X-Pvoutput-SystemId", systemId);
	request.add("X-Pvoutput-Apikey", apiKey);

	HTMLForm form;
	form.add("d", bg::to_iso_string(date));
	form.add("g", std::to_string(yield));
	form.prepareSubmit(request);

	HTTPClientSession session(PVOUTPUT_URI, PVOUTPUT_PORT);
	std::ostream& send = session.sendRequest(request);
	form.write(send);

	HTTPResponse response;
	session.receiveResponse(response);

	if (response.getStatus() != HTTPResponse::HTTP_OK) {
		LOG(Error) << "Error sending day yield data. HTTP error: " << response.getStatus();
	}
}

void PvoutputUploader::uploadSpotData(const std::vector<SpotData>& spotDatas) {
	std::string id;
	std::string apiKey;

	readIdApiKey(db, id, apiKey);
	if (id == "" || apiKey == "") {
		return;
	}

	int32_t power = 0;
	for (const SpotData& sd : spotDatas) {
		power += sd.power;
	}

	uploadSpotDataSum(spotDatas.begin()->time, power, id, apiKey);
}

void PvoutputUploader::uploadDayData(const std::vector<DayData>& dayDatas) {
	std::string id;
	std::string apiKey;

	readIdApiKey(db, id, apiKey);
	if (id == "" || apiKey == "") {
		return;
	}

	int32_t yield = 0;
	for (const DayData& sd : dayDatas) {
		yield += sd.dayYield;
	}

	uploadDayDataSum(dayDatas.begin()->date, yield, id, apiKey);
}
