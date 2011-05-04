#include "SqliteDatabase.h"

#include "PvlogException.h"

void SqliteDatabase::open(const std::string & database,
                          const std::string &,
                          const std::string &,
                          const std::string &,
                          const std::string &,
                          int flags) {
    if (db) close();

    filename = std::string(database);

    if (sqlite3_open_v2(filename.c_str(), &db, flags, NULL) != SQLITE_OK)
        PVLOG_EXCEPT(sqlite3_errmsg(db));

}

void SqliteDatabase::close() {
    if (db == 0) return ;

    if (sqlite3_close(db) != SQLITE_OK)
        PVLOG_EXCEPT(sqlite3_errmsg(db));
}


