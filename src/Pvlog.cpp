#include "Pvlog.h"

#include <stdlib.h>

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
            PVLOG_EXCEPT("Could not get enviroment variable \"HOME\"!");
        } else {
            filename = std::string(home) + "/.pvlog.conf";
        }
    } else {
        filename = configFile;
    }

    ConfigReader configReader(filename);
    configReader.parse();

    std::string databaseType = configReader.getValue("database_type");
    std::string databaseName = configReader.getValue("database_name");
    std::string hostname     = configReader.getValue("hostname");
    std::string port         = configReader.getValue("port");
    std::string username     = configReader.getValue("username");
    std::string password     = configReader.getValue("password");

    database = std::move(std::unique_ptr<Database>(new SqliteDatabase()));
    database->open(databaseName, hostname, port, username, password);
}

void Pvlog::initPvlib()
{
	pvlib = std::move(std::unique_ptr<Pvlib>(new Pvlib(stderr)));

	std::set<std::string> connections = pvlib->supportedConnections();
	std::set<std::string> protocols   = pvlib->supportedProtocols();

	std::vector<Database::Plant> plants = database->plants();

	for (std::vector<Database::Plant>::const_iterator it = plants.begin();it != plants.end(); ++it) {
		Database::Plant plant = *it;

		if (connections.find(plant.connection) == connections.end()) {
			log_error("plant: %s has unsupported connection: %s", plant.name, plant.connection);
		}
		if (protocols.find(plant.protocol) == protocols.end()) {
			log_error("plant: %s has unsupported protocols: %s", plant.name, plant.protocol);
		}
		pvlib->openPlant(plant.name,
		                 plant.connection,
		                 plant.conParam1,
		                 plant.protocol,
		                 plant.password);
	}

}

Pvlog::Pvlog(const std::string& configFile)
{
	initDatabase(configFile);
	initPvlib();

	int timeout = readTimeout();

    std::unique_ptr<DaemonWork> dataLogger =
    	std::unique_ptr<DaemonWork>(new DataLogger(database.get()/*std::move(database)*/, pvlib.get()/*std::move(pvlib)*/, timeout));

    daemon = std::move(std::unique_ptr<Daemon>(new ForgroundDaemon(dataLogger.get())));
}

Pvlog::~Pvlog()
{
}

void Pvlog::start()
{
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

