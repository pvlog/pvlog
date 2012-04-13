#include "Pvlog.h"

#include <cstdlib>

#include "PvlogException.h"
#include "ConfigReader.h"
#include "DataLogger.h"
#include "SqliteDatabase.h"
#include "ForgroundDaemon.h"
#include "Log.h"

void Pvlog::initDatabase(const std::string& configFile)
{
	std::string filename;

	if (configFile.empty()) {
		char *home;
		home = getenv("HOME");
		if (home == NULL) {
			PVLOG_EXCEPT("Could not get environment variable \"HOME\"!");
		} else {
			filename = std::string(home) + "/.pvlog/pvlog.conf";
		}
	} else {
		filename = configFile;
	}

	LOG(Info) << "Reading database configuration file.";
	ConfigReader configReader(filename);
	configReader.parse();

	std::string databaseType = configReader.getValue("database_type");
	std::string databaseName = configReader.getValue("database_name");
	std::string hostname = configReader.getValue("hostname");
	std::string port = configReader.getValue("port");
	std::string username = configReader.getValue("username");
	std::string password = configReader.getValue("password");
	LOG(Info) << "Successfully parsed database configuration file.";

	database = std::unique_ptr<Database>(new SqliteDatabase());
	database->open(databaseName, hostname, port, username, password);
}

void Pvlog::initPvlib()
{
	pvlib = std::unique_ptr<Pvlib>(new Pvlib(stderr));

	std::set<std::string> connections = pvlib->supportedConnections();
	std::set<std::string> protocols = pvlib->supportedProtocols();

	std::vector<Database::Plant> plants = database->plants();

	for (std::vector<Database::Plant>::const_iterator it = plants.begin(); it != plants.end(); ++it) {
		Database::Plant plant = *it;

		LOG(Info) << "Opening plant " << plant.name << " [" << plant.connection << ", "
		        << plant.protocol << "]";

		if (connections.find(plant.connection) == connections.end()) {
			LOG(Error) << "plant: " << plant.name << "has unsupported connection: "
			        << plant.connection;
		}
		if (protocols.find(plant.protocol) == protocols.end()) {
			LOG(Error) << "plant: " << plant.name << "has unsupported protocol: " << plant.protocol;
		}
		pvlib->openPlant(plant.name, plant.connection, plant.protocol, plant.conParam1,
		        plant.password);

		LOG(Info) << "Successfull opened plant " << plant.name << " [" << plant.connection << ", "
		        << plant.protocol << "]";
	}
}

Pvlog::Pvlog(const std::string& configFile)
{
	LOG(Info) << "Opening database.";
	initDatabase(configFile);
	LOG(Info) << "Successfully opened database.";

	LOG(Info) << "Initializing pvlib.";
	initPvlib();
	LOG(Info) << "Successfully initialized pvlib";

	LOG(Info) << "Reading log timeout.";
	int timeout = readTimeout();
	LOG(Info) << "Successfully read log timeout: " << timeout << " seconds.";

	std::unique_ptr<DaemonWork> dataLogger = std::unique_ptr<DaemonWork>(new DataLogger(
	        database.get(), pvlib.get(), timeout));

	daemon = std::unique_ptr<Daemon>(new ForgroundDaemon(std::move(dataLogger)));
}

Pvlog::~Pvlog()
{
}

void Pvlog::start()
{
	LOG(Info) << "Start logging...";
	daemon->start();
}

int Pvlog::readTimeout()
{
	std::string timeoutStr = database->readConfig("timeout");
	std::stringstream ss(timeoutStr);
	int timeout;
	ss >> timeout;
	return timeout;
}

