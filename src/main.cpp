#include "Utility.h"
#include "SqliteDatabase.h"


int main(int argc, char **argv)
{
	if (argc != 3) {
		fprintf(stderr, "Usage %s <database path> <mac>", argv[0]);
	}

	Database* database = new SqliteDatabase();
	database->open(argv[1], "", "", "", "");
	database->createSchema();
	database->addPlant("sunnyboy", "rfcomm", argv[2], "", "smabluetooth", "0000");
	database->addLogicalPlant("sunnyboy", Location(-10.9, 49.7), 45, 170);
	database->addInverter(2100106015, "sunnyboy", "sunnyboy", "sunnyboy", 5000);
	database->close();

	delete database;
}
