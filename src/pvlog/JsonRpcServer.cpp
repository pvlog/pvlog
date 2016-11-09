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
using model::DayDataMonth;
using model::DayDataYear;

JsonRpcServer::JsonRpcServer(jsonrpc::AbstractServerConnector &conn, odb::database* database) :
		AbstractPvlogServer(conn), db(database) {
	//Nothing to do
}

JsonRpcServer::~JsonRpcServer() {
	//Nothing to do
}

std::unordered_map<InverterPtr, std::vector<SpotDataPtr>> JsonRpcServer::readSpotData(const bg::date& date) {
	using Query  = odb::query<SpotData>;
	using Result = odb::result<SpotData>;

	//use local day begin
	pt::ptime begin = dt::c_local_adjustor<pt::ptime>::utc_to_local(pt::ptime(date));
	pt::ptime end(date, pt::hours(24));

	Query filterData(Query::time >= Query::_ref(begin) && Query::time < Query::_ref(end));
	Query sortResult("ORDER BY" + Query::inverter + "," + Query::time);

	InverterSpotData resultData;

	odb::session s; //Session is needed for SpotData
	odb::transaction t(db->begin());
	Result r(db->query<SpotData>(filterData + sortResult));

	for (Result::iterator i (r.begin ()); i != r.end (); ++i) {
		SpotDataPtr spotData = i.load();
		resultData[spotData->inverter].push_back(spotData);
	}
	t.commit();

	return resultData;
}

Json::Value JsonRpcServer::getSpotData(const std::string& dateString) {

	Json::Value jsonResult;

	try {
		bg::date date = bg::from_simple_string(dateString);
		if (date.is_not_a_date()) {
			return jsonResult;
		}

		InverterSpotData spotData = readSpotData(date);

		Json::Value jsonResult;
		for (auto const& entry : spotData) {
			const InverterPtr& inverter = entry.first;
			const std::vector<SpotDataPtr>& inverterSpotData = entry.second;

			Json::Value jsonInverterSpotData;
			for (const SpotDataPtr& spotData : inverterSpotData) {
				jsonInverterSpotData.append(toJson(*spotData));
			}

			jsonResult[std::to_string(inverter->id)] = jsonInverterSpotData;
		}
	} catch (const std::exception& ex) {
		LOG(Error) << "Error gettig spot data" <<  ex.what();
		jsonResult = Json::Value();
	}

	return jsonResult;
}

Json::Value JsonRpcServer::getLiveSpotData() {
	Json::Value result;
	return result;
}

Json::Value JsonRpcServer::getDayData(const std::string& from, const std::string& to) {
	Json::Value result;
	return result;
}

Json::Value JsonRpcServer::getMonthData(const std::string& year) {
	Json::Value result;
	using Result = odb::result<DayDataMonth>;

	int y = std::stoi(year);

	odb::transaction t(db->begin());
	Result r(db->query<DayDataMonth> ("year = \"" + std::to_string(y) + "\""));
	for (const DayDataMonth& d: r) {
		result[std::to_string(d.month)] = static_cast<Json::Int64>(d.yield);
	}
	t.commit();
	return result;
}

Json::Value JsonRpcServer::getYearData() {
	Json::Value result;
	using Result = odb::result<DayDataYear>;

	odb::transaction t(db->begin());
	Result r(db->query<DayDataYear> ());
	for (const DayDataYear& d: r) {
		result[std::to_string(d.year)] = static_cast<Json::Int64>(d.yield);
	}
	t.commit();
	return result;
}

Json::Value JsonRpcServer::getInverter() {
	Json::Value result;
	return result;
}

Json::Value JsonRpcServer::getPlants() {
	Json::Value result;
	return result;
}
