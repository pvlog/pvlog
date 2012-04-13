#include <cstdlib>
#include <iostream>

#include "Pvlog.h"
#include "Log.h"

int main(int argc, char **argv)
{
	Log::ReportingLevel() = Debug;

	if (argc != 2) {
		std::cerr << "Usage %s <config_file_path>" << std::endl;
		exit( EXIT_FAILURE);
	}

	Pvlog pvlog(argv[1]);
	pvlog.start();

	/*
	 Database* database = new SqliteDatabase();
	 database->open(argv[1], "", "", "", "");
	 database->createSchema();
	 database->addPlant("sunnyboy", "rfcomm", argv[2], "", "smabluetooth", "0000");
	 database->addLogicalPlant("sunnyboy", Database::Location(-10.9, 49.7), 45, 170);
	 database->addInverter(2100106015, "sunnyboy", "sunnyboy", "sunnyboy", 5000);
	 database->close();

	 delete database;
	 */
}
