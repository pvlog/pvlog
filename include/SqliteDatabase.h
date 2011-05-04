#ifndef SQLITEDATABASE_H
#define SQLITEDATABASE_H

#include "Datatabase.h"
#include "sqlite3.h"
#include <string>


class SqliteDatabase : public Datatabase {
    public:
        enum Flag {
            OPEN_READONLY  = SQLITE_OPEN_READONLY,
            OPEN_READWRITE = SQLITE_OPEN_READWRITE,
            OPEN_CREATE    = SQLITE_OPEN_CREATE
        };

        SqliteDatabase();
        virtual ~SqliteDatabase();
        bool open(const std::string &file, int flags);
        void close();



    private:
        sqlite3 *db;
        std::string filename;
};

#endif // SQLITEDATABASE_H
