#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include "Database.h"
#include "Value.h"

/*
 #include "PvlogException.h"

 class DatabaseException : public PvlogException {
 public:
 DatabaseException(const std::string & file, unsigned int line, const std::string & message) :
 PvlogException(file, line, message) {}
 };

 #define DATABASE_EXCEPT(MESSAGE) throw DatabaseException(__FILE__, __LINE__, MESSAGE)

 */

class SqlDatabase: public Database {
private:
	static const int VERSION = 0;
	std::string statment;
	std::vector<Value> values;
public:
	virtual void exec(std::vector<Value> & values) = 0;

	inline void bindValueAdd(const Value& value)
	{
		values.push_back(value);
	}

	inline void bindValueAdd()
	{
		values.push_back(Value());
	}

	inline void execQuery()
	{
		exec(values);
	}

	virtual void execQuery(const std::string& statment)
	{
		prepare(statment);
		std::vector<Value> values;
		exec(values);
	}

	virtual void beginTransaction() = 0;

	virtual void commitTransaction() = 0;

	virtual void prepare(const std::string& statment) = 0;

	virtual bool next() = 0;

	virtual Value getValue(int index) = 0;
public:
	SqlDatabase(const SqlDatabase&) = delete;

	SqlDatabase();

	virtual ~SqlDatabase();

	virtual void open(const std::string& database,
	                  const std::string& hostname,
	                  const std::string& port,
	                  const std::string& username,
	                  const std::string& password) = 0;

	virtual void close() = 0;

	virtual void createSchema() override;

	virtual bool checkDatabase() override;

	virtual void storeConfig(const std::string& value, const std::string& data) override;

	virtual std::string readConfig(const std::string& key) override;

	virtual void addPlant(const std::string& name,
	                      const std::string& connection,
	                      const std::string& conParam1,
	                      const std::string& conParam2,
	                      const std::string& protocol,
	                      const std::string& password) override;

	virtual std::vector<Plant> plants() override;

	virtual void addLogicalPlant(const std::string& name,
	                             double longitude,
	                             double latitude,
	                             double declination,
	                             double orientation) override;

	virtual std::vector<LogicalPlant> logicalPlants() override;

	virtual void addInverter(uint32_t id,
	                         const std::string& name,
	                         const std::string& plant,
	                         const std::string& logicalPlant,
	                         int32_t wattPeak,
	                         int phaseCount,
	                         int trackerCount) override;

	virtual std::vector<Inverter> inverters() override;

	Database::Inverter inverter(uint32_t id) override;

	virtual void storeAc(const Ac& ac, uint32_t id) override;

	virtual void storeDc(const Dc& dc, uint32_t id) override;

	virtual void storeStats(const Stats& stats, uint32_t id) override;

	virtual std::vector<std::pair<uint32_t, uint32_t> > readAc(uint32_t id,
	                                                           int line,
	                                                           Type type,
	                                                           const DateTime& from,
	                                                           const DateTime& to) override;

	virtual std::vector<std::pair<uint32_t, uint32_t> > readDc(uint32_t id,
	                                                           int trackerNum,
	                                                           Type type,
	                                                           const DateTime& from,
	                                                           const DateTime& to) override;

	virtual std::vector<std::pair<DateTime, uint32_t>> readDayPower(uint32_t,
	                                                               const DateTime& fromDay,
	                                                               const DateTime& toDay) override;
};

#endif // DATABASE_H
