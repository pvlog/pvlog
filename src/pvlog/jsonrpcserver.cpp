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

#include "jsonrpcserver.h"

#include <string>

#include <odb/database.hxx>
#include <odb/query.hxx>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>

#include "datalogger.h"
#include "log.h"
#include "timeutil.h"

#include "models/plant.h"
#include "models/plant_odb.h"
#include "models/spotdata.h"
#include "models/spotdata_odb.h"
#include "models/daydata.h"
#include "models/daydata_odb.h"
#include "models/event.h"
#include "models/event_odb.h"
#include "models/inverter.h"
#include "models/inverter_odb.h"

namespace bg = boost::gregorian;
namespace pt = boost::posix_time;
namespace dt = boost::date_time;

using model::SpotData;
using model::SpotDataPtr;
using model::Inverter;
using model::InverterPtr;
using model::toJson;
using model::DayData;
using model::DayDataMonth;
using model::DayDataYear;
using model::Plant;
using model::Event;
using model::TopNDay;
using model::LowNDay;
using model::TopNMonth;
using model::LowNMonth;
using model::DayStats;
using model::MonthStats;

JsonRpcServer::JsonRpcServer(jsonrpc::AbstractServerConnector &conn, Datalogger* datalogger, odb::database* database) :
		AbstractPvlogServer(conn), db(database), datalogger(datalogger) {
	//Nothing to do
}

JsonRpcServer::~JsonRpcServer() {
	//Nothing to do
}

Json::Value JsonRpcServer::getSpotData(const std::string& date) {
	Json::Value result;
	using Query  = odb::query<SpotData>;
	using Result = odb::result<SpotData>;

	try {
		LOG(Debug) << "JsonRpcServer::getSpotData: " << date;

		bg::date d = bg::from_simple_string(date);
		if (d.is_not_a_date()) {
			return result;
		}

		pt::ptime begin = util::local_to_utc(pt::ptime(d));
		pt::ptime end   = begin + pt::hours(24);

		Query filterData(Query::time >= Query::_ref(begin) && Query::time < Query::_ref(end));
		Query sortResult("ORDER BY" + Query::inverter + "," + Query::time);

		odb::session session; //Session is needed for SpotData
		odb::transaction t(db->begin());
		Result r(db->query<SpotData>(filterData + sortResult));

		for (const SpotData& d : r) {
			result[std::to_string(d.inverter->id)][std::to_string(pt::to_time_t(d.time))] = toJson(d);
		}
		t.commit();
	} catch (const std::exception& ex) {
		LOG(Error) << "Error getting spot data" <<  ex.what();
		result = Json::Value();
	}

	return result;
}

Json::Value JsonRpcServer::getLiveSpotData() {
	Json::Value result;

	for (const auto& entry : datalogger->getLiveData()) {
		const SpotData& d = entry.second;

		pt::ptime curTime = pt::second_clock::universal_time();
		if (curTime - d.time < pt::minutes(5)) { //do do output old data
			result[std::to_string(d.inverter->id)][std::to_string(pt::to_time_t(d.time))] = toJson(d);
		}
	}

	return result;
}

Json::Value JsonRpcServer::getDataloggerStatus() {
	Json::Value result;

	LOG(Debug) << "JsonRpcServer::getDataloggerStatus";

	Datalogger::Status status = datalogger->getStatus();
	result["dataloggerStatus"] = status;

	return result;
}

Json::Value JsonRpcServer::getDayData(const std::string& from, const std::string& to) {
	Json::Value result;
	using Result = odb::result<DayData>;
	using Query  = odb::query<DayData>;

	try {
		LOG(Debug) << "JsonRpcServer::getDayData: " << from << "->" << to;

		bg::date fromTime = bg::from_simple_string(from);
		bg::date toTime   = bg::from_simple_string(to);

		odb::session session;
		odb::transaction t(db->begin());
		Result r(
				db->query<DayData>(Query::date >= fromTime && Query::date <= toTime));
		for (const DayData& d : r) {
			result[std::to_string(d.inverter->id)][bg::to_iso_extended_string(d.date)] =
					static_cast<Json::Int64>(d.dayYield);
		}
		t.commit();
	} catch (const std::exception& ex) {
		LOG(Error) << "Error getting day data" <<  ex.what();
		result = Json::Value();
	}

	return result;
}

Json::Value JsonRpcServer::getDayStats(const std::string& from, const std::string& to) {
	Json::Value result;
	using Result = odb::result<DayStats>;
	//using Query  = odb::query<DayStats>;

	try {
		LOG(Debug) << "JsonRpcServer::getDayStats: " << from << "->" << to;

		bg::date fromTime = bg::from_simple_string(from);
		bg::date toTime   = bg::from_simple_string(to);

		odb::session session;
		odb::transaction t(db->begin());

		std::string fromMonth = std::to_string(fromTime.month());
		std::string fromDay   = std::to_string(fromTime.day());

		std::string toMonth = std::to_string(toTime.month());
		std::string toDay   = std::to_string(toTime.day());

		int startYear = fromTime.year();
		int endYear   = toTime.year();
		if (startYear != endYear && startYear + 1 != endYear) {
			LOG(Error) << "Invalid time string";
			result = Json::Value();
			return result;
		}

		std::string queryString =
				std::string(("(") + fromMonth + " < " + " month or (month = " + fromMonth +  " and day >= " + fromDay +
				")) and ( month < " + toMonth  + " or (month = " + toMonth + " and day <= " + toDay + "))");

		int curYear   = startYear;
		int prevMonth = 1;
		Result r(db->query<DayStats>(queryString));
		for (const DayStats& d : r) {
			if (startYear != endYear && prevMonth > d.month) {
				curYear = endYear;
			}
			bg::date date(curYear, d.month, d.day);
			prevMonth = d.month;

			result[bg::to_iso_extended_string(date)] = toJson(d);
		}
		t.commit();
	} catch (const std::exception& ex) {
		LOG(Error) << "Error getting day stats: " <<  ex.what();
		result = Json::Value();
	}

	return result;
}

Json::Value JsonRpcServer::getMonthData(const std::string& year) {
	Json::Value result;
	using Result = odb::result<DayDataMonth>;

	try {
		LOG(Debug) << "JsonRpcServer::getMonthData: " << year;

		int y = std::stoi(year);

		odb::transaction t(db->begin());
		Result r(db->query<DayDataMonth> ("year = \"" + std::to_string(y) + "\""));
		for (const DayDataMonth& d: r) {
			result[std::to_string(d.inverterId)][std::to_string(y) + "-" + util::to_string(d.month, 2)] =
					static_cast<Json::Int64>(d.yield);
		}
		t.commit();
	} catch (const std::exception& ex) {
		LOG(Error) << "Error getting month data" <<  ex.what();
		result = Json::Value();
	}

	return result;
}

Json::Value JsonRpcServer::getMonthStats(const std::string& year) {
	Json::Value result;
	using Result = odb::result<MonthStats>;

	try {
		LOG(Debug) << "JsonRpcServer::getMonthStats: " << year;

		int y = std::stoi(year);

		odb::transaction t(db->begin());
		Result r(db->query<MonthStats>());
		for (const MonthStats& d: r) {
			result[std::to_string(y) + "-" + util::to_string(d.month, 2)] =
					toJson(d);
		}
		t.commit();
	} catch (const std::exception& ex) {
		LOG(Error) << "Error getting month stats" <<  ex.what();
		result = Json::Value();
	}

	return result;
}

Json::Value JsonRpcServer::getStatistics() {
	Json::Value result;
	using ResultTopNDay = odb::result<TopNDay>;
	using ResultLowNDay = odb::result<LowNDay>;
	using ResultTopNMonth = odb::result<TopNMonth>;
	using ResultLowNMonth = odb::result<LowNMonth>;

	try {
		LOG(Debug) << "JsonRpcServer::getStatstics";

		odb::transaction t(db->begin());

		ResultTopNDay topNDayRes(db->query<TopNDay>());
		Json::Value topNDayJson;
		for (const TopNDay& d: topNDayRes) {
			Json::Value entry;
			entry["date"]   = bg::to_iso_extended_string(d.date);
			entry["energy"] = static_cast<Json::Int64>(d.yield);
			topNDayJson.append(entry);
		}

		ResultLowNDay lowNDayRes(db->query<LowNDay>());
		Json::Value lowNDayJson;
		for (const LowNDay& d: lowNDayRes) {
			Json::Value entry;
			entry["date"]   = bg::to_iso_extended_string(d.date);
			entry["energy"] = static_cast<Json::Int64>(d.yield);
			lowNDayJson.append(entry);
		}

		ResultTopNMonth topNMonthRes(db->query<TopNMonth>());
		Json::Value topNMonthJson;
		for (const TopNMonth& d: topNMonthRes) {
			Json::Value entry;
			entry["date"]   = std::to_string(d.year) + "-" + util::to_string(d.month, 2);
			entry["energy"] = static_cast<Json::Int64>(d.yield);
			topNMonthJson.append(entry);
		}

		ResultLowNMonth lowNMonthRes(db->query<LowNMonth>());
		Json::Value lowNMonthJson;
		for (const LowNMonth& d: lowNMonthRes) {
			Json::Value entry;
			entry["date"]   = std::to_string(d.year) + "-" + util::to_string(d.month, 2);
			entry["energy"] = static_cast<Json::Int64>(d.yield);
			lowNMonthJson.append(entry);
		}

		result["topDays"] = topNDayJson;
		result["lowDays"] = lowNDayJson;
		result["topMonths"] = topNMonthJson;
		result["lowMonths"] = lowNMonthJson;

		t.commit();
	} catch (const std::exception& ex) {
		LOG(Error) << "Error getting statistics" <<  ex.what();
		result = Json::Value();
	}

	return result;
}

Json::Value JsonRpcServer::getYearData() {
	Json::Value result;
	using Result = odb::result<DayDataYear>;

	try {
		LOG(Debug) << "JsonRpcServer::getYearData";

		odb::transaction t(db->begin());
		Result r(db->query<DayDataYear>());
		for (const DayDataYear& d: r) {
			result[std::to_string(d.inverterId)][std::to_string(d.year)] =
					static_cast<Json::Int64>(d.yield);
		}
		t.commit();
	} catch (const std::exception& ex) {
		LOG(Error) << "Error getting year data" <<  ex.what();
		result = Json::Value();
	}

	return result;
}

Json::Value JsonRpcServer::getInverters() {
	Json::Value result;
	using Result = odb::result<Inverter>;

	try {
		LOG(Debug) << "JsonRpcServer::getInverters";

		odb::session session;
		odb::transaction t(db->begin());
		Result r(db->query<Inverter>());
		for (const Inverter& i : r) {
			result.append(toJson(i));
		}
		t.commit();
	} catch (const std::exception& ex) {
		LOG(Error) << "Error getting inverters" <<  ex.what();
		result = Json::Value();
	}

	return result;
}

Json::Value JsonRpcServer::getPlants() {
	Json::Value result;
	using Result = odb::result<Plant>;

	try {
		LOG(Debug) << "JsonRpcServer::getPlants";

		odb::session session;
		odb::transaction t(db->begin());
		Result r(db->query<Plant>());
		for (const Plant& p : r) {
			result.append(toJson(p));
		}
		t.commit();
	} catch (const std::exception& ex) {
		LOG(Error) << "Error getting plants" <<  ex.what();
		result = Json::Value();
	}

	return result;
}

Json::Value JsonRpcServer::getEvents() {
	Json::Value result;
	using Result = odb::result<Event>;
	using Query  = odb::query<Event>;

	try {
		LOG(Debug) << "JsonRpcServer::getEvents";

		odb::session session;
		odb::transaction t(db->begin());
		Result r(db->query<Event>("ORDER BY" + Query::inverter + "," + Query::time  + "DESC"));
		for (const Event e : r) {
			result[std::to_string(e.inverter->id)].append(toJson(e));
		}
		t.commit();
	} catch (const std::exception& ex) {
		LOG(Error) << "Error getting events" << ex.what();
		result = Json::Value();
	}

	return result;
}
