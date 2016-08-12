#include "Database.h"
#include "SqliteDatabase.h"
#include "Log.h"
#include "ConfigReader.h"
#include "SunriseSunset.h"
int main(int argc, char **argv) {
//    std::string filename;
//
//    char *home;
//    home = getenv("HOME");
//    if (home == NULL) {
//        PVLOG_EXCEPT("Could not get environment variable \"HOME\"!");
//    } else {
//        filename = std::string(home) + "/.pvlog/pvlog.conf";
//    }
//
//
//    LOG(Info) << "Reading database configuration file.";
//    ConfigReader configReader(filename);
//    configReader.parse();
//
//    std::string databaseType = configReader.getValue("database_type");
//    std::string databaseName = configReader.getValue("database_name");
//    std::string hostname = configReader.getValue("hostname");
//    std::string port = configReader.getValue("port");
//    std::string username = configReader.getValue("username");
//    std::string password = configReader.getValue("password");
//    LOG(Info) << "Successfully parsed database configuration file.";
//
//    Database* database = new SqliteDatabase();
//    database->open(databaseName, "", "", "", "");
//    database->createSchema();
//    database->addPlant("sunnyboy", "rfcomm", argv[1], "", "smadata2plus", "0000");
//    database->addLogicalPlant("sunnyboy", 0, 0, 0, 0);
//    database->addInverter(2100106015, "sunnyboy", "sunnyboy", "sunnyboy", 5000, 1, 2);
//    database->storeConfig("timeout", std::to_string(60 * 5));
//    database->storeConfig("longitude", std::to_string(-10.97));
//    database->storeConfig("latitude", std::to_string(49.71));
//    database->close();

    SunriseSunset sunrisesunset(49, -10);

    DateTime now;
    int julianDay = now.julianDay();

    std::cerr.precision(17);
    std::cerr << "julian Day: " << now.julianDay() << std::endl;

    DateTime sunrise = sunrisesunset.sunrise(julianDay);
    DateTime sunset  = sunrisesunset.sunset(julianDay);

    std::cerr << sunrise.timeString() << " " << sunset.timeString() << std::endl;

    //delete database;
}
