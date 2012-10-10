#ifndef SQLITEDATABASE_H
#define SQLITEDATABASE_H

#include <string>

#include "SqlDatabase.h"
#include "sqlite3.h"
#include "Value.h"

class SqliteDatabase: public SqlDatabase {
private:
	sqlite3* db;
	std::string filename;
	sqlite3_stmt* stmt;
	std::vector<Value> values;
	bool stepRet;
	bool first;
protected:
	virtual void beginTransaction();

	virtual void commitTransaction();

	virtual void prepare(const std::string& statment);

	void exec(std::vector<Value>& values);

	virtual bool step();

	virtual bool next();

	virtual Value getValue(int index);
public:
	/*   enum Flag {
	 OPEN_READONLY  = SQLITE_OPEN_READONLY,
	 OPEN_READWRITE = SQLITE_OPEN_READWRITE,
	 OPEN_CREATE    = SQLITE_OPEN_CREATE
	 };
	 */
	SqliteDatabase();

	SqliteDatabase(const std::string& database);

	virtual ~SqliteDatabase();

	virtual void open(const std::string & database,
	                  const std::string &,
	                  const std::string &,
	                  const std::string &,
	                  const std::string &);

	virtual void close();
};

#endif // SQLITEDATABASE_H
