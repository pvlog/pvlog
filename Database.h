#ifndef DATABASE_H
#define DATABASE_H

#include <string>
/*
#include "PvlogException.h"

class DatabaseException : public PvlogException {
    public:
        DatabaseException(const std::string & file, unsigned int line, const std::string & message) :
            PvlogException(file, line, message) {}
};

#define DATABASE_EXCEPT(MESSAGE) throw DatabaseException(__FILE__, __LINE__, MESSAGE)

*/


class Query;

class Database {
public:
    virtual ~Database() {};
    virtual void open(const std::string & database,
                      const std::string & hostname,
                      const std::string & port,
                      const std::string & username,
                      const std::string & password,
                      int flags = 0) = 0;
    virtual void close() = 0;
    Query *createQuery();
};

#endif // DATABASE_H
