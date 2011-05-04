#include "Pvlog.h"

#include <fstream>


Pvlog::Pvlog()
{
    //ctor
}

Pvlog::~Pvlog()
{
    //dtor
}

void Pvlog::openDatabase()
{
   //database = new SqliteDatabase()
}

void Pvlog::start(bool daemon)
{

}

/*
void Pvlog::readConfig(const std::string & path)
{
   std::string filename;
    if (path.empty()) {
        filename = "/etc/pvlog.conf";
    } else {
        filename = std::string(path);
    }

    ConfigReader config(filename);
    config.parse();

    name = config.getValue("database");

    try {
        hostname = config.getValue("hostname");
        port     = config.getValue("port");
        password = config.getValue("password");
    }
    catch(PvlogException &) {
        //not need for all sql databases.
    }

}
*/
