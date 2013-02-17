#ifndef UTIL_H
#define UTIL_H

#include <string>
#include "PvlogException.h"
#include "ConfigReader.h"
#include "SqliteDatabase.h"

class Util {
public:
	inline static  std::string readEnvVar(const std::string& var)
	{
		char *env = getenv(var.c_str());
		if (env == NULL) {
			PVLOG_EXCEPT("Failed reading environment variable: " + var);
		}
		return std::string(env);
	}

	inline static Database* openDatabase(const std::string& configFile)
	{
		ConfigReader configReader(configFile);
		configReader.parse();

		std::string databaseType = configReader.getValue("database_type");
		std::string databaseName = configReader.getValue("database_name");
		std::string hostname = configReader.getValue("hostname");
		std::string port = configReader.getValue("port");
		std::string username = configReader.getValue("username");
		std::string password = configReader.getValue("password");

		return new SqliteDatabase(databaseName);
	}

};

#endif //#ifndef UTIL_H
