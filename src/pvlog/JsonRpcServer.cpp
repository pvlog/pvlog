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

namespace bg = boost::gregorian;
namespace pt = boost::posix_time;
namespace dt = boost::date_time;

using model::SpotData;
using model::SpotDataPtr;
using model::Inverter;
using model::InverterPtr;
using model::toJson;


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
	Query sortResult("ORDER BY" + Query::inverter + Query::time);

	InverterSpotData resultData;

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

	Json::Value value;

	bg::date date = bg::from_simple_string(dateString);
	if (date.is_not_a_date()) {
		return value;
	}

	InverterSpotData spotData = readSpotData(date);

	Json::Value jsonResult;
	for (auto const& entry : spotData) {
		InverterPtr const& inverter = entry.first;
		const std::vector<SpotDataPtr> inverterSpotData;

		Json::Value jsonInverterSpotData;
		for (SpotDataPtr const& spotData : inverterSpotData) {
			jsonInverterSpotData.append(toJson(*spotData));
		}

		jsonResult[std::to_string(inverter->id)] = jsonInverterSpotData;
	}

	return jsonResult;
}

Json::Value JsonRpcServer::getLiveSpotData() {
	Json::Value result;
	return result;
}

Json::Value JsonRpcServer::getMonthData(const std::string& month) {
	Json::Value result;
	return result;
}

Json::Value JsonRpcServer::getYearData(const std::string& year) {
	Json::Value result;
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
