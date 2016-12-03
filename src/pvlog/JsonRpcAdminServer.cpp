#include "JsonRpcAdminServer.h"

#include <odb/database.hxx>
#include <odb/query.hxx>

#include "Datalogger.h"
#include "odb/database.hxx"
#include "models/Inverter.h"
#include "Inverter_odb.h"
#include "models/Plant.h"
#include "Plant_odb.h"
#include "pvlibhelper.h"


using model::PlantPtr;
using model::Plant;
using model::Inverter;
using model::InverterPtr;
using model::toJson;
using model::inverterFromJson;
using model::plantFromJson;

static Json::Value errorToJson(int num ,std::string message) {
	Json::Value value;
	Json::Value error;

	error["code"]    = num;
	error["message"] = message;

	value["error"] = error;

	return error;
}

JsonRpcAdminServer::JsonRpcAdminServer(jsonrpc::AbstractServerConnector& conn, Datalogger* datalogger, odb::database* db) :
		AbstractAdminServer(conn),
		datalogger(datalogger),
		db(db) {
	//nothing to do
}

JsonRpcAdminServer::~JsonRpcAdminServer() {
	//nothing to do
}

void JsonRpcAdminServer::stopDatalogger() {
	datalogger->stop();
}
void JsonRpcAdminServer::startDatalogger() {
	datalogger->start();
}

Json::Value JsonRpcAdminServer::getInverter() {
	return Json::Value();
}

Json::Value JsonRpcAdminServer::getPlants() {
	return Json::Value();
}

Json::Value JsonRpcAdminServer::scanForInverters(const Json::Value& plantJson) {
	Json::Value result;

	try {
		Plant plant = plantFromJson(plantJson);

		pvlib_plant* pvlibPlant = connectPlant(plant.connection, plant.protocol, plant.connectionParam, plant.protocolParam);
		std::unordered_set<int64_t> inverters = getInverters(pvlibPlant);

		for (int64_t inverterId : inverters) {
			result.append(static_cast<Json::Int64>(inverterId));
		}

		pvlib_close(pvlibPlant);

	} catch (PvlogException& ex) {
	}

	return result;
}

Json::Value JsonRpcAdminServer::saveInverter(const Json::Value& inverterData) {
	try {
		Inverter inverter = inverterFromJson(inverterData);
		db->persist(inverter);
	} catch (std::exception &ex) {
		return errorToJson(-1, "Conversion error!");
	}

	return Json::Value();
}

Json::Value JsonRpcAdminServer::savePlant(const Json::Value& plantJson) {
	try {
		Plant plant = plantFromJson(plantJson);
		db->persist(plant);
	} catch (std::exception &ex) {
		return errorToJson(-1, "Conversion error!");
	}

	return Json::Value();
}
