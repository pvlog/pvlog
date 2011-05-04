#ifndef SQLITE_QUERY_H
#define SQLITE_QUERY_H

#include "Query.h"

class SqliteDatabase;
struct sqlite3_stmt;

class SqliteQuery : public Query {
    friend class SqliteDatabase;
private:
    SqliteDatabase      *db;
    sqlite3_stmt        *stmt;
    std::vector<Value>  values;
    bool                hasNext;
    bool                first;

protected:
    virtual void exec(std::vector<Value> & values);
    bool step();
public:
    SqliteQuery(SqliteDatabase *sqliteDb) : db(sqliteDb), stmt(NULL), hasNext(0) {}

    virtual void prepare(const std::string & statment);

    virtual void beginTransaction()
    {
        Query::exec("BEGIN TRANSACTION;");
    }

    virtual void commitTransaction()
    {
        Query::exec("COMMIT TRANSACTION;");
    }

    virtual bool next()
    {
        if (first) {
            first = false;
            return hasNext;
        } else {
            return step();
        }
    }

    Value getValue(int index)
    {
        if (static_cast<size_t>(index) >= values.size())
            PVLOG_EXCEPT("Invalid index!");

        Value Value = values.at(index);
        if (first) first = false;
        return Value;
    }

};

#endif // #ifndef SQLITE_QUERY_H

