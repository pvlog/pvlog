#ifndef PVLOG_H
#define PVLOG_H

#include <string>
#include <memory>

#include "Utility.h"

class Database;
class Daemon;
class Pvlib;

class Pvlog {
	DISABLE_COPY( Pvlog)
private:
	std::string databaseType;
	std::string hostname;
	std::string port;
	std::string username;
	std::string password;

	std::unique_ptr<Daemon> daemon;
	std::unique_ptr<Database> database;
	std::unique_ptr<Pvlib> pvlib;

	int readTimeout();
	void initDatabase(const std::string& configFile);
	void initPvlib();

public:
	explicit Pvlog(const std::string& configFile = std::string());

	~Pvlog();

	void start();
};

#endif // PVLOG_H
