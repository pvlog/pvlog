#include "Database.h"
#include "SqliteDatabase.h"
#include "Log.h"
#include "ConfigReader.h"

int main(int argc, char **argv) {
    std::string filename;

    char *home;
    home = getenv("HOME");
    if (home == NULL) {
        PVLOG_EXCEPT("Could not get environment variable \"HOME\"!");
    } else {
        filename = std::string(home) + "/.pvlog/pvlog.conf";
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

    Database* database = new SqliteDatabase();
    database->open(argv[1], "", "", "", "");
    database->createSchema();
    database->addPlant("sunnyboy", "rfcomm", argv[2], "", "smabluetooth", "0000");
    database->addInverter(2100106015, "sunnyboy", "sunnyboy", "sunnyboy", 5000, 1, 2);
    database->close();

    delete database;
}
