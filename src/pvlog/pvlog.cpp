#include <configreader.h>
#include <configservice.h>
#include <datalogger.h>
#include <forgrounddaemon.h>
#include <log.h>
#include <cstdlib>
#include <memory>

#include <odb/database.hxx>
#include <odb/sqlite/database.hxx>
#include <pvlog.h>

#include <pvlogexception.h>

using pvlib::Pvlib;

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

	database = std::unique_ptr<odb::core::database>(new odb::sqlite::database(databaseName, SQLITE_OPEN_READWRITE));
}

void Pvlog::initPvlib()
{
	pvlib = std::unique_ptr<Pvlib>(new Pvlib(stderr));
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

	std::unique_ptr<DaemonWork> dataLogger = std::unique_ptr<DaemonWork>(new Datalogger(
	        database.get(), pvlib.get()));

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
	return std::stoi(readConfig(database.get(), "timeout"));
}
