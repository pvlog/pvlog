#include "SqliteDatabase.h"

#include "PvlogException.h"
#include "sqlite3.h"

SqliteDatabase::SqliteDatabase() : db(NULL), stmt(NULL) { /* nothing to do */ }

SqliteDatabase::~SqliteDatabase()
{
	close();
}

void SqliteDatabase::beginTransaction()
{
	SqlDatabase::exec("BEGIN TRANSACTION;");
}

void SqliteDatabase::commitTransaction()
{
	SqlDatabase::exec("COMMIT TRANSACTION;");
}

void SqliteDatabase::prepare(const std::string& statment)
{
    if (stmt != NULL) sqlite3_finalize(stmt);

    if (sqlite3_prepare_v2(db, statment.c_str(), statment.size() + 1, &stmt, 0) != SQLITE_OK)
        PVLOG_EXCEPT(sqlite3_errmsg(db));
}

void SqliteDatabase::exec(std::vector<Value>& values)
{
    if (sqlite3_reset(stmt) != SQLITE_OK)
        PVLOG_EXCEPT("Failed to reset statment!");

    if (values.size() != static_cast<size_t>(sqlite3_bind_parameter_count(stmt))) {
        values.clear();
        PVLOG_EXCEPT("Parameter count missmatch.");
    }

    for (size_t i = 0; i < values.size(); ++i) {
        int ret = 0;
        Value value = values.at(i);

        switch (value.getType()) {
        case Value::Null :
            ret = sqlite3_bind_null(stmt, i + 1);
            break;
        case Value::Int16 : //Falltrough
        case Value::Int32 :
            ret = sqlite3_bind_int(stmt, i + 1, value.getInt32());
            break;
        case Value::Int64 :
            ret = sqlite3_bind_int64(stmt, i + 1, value.getInt64());
            break;
        case Value::Float : //Fallthrough
        case Value::Double :
            ret = sqlite3_bind_double(stmt, i + 1, value.getDouble());
            break;
        case Value::String :
            ret = sqlite3_bind_text(stmt, i + 1, value.getString().c_str(), -1,  NULL);
            break;
        case Value::Blob :
            //ret = sqlite3_bind_blob(stmt, reinterpret_cast<void*>(&value.getBlob().begin(), value.size, NULL);
            break;
        default :
            values.clear();
            PVLOG_EXCEPT("Invalid value!");
            break;
        }

        if (ret != SQLITE_OK) {
            values.clear();
            PVLOG_EXCEPT(sqlite3_errmsg(db));
        }

    }
    values.clear();
    hasNext = step();
    first   = true;
}

bool SqliteDatabase::next()
{
	if (first) {
		first = false;
		return hasNext;
	} else {
		return step();
	}
}

bool SqliteDatabase::step()
{
    values.clear();

    switch (sqlite3_step(stmt)) {
    case SQLITE_ROW :
        for (int i = 0; i < sqlite3_column_count(stmt); ++i) {
            switch (sqlite3_column_type(stmt, i)) {
            case SQLITE_INTEGER :
                values.push_back(sqlite3_column_int(stmt, i));
                break;
            case SQLITE_FLOAT :
                values.push_back(sqlite3_column_double(stmt, i));
                break;
            case SQLITE_NULL :
                values.push_back(Value());
                break;
            case SQLITE_BLOB :
                //TODO
                break;
            default :
                values.push_back(reinterpret_cast<const char *>(sqlite3_column_text(stmt, i)));
                break;
            }
        }
		break;
    case SQLITE_DONE :
        sqlite3_finalize(stmt);
        stmt = NULL;
        return false;
        break;
    default :
        sqlite3_finalize(stmt);
        stmt = NULL;
        PVLOG_EXCEPT("Unable to fetch row!");
        break;
    }

    return true;
}

Value SqliteDatabase::getValue(int index)
{
	if (static_cast<size_t>(index) >= values.size())
		PVLOG_EXCEPT("Invalid index!");

	Value Value = values.at(index);
	if (first) first = false;
	return Value;
}

void SqliteDatabase::open(const std::string & database,
                          const std::string &,
                          const std::string &,
                          const std::string &,
                          const std::string &)
{
    if (db) close();

    filename = std::string(database);

    if (sqlite3_open_v2(filename.c_str(), &db, SQLITE_OPEN_READWRITE |
			SQLITE_OPEN_CREATE, NULL) != SQLITE_OK) {
        PVLOG_EXCEPT(sqlite3_errmsg(db));
	}
}

void SqliteDatabase::close() {
    if (db == NULL) return ;
	if (stmt != NULL) sqlite3_finalize(stmt);

    if (sqlite3_close(db) != SQLITE_OK)
        PVLOG_EXCEPT(sqlite3_errmsg(db));

	db = NULL;
}


