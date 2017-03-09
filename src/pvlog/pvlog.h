#ifndef PVLOG_H
#define PVLOG_H

#include <string>
#include <memory>
#include <odb/database.hxx>
#include <pvlib.h>

#include <utility.h>

class Daemon;

class Pvlog {
	DISABLE_COPY( Pvlog)
private:
	std::string databaseType;
	std::string hostname;
	std::string port;
	std::string username;
	std::string password;

	std::unique_ptr<Daemon> daemon;
	std::unique_ptr<odb::core::database> database;
	std::unique_ptr<pvlib::Pvlib> pvlib;

	int readTimeout();
	void initDatabase(const std::string& configFile);
	void initPvlib();

public:
	explicit Pvlog(const std::string& configFile = std::string());

	~Pvlog();

	void start();
};

#endif // PVLOG_H
