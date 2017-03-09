#include <configreader.h>
#include <datalogger.h>
#include <jsonrpcadminserver.h>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <odb/database.hxx>
#include <odb/sqlite/database.hxx>
#include <jsonrpccpp/server/connectors/httpserver.h>
#include <jsonrpcserver.h>
#include <log.h>
#include <pvlib.h>


std::unique_ptr<odb::core::database> openDatabase(const std::string& configFile)
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

	return std::unique_ptr<odb::database>(new odb::sqlite::database(databaseName, SQLITE_OPEN_READWRITE, true));
}

int main(int argc, char **argv)
{
	Log::reportingLevel() = Trace;

	if (argc != 2) {
		std::cerr << "Usage %s <config_file_path>" << std::endl;
		exit( EXIT_FAILURE);
	}

	LOG(Info) << "Opening database.";
	std::unique_ptr<odb::database> db = openDatabase(argv[1]);
	LOG(Info) << "Successfully opened database.";


	Datalogger datalogger(db.get());

	//start json server
	jsonrpc::HttpServer httpserver(8383);
	JsonRpcServer server(httpserver, &datalogger, db.get());
	server.StartListening();

	jsonrpc::HttpServer adminHttpserver(8384);
	JsonRpcAdminServer adminServer(adminHttpserver, &datalogger, db.get());
	adminServer.StartListening();


	//start datalogger work
	datalogger.work();
}
