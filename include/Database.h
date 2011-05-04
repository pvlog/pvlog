#ifndef DATABASE_H
#define DATABASE_H

#include "PvlogException.h"

class DatabaseException : public PvlogException {
};

#define DATABASE_EXCEPT(MESSAGE) throw DatabasException(__FILE__, __LINE__, MESSAGE);

class Query;

class Database
{
    public:
        Database();
        virtual ~Database();
        bool open(const std::string & database,
                       const std::string & hostname,
                       const std::string & port,
                       const std::string & password,
                       int flags = 0)
        void close();
        Query *query();
};

#endif // DATABASE_H
