/*
 * This file is part of Pvlog.
 *
 * Copyright (C) 2017 pvlogdev@gmail.com
 *
 * Pvlog is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Pvlog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Pvlog.  If not, see <http://www.gnu.org/licenses/>.
 */

#define PVLOG_LOG_MODULE "pvoutputuploader"

#include "pvoutputuploader.h"

#include <exception>

#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPMessage.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/Net/NetException.h>
#include <Poco/Net/SSLManager.h>
#include <Poco/Net/AcceptCertificateHandler.h>

#include "log.h"
#include "models/configservice.h"

using Poco::Net::HTTPRequest;
using Poco::Net::HTTPMessage;
using Poco::Net::HTMLForm;
using Poco::Net::HTTPSClientSession;
using Poco::Net::HTTPResponse;
using Poco::SharedPtr;
using Poco::Net::InvalidCertificateHandler;
using Poco::Net::Context;
using Poco::Net::SSLManager;
using Poco::Net::AcceptCertificateHandler;

namespace bg = boost::gregorian;
namespace pt = boost::posix_time;

using model::SpotData;
using model::DayData;

static const std::string LIVE_DATA_PATH = "/service/r2/addstatus.jsp";
static const std::string DAY_DATA_PATH = "/service/r2/addoutput.jsp";
static const std::string PVOUTPUT_HOST = "pvoutput.org";
static const int PVOUTPUT_PORT = 443;

void readIdApiKey(odb::database* db, std::string& id, std::string& apiKey) {
	try {
		id     = readConfig(db, "pvoutputSystemId");
		apiKey = readConfig(db, "pvoutputApiKey");
	} catch (const std::exception& ex) {
		LOG(Debug) << "uploadLiveData to pvoutput.ord disabled!";
		id = "";
		apiKey = "";
	}
}

PvoutputUploader::PvoutputUploader(odb::database* db) : db(db) {
	SharedPtr<InvalidCertificateHandler> ptrHandler =
			new AcceptCertificateHandler(false);

	ptrContext = new Context(Context::CLIENT_USE, "", "", "",
			Context::VERIFY_RELAXED, 9, true,
			"ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");

	SSLManager::instance().initializeClient(0, ptrHandler, ptrContext);
}

void PvoutputUploader::uploadSpotDataSum(pt::ptime datetime, int32_t power, int32_t yield,
		const std::string& systemId, const std::string& apiKey) {
	LOG(Debug) << "Uploading live data. Time: " << datetime << " power: "
			<< power << "W, dayYield: " << yield << "Wh";

	HTTPRequest request(HTTPRequest::HTTP_POST, LIVE_DATA_PATH, HTTPMessage::HTTP_1_1);

	request.add("X-Pvoutput-SystemId", systemId);
	request.add("X-Pvoutput-Apikey", apiKey);

	HTMLForm form;
	form.add("d", bg::to_iso_string(datetime.date()));
	int hours   = datetime.time_of_day().hours();
	int minutes = datetime.time_of_day().minutes();
	form.add("t", std::to_string(hours) + ":" + std::to_string(minutes));
	if (yield >= 0) {
		form.add("v1", std::to_string(yield));
	}
	form.add("v2", std::to_string(power));
	form.prepareSubmit(request);

	HTTPSClientSession session(PVOUTPUT_HOST, PVOUTPUT_PORT, ptrContext);
	std::ostream& send = session.sendRequest(request);
	form.write(send);

	HTTPResponse response;
	std::istream& rs = session.receiveResponse(response);

	if (response.getStatus() != HTTPResponse::HTTP_OK) {
		LOG(Error) << "Error sending live power data. HTTP error: " << response.getStatus()
				<< " " << rs.rdbuf();
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

	HTTPSClientSession session(PVOUTPUT_HOST, PVOUTPUT_PORT, ptrContext);
	std::ostream& send = session.sendRequest(request);
	form.write(send);

	HTTPResponse response;
	std::istream& rs = session.receiveResponse(response);

	if (response.getStatus() != HTTPResponse::HTTP_OK) {
		LOG(Error) << "Error sending day yield data. HTTP error: " << response.getStatus()
				<< " " << rs.rdbuf();
	}
}

void PvoutputUploader::uploadSpotData(const std::vector<SpotData>& spotDatas) {
	std::string id;
	std::string apiKey;

	assert(!spotDatas.empty());

	readIdApiKey(db, id, apiKey);
	if (id == "" || apiKey == "") {
		return;
	}

	int32_t power = 0;
	int32_t yield = 0;
	for (const SpotData& sd : spotDatas) {
		power += sd.power;
		if (sd.dayYield) {
			if (yield >= 0) {
				yield += *sd.dayYield;
			}
		} else {
			yield = -1;
		}
	}
	try {
		uploadSpotDataSum(spotDatas.begin()->time, power, yield, id, apiKey);
	} catch (const std::exception& ex) {
		LOG(Error) << "Uploading spot data to pvoutput failed: " << ex.what();
	}
}

void PvoutputUploader::uploadDayData(const std::vector<DayData>& dayDatas) {
	std::string id;
	std::string apiKey;

	assert(!dayDatas.empty());

	readIdApiKey(db, id, apiKey);
	if (id == "" || apiKey == "") {
		return;
	}

	int32_t yield = 0;
	for (const DayData& sd : dayDatas) {
		yield += sd.dayYield;
	}

	try {
		uploadDayDataSum(dayDatas.begin()->date, yield, id, apiKey);
	} catch (const std::exception& ex) {
		LOG(Error) << "Uploading day data to pvoutput failed: " << ex.what();
	}
}
