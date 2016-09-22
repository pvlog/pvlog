#include <memory>

#include <odb/database.hxx>
#include <odb/transaction.hxx>
#include <odb/sqlite/database.hxx>
#include <odb/schema-catalog.hxx>

#include "Database.h"
#include "PvlogException.h"
#include "Log.h"
#include "ConfigReader.h"
#include "models/Plant.h"
#include "Plant_odb.h"
#include "models/Inverter.h"
#include "Inverter_odb.h"
#include "models/Config.h"
#include "Config_odb.h"


using model::Plant;
using model::Config;
using model::Inverter;
using std::unique_ptr;
using odb::core::database;
using std::shared_ptr;
using std::make_shared;


int main(int argc, char **argv) {
//	std::string filename;
//
//	char *home;
//	home = getenv("HOME");
//	if (home == NULL) {
//		PVLOG_EXCEPT("Could not get environment variable \"HOME\"!");
//	} else {
//		filename = std::string(home) + "/.pvlog/pvlog.conf";
//	}
//
//	LOG(Info) << "Reading database configuration file.";
//	ConfigReader configReader(filename);
//	configReader.parse();
//
//	std::string databaseType = configReader.getValue("database_type");
//	std::string databaseName = configReader.getValue("database_name");
//	std::string hostname = configReader.getValue("hostname");
//	std::string port = configReader.getValue("port");
//	std::string username = configReader.getValue("username");
//	std::string password = configReader.getValue("password");
//	LOG(Info) << "Successfully parsed database configuration file.";

	std::string databaseName = "test.db";

	unique_ptr<database> db (new odb::sqlite::database (databaseName, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE));

	{
		odb::transaction t (db->begin ());
		odb::schema_catalog::create_schema (*db);
		t.commit ();
	}

	{
		Plant plant("sunnyboy", "rfcomm", argv[1], "", "smadata2plus", "0000");
		shared_ptr<Inverter> inverter = make_shared<Inverter>(2100106015, "sunnyboy", 5000, 1, 2);
		plant.inverters.push_back(inverter);
		odb::transaction t (db->begin ());
		db->persist(plant);
		t.commit();
	}

	{
		Config timeout("timeout", std::to_string(60 * 5));
		Config longitude("longitude", std::to_string(-10.97));
		Config latitude("latitude", std::to_string(49.71));

		odb::transaction t (db->begin ());
		db->persist(timeout);
		db->persist(longitude);
		db->persist(latitude);
		t.commit();

	}
}
