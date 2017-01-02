#include "JsonRpcAdminServer.h"

#include <odb/database.hxx>
#include <odb/query.hxx>

#include "Datalogger.h"
#include "odb/database.hxx"
#include "models/Inverter.h"
#include "Inverter_odb.h"
#include "models/Plant.h"
#include "Plant_odb.h"
#include "models/Config.h"
#include "Config_odb.h"
#include "pvlibhelper.h"
#include "Log.h"


using model::PlantPtr;
using model::Plant;
using model::Inverter;
using model::InverterPtr;
using model::toJson;
using model::inverterFromJson;
using model::plantFromJson;
using model::Config;
using model::ConfigPtr;
using model::configFromJson;

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

Json::Value JsonRpcAdminServer::getInverters() {
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

Json::Value JsonRpcAdminServer::getPlants() {
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

Json::Value JsonRpcAdminServer::scanForInverters(const Json::Value& plantJson) {
	Json::Value result;

	try {
		Plant plant = plantFromJson(plantJson);

		pvlib_plant* pvlibPlant = connectPlant(plant.connection, plant.protocol, plant.connectionParam, plant.protocolParam);
		std::unordered_set<int64_t> inverters = ::getInverters(pvlibPlant);

		for (int64_t inverterId : inverters) {
			result.append(static_cast<Json::Int64>(inverterId));
		}

		pvlib_close(pvlibPlant);

	} catch (PvlogException& ex) {
	}

	return result;
}

Json::Value JsonRpcAdminServer::getSupportedConnections() {
	Json::Value result;
	int conNum = pvlib_connection_num();
	std::vector<uint32_t> conHandles;
	conHandles.resize(conNum);

	pvlib_connections(conHandles.data(), conHandles.size());
	for (uint32_t conHandle : conHandles) {
		result.append(pvlib_connection_name(conHandle));
	}

	return result;
}

Json::Value JsonRpcAdminServer::getSupportedProtocols() {
	Json::Value result;
	int protocolNum = pvlib_protocol_num();
	std::vector<uint32_t> protocolHandles;
	protocolHandles.resize(protocolNum);

	pvlib_protocols(protocolHandles.data(), protocolHandles.size());
	for (uint32_t protocoHandle : protocolHandles) {
		result.append(pvlib_protocol_name(protocoHandle));
	}

	return result;
}

Json::Value JsonRpcAdminServer::saveInverter(const Json::Value& inverterData) {
	Json::Value result;

	try {
		Inverter inverter = inverterFromJson(inverterData);
		InverterPtr inv = db->load<Inverter>(inverter.id);
		if (inv == nullptr) {
			db->persist(inverter);
		} else {
			db->update(inverter);
		}

		result = Json::Value();
	} catch (odb::exception &ex) {
		result = errorToJson(-11, "Database error!");
	} catch (std::exception &ex) {
		result = errorToJson(-1, "Conversion error!");
	}

	return result;
}

Json::Value JsonRpcAdminServer::savePlant(const Json::Value& plantJson) {
	Json::Value result;

	try {
		Plant plant = plantFromJson(plantJson);
		if (plant.id == -1) {
			db->persist(plant);
		} else {
			db->update(plant);
		}

		result = Json::Value(static_cast<Json::Int64>(plant.id));
	} catch (odb::exception &ex) {
		result = errorToJson(-11, "Database error!");
	} catch (std::exception &ex) {
		result = errorToJson(-1, "Conversion error!");
	}

	return result;
}


Json::Value JsonRpcAdminServer::getConfigs() {
	Json::Value result;
	using Result = odb::result<Config>;

	try {
		LOG(Debug) << "JsonRpcServer::getConfig";

		odb::session session;
		odb::transaction t(db->begin());
		Result r(db->query<Config>());
		for (const Config& c : r) {
			result.append(toJson(c));
		}
		t.commit();
	} catch (odb::exception &ex) {
		result = errorToJson(-11, "Database error!");
	} catch (const std::exception& ex) {
		LOG(Error) << "Error getting configs" <<  ex.what();
		result = errorToJson(-1, "Error reading Configurations");
	}

	return result;
}

Json::Value JsonRpcAdminServer::saveConfig(const Json::Value& configJson) {
	Json::Value result;

	try {
		Config config = configFromJson(configJson);
		ConfigPtr c(db->load<Config>(config.key));
		if (c != nullptr) {
			//c->value = config.value;
			db->update(config);
		} else {
			db->persist(config);
		}

		result = Json::Value();
	} catch (std::exception &ex) {
		result = errorToJson(-1, "Conversion error!");
	}

	return result;
}
