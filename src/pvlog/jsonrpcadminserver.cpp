#include <config.h>
#include <datalogger.h>
#include <inverter.h>
#include <jsonrpcadminserver.h>
#include <log.h>
#include <odb/database.hxx>
#include <odb/query.hxx>
#include <plant.h>

#include "odb/database.hxx"
#include "inverter_odb.h"
#include "plant_odb.h"
#include "config_odb.h"
#include "pvlibhelper.h"
#include "email.h"


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

static void saveOrUpdate(odb::database* db, const Config& config) {
	using Query = odb::query<Config>;

	ConfigPtr c = db->query_one<Config>(Query::key == config.key);
	if (c != nullptr) {
		db->update(config);
	} else {
		db->persist(config);
	}
}

static ConfigPtr readConfig(odb::database* db, const std::string& key) {
	using Query = odb::query<Config>;

	ConfigPtr c = db->query_one<Config>(Query::key == key);
	return c;
}

static Json::Value errorToJson(int num ,std::string message) {
	Json::Value value;
	Json::Value error;

	error["code"]    = num;
	error["message"] = message;

	value["error"] = error;

	return value;
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

bool JsonRpcAdminServer::isDataloggerRunning() {
	return datalogger->isRunning();
}

Json::Value JsonRpcAdminServer::getInverters() {
	Json::Value result;
	using Result = odb::result<Inverter>;

	try {
		LOG(Debug) << "JsonRpcServer::getInverters";

		result = Json::Value(Json::arrayValue);

		odb::session session;
		odb::transaction t(db->begin());
		Result r(db->query<Inverter>());
		for (const Inverter& i : r) {
			result.append(toJson(i));
		}
		t.commit();
	} catch (const odb::exception &ex) {
		LOG(Error) << "Error getting inverters: " <<  ex.what();
		result = errorToJson(-11, "Database error!");
	} catch (const std::exception &ex) {
		LOG(Error) << "Error getting inverters: " <<  ex.what();
		result = errorToJson(-1, "General error!");
	}

	return result;
}

Json::Value JsonRpcAdminServer::getPlants() {
	Json::Value result;
	using Result = odb::result<Plant>;

	try {
		LOG(Debug) << "JsonRpcServer::getPlants";

		result = Json::Value(Json::arrayValue);

		odb::session session;
		odb::transaction t(db->begin());
		Result r(db->query<Plant>());
		for (const Plant& p : r) {
			result.append(toJson(p));
		}
		t.commit();
	} catch (const odb::exception &ex) {
		LOG(Error) << "Error getting plants: " <<  ex.what();
		result = errorToJson(-11, "Database error!");
	} catch (const std::exception &ex) {
		LOG(Error) << "Error getting plant: " <<  ex.what();
		result = errorToJson(-1, "General error!");
	}

	return result;
}

Json::Value JsonRpcAdminServer::scanForInverters(const Json::Value& plantJson) {
	Json::Value result;

	try {
		LOG(Debug) << "JsonRpcServer::scanForInverters";

		if (datalogger->isRunning()) {
			result = errorToJson(-2, "Datalogger needs to be stopped before scanning for inverters!");
			return result;
		}

		Plant plant = plantFromJson(plantJson);

		pvlib_plant* pvlibPlant = connectPlant(plant.connection, plant.protocol, plant.connectionParam, plant.protocolParam);
		std::unordered_set<int64_t> inverters = ::getInverters(pvlibPlant);

		for (int64_t inverterId : inverters) {
			Json::Value inverter;
			inverter["id"] = static_cast<Json::Int64>(inverterId);
			result.append(inverter);
		}

		pvlib_close(pvlibPlant);

	} catch (const PvlogException& ex) {
		LOG(Error) << "Error scan for inverters: " <<  ex.what();
		result = errorToJson(-111, ex.what());
	} catch (const std::exception& ex) {
		LOG(Error) << "Error scan for inverters: " <<  ex.what();
		result = errorToJson(-1, "General error!");
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
	using InverterQuery = odb::query<Inverter>;
	using PlantQuery    = odb::query<Plant>;

	try {
		LOG(Debug) << "JsonRpc::saveInverter";

		Inverter inverter = inverterFromJson(inverterData);
		int64_t plantId = inverterData["plantId"].asInt64();

		odb::session session;
		odb::transaction t(db->begin());
		InverterPtr inv = db->query_one<Inverter>(InverterQuery::id == inverter.id);
		PlantPtr pl = db->query_one<Plant>(PlantQuery::id == plantId);
		if (pl == nullptr) {
			result = errorToJson(-11, "Database error!");
			return result;
		}

		inverter.plant = pl;

		if (inv == nullptr) {
			db->persist(inverter);
		} else {
			db->update(inverter);
		}
		t.commit();

		result = Json::Value(Json::ValueType::objectValue);
	} catch (const odb::exception &ex) {
		LOG(Error) << "save inverter: " << ex.what();
		result = errorToJson(-11, "Database error!");
//	} catch (const Json::Exception &ex) {
//		LOG(Error) << "save inverter: " << ex.what();
//		result = errorToJson(-1, "Conversion error!");
	} catch (const std::exception &ex) {
		LOG(Error) << "save inverter: " << ex.what();
		result = errorToJson(-1, "General error!");
	}

	return result;
}

Json::Value JsonRpcAdminServer::deleteInverter(const std::string& inverterId) {
	Json::Value result;

	try {
		LOG(Debug) << "JsonRpc::deleteInverter";

		int64_t id = std::stoll(inverterId);

		odb::transaction t(db->begin());
		db->erase<Inverter>(id);
		t.commit();

		result = Json::Value(Json::ValueType::objectValue);
	} catch (const odb::object_not_persistent&) {
		//inverter is not persisted, this is not an error!
		result = Json::Value(Json::ValueType::objectValue);
	} catch (const odb::exception &ex) {
		LOG(Error) << "delete: " << ex.what();
		result = errorToJson(-11, "Database error! Note: you cant delete inverter with associated data in database!");
	} catch (const std::exception &ex) {
		LOG(Error) << "save inverter: " << ex.what();
		result = errorToJson(-1, "General error!");
	}

	return result;
}

Json::Value JsonRpcAdminServer::savePlant(const Json::Value& plantJson) {
	Json::Value result;

	try {
		LOG(Debug) << "JsonRpc::savePlant";

		Plant plant = plantFromJson(plantJson);

		odb::transaction t(db->begin());
		if (plant.id == -1) {
			db->persist(plant);
		} else {
			db->update(plant);
		}

		result["id"] = Json::Value(static_cast<Json::Int64>(plant.id));
		t.commit();
	} catch (const odb::exception &ex) {
		LOG(Error) << "save plant: " << ex.what();
		result = errorToJson(-11, "Database error!");
//	} catch (const Json::Exception &ex) {
//		LOG(Error) << "save plant: " << ex.what();
//		result = errorToJson(-1, "Conversion error!");
	} catch (const std::exception &ex) {
		LOG(Error) << "save plant: " << ex.what();
		result = errorToJson(-1, "General error!");
	}

	return result;
}

Json::Value JsonRpcAdminServer::deletePlant(const std::string& plantId) {
	Json::Value result = Json::Value(Json::ValueType::objectValue);
	using Query = odb::query<Plant>;

	try {
		LOG(Debug) << "JsonRpc::deletePlant";

		int64_t id = std::stoll(plantId);

		odb::session session;
		odb::transaction t(db->begin());
		PlantPtr plant = db->query_one<Plant>(Query::id == id);

		if (plant == nullptr) {
			return result;
		}

		if (!plant->inverters.empty()) {
			errorToJson(-11, "Plant has inverters => plant can not be deleted!");
		}

		db->erase(plant);
		t.commit();
	} catch (const odb::exception &ex) {
		LOG(Error) << "delete Plant: " << ex.what();
		result = errorToJson(-11, "Database error!");
//	} catch (const Json::Exception &ex) {
//		LOG(Error) << "save plant: " << ex.what();
//		result = errorToJson(-1, "Conversion error!");
	} catch (const std::exception &ex) {
		LOG(Error) << "delete plant: " << ex.what();
		result = errorToJson(-1, "General error!");
	}

	return result;
}


Json::Value JsonRpcAdminServer::getConfigs() {
	Json::Value result;
	using Result = odb::result<Config>;

	try {
		LOG(Debug) << "JsonRpcServer::getConfig";

		result = Json::Value(Json::arrayValue);

		odb::session session;
		odb::transaction t(db->begin());
		Result r(db->query<Config>());
		for (const Config& c : r) {
			result.append(toJson(c));
		}
		t.commit();
	} catch (odb::exception &ex) {
		LOG(Error) << "Error getting configs" <<  ex.what();
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
		LOG(Debug) << "JsonRpcServer::saveConfig";

		Config config = configFromJson(configJson);

		odb::transaction t(db->begin());
		saveOrUpdate(db, config);
		t.commit();

		result = Json::Value(Json::ValueType::objectValue);
	} catch (const odb::exception &ex) {
		LOG(Error) << "save config: " << ex.what();
		result = errorToJson(-11, "Database error!");
//	} catch (const Json::Exception &ex) {
//		LOG(Error) << "save config: " << ex.what();
//		result = errorToJson(-1, "Conversion error!");
	} catch (const std::exception &ex) {
		LOG(Error) << "save config: " << ex.what();
		result = errorToJson(-1, "General error!");
	}

	return result;
}

Json::Value JsonRpcAdminServer::saveEmailServer(const std::string& server, int port, const std::string& user, const std::string& password) {
	Json::Value result;

	Config smtpServer("smtpServer", server);
	Config smtpPort("smtpPort", std::to_string(port));
	Config smtpUser("smtpUser", user);
	Config smtpPassword("smtpPassword", password);

	try {
		odb::transaction t(db->begin());
		saveOrUpdate(db, smtpServer);
		saveOrUpdate(db, smtpPort);
		saveOrUpdate(db, smtpUser);
		saveOrUpdate(db, smtpPassword);
		t.commit();

		result = Json::Value(Json::ValueType::objectValue);
	} catch (const odb::exception &ex) {
		LOG(Error) << "saveEmailServer: " << ex.what();
		result = errorToJson(-11, "Database error!");
	} catch (const std::exception &ex) {
		LOG(Error) << "saveEmailServer: " << ex.what();
		result = errorToJson(-1, "General error!");
	}

	return result;
}

Json::Value JsonRpcAdminServer::saveEmail(const std::string& email) {
	Json::Value result;

	Config emailConfig("email", email);

	try {
		odb::transaction t(db->begin());
		saveOrUpdate(db, emailConfig);
		t.commit();

		result = Json::Value(Json::ValueType::objectValue);
	} catch (const odb::exception &ex) {
		LOG(Error) << "saveEmail: " << ex.what();
		result = errorToJson(-11, "Database error!");
	} catch (const std::exception &ex) {
		LOG(Error) << "saveEmail: " << ex.what();
		result = errorToJson(-1, "General error!");
	}

	return result;

}

Json::Value JsonRpcAdminServer::sendTestEmail() {
	Json::Value result;

	try {
		ConfigPtr smtpServerConf   = readConfig(db, "smtpServer");
		ConfigPtr smtpPortConf     = readConfig(db, "smtpPort");
		ConfigPtr smtpUserConf     = readConfig(db, "smtpUser");
		ConfigPtr smtpPasswordConf = readConfig(db, "smtpPassword");

		ConfigPtr emailConf = readConfig(db, "email");

		const std::string& smtpServer = smtpServerConf->value;
		int smtpPort                  = std::stoi(smtpPortConf->value);
		const std::string& user       = smtpUserConf->value;
		const std::string& password   = smtpPasswordConf->value;

		const std::string& targetEmail = emailConf->value;

		Email email(smtpServer, smtpPort, user, password);
		email.send(user, targetEmail, "Pvlog test email", "Success: Pvlog email transmission is working!");

		result = Json::Value(Json::ValueType::objectValue);
	} catch (const odb::exception &ex) {
		LOG(Error) << "sendTestEmail: " << ex.what();
		result = errorToJson(-11, "Database error!");
	} catch (const std::exception &ex) {
		LOG(Error) << "sendTestEmail: " << ex.what();
		result = errorToJson(-1, "General error!");
	}

	return result;
}
