#include <iostream>
#include <vector>
#include "SqliteDatabase.h"
#include "SqliteQuery.h"
#include "DataStorage.h"

static void printUsage()
{
    std::cout << "pvlogcontrol [OPTION]" << std::endl;
    std::cout << std::endl;

    std::cout << "Options:" << std::endl;
    std::cout << "-a, --add         add new stringinverter." << std::endl;
}

int main(int argc, char **argv)
{ /*


*/
/*
    bool add = false;
    if (argc > 2) printUsage();

    if (argc == 2) {
        if (std::string(argv[1]) == "-a" || std::string(argv[1]) == "--add")
            add = true;
        else printUsage();
    } else {
        add = false;
    }

    if (add) {
        std::cout <<
    }
    */

 //   try
 //   {
  /*      database->open("/home/benjamin/pvlog.db", "", "", "", SqliteDatabase::OPEN_CREATE | SqliteDatabase::OPEN_READWRITE);


        Query *query = new SqliteQuery(static_cast<SqliteDatabase*>(database));
        query->exec("CREATE TABLE stored_values (date DATE PRIMARY KEY, dc_value INTEGER, ac_value INTEGER);");

        query->beginTransaction();

        query->exec("SELECT date FROM stored_values WHERE date > 100000 AND date < 300000;");
        std::vector<Query::Value> values;
        query->fetchNext(values);
        std::cout << values.at(0).getInt() << std::endl;

        for (i = 0; i < 1000; ++i) {
            //query->prepare("INSERT INTO stored_values(date, dc_value, ac_value) VALUES(datetime(:time1, 'unixepoch'), :dc, :ac);");
            query->prepare("INSERT INTO stored_values(date, dc_value, ac_value) VALUES(:time, :dc, :ac);");
            query->bindValueAdd(i);
            query->bindValueAdd(i);
            query->bindValueAdd(20);
            query->exec();
        }
        query->commitTransaction();
*/
    DataStorage dataStorage("sqlite");
    dataStorage.openDatabase("/home/benjamin/pvlog.db", "", "", "", "");
    dataStorage.createSchema();

 /*   }
    catch (PvlogException & exception)
    {
        std::cout << exception.what() << std::endl;

        return -1;
    }
*/
    return 0;
}
