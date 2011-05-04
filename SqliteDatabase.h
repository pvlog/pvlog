#ifndef SQLITEDATABASE_H
#define SQLITEDATABASE_H

#include "Database.h"
#include "sqlite3.h"
#include <string>

class SqliteQuery;

class SqliteDatabase : public Database {
    friend class SqliteQuery;
public:
    enum Flag {
        OPEN_READONLY  = SQLITE_OPEN_READONLY,
        OPEN_READWRITE = SQLITE_OPEN_READWRITE,
        OPEN_CREATE    = SQLITE_OPEN_CREATE
    };

    SqliteDatabase() : db(0) {}
    virtual ~SqliteDatabase() {}
    virtual void open(const std::string & database,
              const std::string &,
              const std::string &,
              const std::string &,
              const std::string &,
              int flags);
    virtual void close();

private:
    sqlite3     *db;
    std::string filename;
};





#endif // SQLITEDATABASE_H
