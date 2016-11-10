#include "JsonRpcServer.h"

#include <string>

#include <odb/database.hxx>
#include <odb/query.hxx>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>

#include "models/SpotData.h"
#include "SpotData_odb.h"
#include "models/Inverter.h"
#include "Inverter_odb.h"
#include "models/Plant.h"
#include "Plant_odb.h"
#include "models/DayData.h"
#include "DayData_odb.h"
#include "Log.h"

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

JsonRpcServer::JsonRpcServer(jsonrpc::AbstractServerConnector &conn, odb::database* database) :
		AbstractPvlogServer(conn), db(database) {
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

		pt::ptime begin = dt::c_local_adjustor<pt::ptime>::utc_to_local(pt::ptime(d));
		pt::ptime end(d, pt::hours(24));

		Query filterData(Query::time >= Query::_ref(begin) && Query::time < Query::_ref(end));
		Query sortResult("ORDER BY" + Query::inverter + "," + Query::time);

		odb::session session; //Session is needed for SpotData
		odb::transaction t(db->begin());
		Result r(db->query<SpotData>(filterData + sortResult));

		for (const SpotData& d : r) {
			Json::Value entry;
			entry[std::to_string(pt::to_time_t(d.time))] = toJson(d);
			result[std::to_string(d.inverter->id)].append(entry);
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
		Result r(db->query<DayData>(Query::date >= fromTime && Query::date <= toTime));
		for (const DayData& d : r) {
			Json::Value m;
			m[bg::to_simple_string(d.date)] = static_cast<Json::Int64>(d.dayYield);
			result[std::to_string(d.inverter->id)].append(m);
		}
		t.commit();
	} catch (const std::exception& ex) {
		LOG(Error) << "Error getting day data" <<  ex.what();
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
			Json::Value m;
			m[std::to_string(d.month)] = static_cast<Json::Int64>(d.yield);
			result[std::to_string(d.inverterId)].append(m);
		}
		t.commit();
	} catch (const std::exception& ex) {
		LOG(Error) << "Error getting month data" <<  ex.what();
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
			Json::Value m;
			m[std::to_string(d.year)] = static_cast<Json::Int64>(d.yield);
			result[std::to_string(d.inverterId)].append(m);
		}
		t.commit();
	} catch (const std::exception& ex) {
		LOG(Error) << "Error getting year data" <<  ex.what();
		result = Json::Value();
	}

	return result;
}

Json::Value JsonRpcServer::getInverter() {
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
