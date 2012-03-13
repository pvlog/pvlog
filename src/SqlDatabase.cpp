#include "SqlDatabase.h"

#include <cctype>
#include <algorithm>
#include <iostream>
#include <sstream>

#include "ConfigReader.h"
#include "DateTime.h"

SqlDatabase::SqlDatabase() { /* nothing to do */ }

SqlDatabase::~SqlDatabase() { /* nothing to do */ }

void SqlDatabase::createSchema()
{
    beginTransaction();

    exec("CREATE TABLE settings(value VARCHAR(128) PRIMARY KEY,"
                "data varchar(128));");
    exec("CREATE TABLE plant(name VARCHAR(64) PRIMARY KEY,"
                "connection VARCHAR(32), con_param1 VARCHAR(32) NOT NULL, con_param2 VARCHAR(32),"
                "protocol VARCHAR(32) NOT NULL, password VARCHAR(32));");
    exec("CREATE TABLE inverter(id INTEGER PRIMARY KEY,"
                " name VARCHAR(64), plant VARCHAR(64) NOT NULL REFERENCES plant(name),"
                "wattpeak INTEGER NOT NULL, total_power INTEGER, operation_time INTEGER,"
                "feed_in_time INTEGER);");
    exec("CREATE TABLE day_values(year SMALLINT, month SMALLINT, day SMALLINT,"
				"inverter INTEGER NOT NULL REFERENCES inverter(id),"
                "power INTEGER, PRIMARY KEY(year, month , day));");
    exec("CREATE TABLE errors(date INTEGER,"
                "inverter INTEGER NOT NULL REFERENCES inverter(id),"
                "message VARCHAR(500), error_code INTEGER);");
    exec("CREATE TABLE ac_values(line INTEGER, inverter INTEGER NOT NULL REFERENCES inverter(id),"
                "date INETGER, power INTEGER, voltage INTEGER, current INTEGER,"
                "PRIMARY KEY(line, inverter, date));");
    exec("CREATE TABLE tracker(num INTEGER,"
                "inverter INTEGER REFERENCES inverter(id), date INTEGER,"
                "voltage INTEGER, current INTEGER, power INTEGER, PRIMARY KEY(num, inverter, date))");

	prepare("INSERT INTO settings (value, data) VALUES(\"version\", :version);");
	std::stringstream ss;
	ss << VERSION;
	bindValueAdd(ss.str());
	exec();

    commitTransaction();
}

bool SqlDatabase::checkDatabase()
{
    exec("SELECT data FROM settings WHERE value=\"version\";");
    std::string str = getValue(0);
    std::stringstream ss(str);
    int version;
    ss >> version;

    return true;


    return (version == VERSION);
}

void SqlDatabase::storeConfig(const std::string& value, const std::string& data)
{
	beginTransaction();
	prepare("INSERT INTO settings (value, data) Values(:value, :data);");
	bindValueAdd(value);
	bindValueAdd(data);
	exec();
	commitTransaction();
}

std::string SqlDatabase::readConfig(const std::string& key)
{
	prepare("SELECT data FROM settings WHERE value = :key;");
	bindValueAdd(key);
	exec();
	std::string data = getValue(0);
	return data;
}

void SqlDatabase::addPlant(const std::string& name,
	                       const std::string& connection,
	                       const std::string& conParam1,
	                       const std::string& conParam2,
	                       const std::string& protocol,
	                       const std::string& password)
{
	beginTransaction();
	prepare("INSERT INTO plant (name, connection, con_param1, con_param2, protocol, password)"
			"VALUES(:name, :connection, :con_param1, :con_param2, :protocol, :password);");
	bindValueAdd(name);
	bindValueAdd(connection);
	bindValueAdd(conParam1);
	bindValueAdd(conParam2);
	bindValueAdd(protocol);
	bindValueAdd(password);
	exec();
	commitTransaction();
}

std::vector<Database::Plant> SqlDatabase::plants()
{
	exec("SELECT name, connection, con_param1, con_param2, protocol, password"
	     "FROM plant.");

	std::vector<Plant> plants;
	while (next()) {
		Plant plant;
		plant.name = getValue(0).getString();
		plant.connection = getValue(1).getString();
		plant.conParam1 = getValue(2).getString();
		plant.conParam2 = getValue(3).getString();
		plant.protocol  = getValue(4).getString();
		plant.password  = getValue(5).getString();
	}

	return plants;
}

void SqlDatabase::addLogicalPlant(const std::string& name,
                                  const Location& location,
                                  float declination,
                                  float orientation)
{
	beginTransaction();
	prepare("INSERT INTO logical_plant (name, longitude, latitude, declination, orientation)"
			"VALUES(:name, :longitude, :latitude, :declination, :orientation);");
	bindValueAdd(name);
	bindValueAdd(location.longitude);
	bindValueAdd(location.latitude);
	bindValueAdd(declination);
	bindValueAdd(orientation);
	exec();
	commitTransaction();
}

Location SqlDatabase::location(const std::string& logicalPlant)
{
	Location location(0.0, 0.0);
	return location;
}

void SqlDatabase::addInverter(uint32_t id,
	                          const std::string& name,
	                          const std::string& plant,
	                          const std::string& logicalPlant,
	                          int32_t wattPeak)
{
	beginTransaction();
	prepare("INSERT INTO inverter (id, name, plant, wattpeak)"
			"VALUES(:id, :name, :plant, :wattpeak);");
	bindValueAdd(static_cast<int64_t>(id));
	bindValueAdd(name);
	bindValueAdd(plant);
	bindValueAdd(logicalPlant);
	bindValueAdd(wattPeak);
	exec();
	commitTransaction();
}

void SqlDatabase::storeAc(const Ac& ac, uint32_t id)
{
    beginTransaction();

    for (int i = 0; i < ac.lineNum; ++i) {
        prepare("INSERT INTO ac_values (line, inverter, date, power, voltage, current)"
				"VALUES(:line, :id, :date, :power, :voltage, :current);");

        bindValueAdd(i);
        bindValueAdd(static_cast<int64_t>(id));
        bindValueAdd(static_cast<int32_t>(ac.time));

		if (Ac::isValid(ac.power[i])) bindValueAdd(ac.power[i]);
		else bindValueAdd();

        if (Ac::isValid(ac.voltage[i])) bindValueAdd(ac.voltage[i]);
        else bindValueAdd();

        if (Ac::isValid(ac.power[i])) bindValueAdd(ac.current[i]);
        else bindValueAdd();

        exec();
    }

    commitTransaction();
}

/*
std::vector<Ac> getAc(uint32_t id, const Date& date)
{
	prepare("SELECT date, num, power, voltage, current FROM tracker WHERE inverter = :inverter"
			"ORDER BY date ASC;");
	bindValueAdd(id);
	exec();

	Ac ac;
	while
}
*/
void SqlDatabase::storeDc(const Dc& dc, uint32_t id)
{
    beginTransaction();

    for (int i = 0; i < dc.trackerNum; i++) {
        prepare("INSERT INTO ac_values (num, inverter, date, power, voltage, current)"
				"VALUES(:num, :inverter, :date, :power, :voltage, :current);");
        bindValueAdd(i);
        bindValueAdd(static_cast<int64_t>(id));
        bindValueAdd(static_cast<int32_t>(dc.time));


        if (Dc::isValid(dc.power[i])) bindValueAdd(dc.power[i]);
		else bindValueAdd();

		if (Dc::isValid(dc.voltage[i])) bindValueAdd(dc.voltage[i]);
		else bindValueAdd();

		if (Dc::isValid(dc.current[i])) bindValueAdd(dc.current[i]);
		else bindValueAdd();

        bindValueAdd(dc.current[i]);
        exec();
    }

    commitTransaction();
}

void SqlDatabase::storeStats(const Stats& stats, uint32_t id)
{
		beginTransaction();
		prepare("UPDATE inverter"
				"SET"
				"total_power = coalesce(:total_power total_power),"
				"operation_time = :coalesce(:operation_time, operation_time),"
				"feed_in_time = coalesce(:feed_in_time, feed_in_time)"
				"WHERE id = :id;");
		if (Stats::isValid(stats.totalYield)) bindValueAdd(static_cast<int32_t>(stats.totalYield));
		else bindValueAdd();

		if (Stats::isValid(stats.operationTime))
			bindValueAdd(static_cast<int32_t>(stats.operationTime));
		else bindValueAdd();

		if (Stats::isValid(stats.feedInTime))
			bindValueAdd(static_cast<int32_t>(stats.feedInTime));
		else bindValueAdd();

		bindValueAdd(static_cast<int64_t>(id));
		exec();

		//day Value
		if (Stats::isValid(stats.dayYield)) {
			DateTime time(stats.time);
			prepare("INSERT INTO day_values(inverter, year, month, day, power)"
					"VALUES(:year, :month, :day, :power);");
			bindValueAdd(static_cast<int64_t>(id));
			bindValueAdd(static_cast<int16_t>(time.year()));
			bindValueAdd(static_cast<int16_t>(time.month()));
			bindValueAdd(static_cast<int16_t>(time.monthDay()));
			bindValueAdd(static_cast<int32_t>(stats.totalYield));

			exec();
		}

		commitTransaction();
}

/*
std::vector<int, std::string> SqlDatabase::getPlants()
{
    exec("SELECT id, name FROM plant;");

    std::vector<int, std::string> inverters;

    while(next()) {
        std::pair<uint32_t, std::string> pair;
        pair.first  = static_cast<uint32_t>(getValue(0).getInt64());
        pair.second = getValue(1).getString();

        inverters.push_back(pair);
    }

    return inverters;
}

std::vector<uint32_t, std::string> SqlDatabase::getInverters(int plantId)
{
    prepare("SELECT id, name FROM string_inverter WHERE id = :id");
    bindValueAdd(plantId);
    exec();

    std::vector<uint32_t, std::string>

    while(next()) {
        std::pair<uint32_t, std::string> pair;
        pair.first  = getValue(0).getInt64();
        pair.second = getValue(1).getString();

        inverters.push_back(pair);
    }

    return inverters;
}

void SqlDatabase::storeStats(const Stats & stats)
{
    beginTransaction();

    if (stats.energieTotal != 0xffffffff) {
        prepare("UPDATE string_inverter SET total_power = :total_power,"
                       "WHERE id = :id;");
        bindValueAdd(stats.energieTotal);
        bindValueAdd(stats.id);
        exec();
    }

    if (stats.operationTime != 0xffffffff) {
        prepare("UPDATE string_inverter SET operation_time = :time,"
                       "WHERE id = :id;");
        exec();
        bindValueAdd(stats.operationTime);
        bindValueAdd(stats.id);
        exec();
    }

    if (stats.feedInTime != 0xffffffff) {
        prepare("UPDATE string_inverter SET feed_in_time = : time,"
                       "WHERE id = :id;");
        bindValueAdd(stats.feedInTime);
        bindValueAdd(stats.id);
        exec();
    }

    prepare("IF EXISTS (SELECT date FROM day_values WHERE string_inverter = :id, date = :date)"
                   "UPDATE day_values SET power = :power WHERE string_inverter = :id, date = :date)"
                   "ELSE"
                   "INSERT INTO day_values(:date, :string_inverter, :power;");
    struct tm * timeInfo = localtime(&stats.time);
    char time[20];
    strftime(time, 20, "%Y-%m-%d", timeInfo);
    bindValueAdd(stats.id);
    bindValueAdd(time);
    bindValueAdd(stats.energieToday);
    bindValueAdd(stats.id);
    bindValueAdd(time);
    bindValueAdd(time);
    bindValueAdd(stats.id);
    bindValueAdd(stats.energieToday);
    exec();

    commitTransaction();
}

int SqlDatabase::numInverter()
{
	prepare("SELECT count(*) As inverter_num from string_inverter"
				   "WHERE plant = :id;");
	bindValueAdd(id);
	exec();
   // }

    return getValue(0).getInt();
}

int SqlDatabase::numInverter(int32_t plantId)
{
	prepare("SELECT count(*) As inverter_num from string_inverter"
				   "WHERE plant = :id;");
	bindValueAdd(id);
	exec();

    return getValue(0).getInt();
}

void SqlDatabase::getInverters(std::vector< std::pair<int32_t, std::string> > & inverters, int plantId)
{
    if (plantId < 0) {
        exec("SELECT id, name FROM string_inverter;");
    } else {
        prepare("SELECT id, name FROM string_inverter WHERE id = :id");
        bindValueAdd(plantId);
        exec();
    }
    while(next()) {
        std::pair<int64_t, std::string> pair;
        pair.first  = getValue(0).getLongLong();
        pair.second = getValue(1).getString();

        inverters.push_back(pair);
    }
}

void SqlDatabase::getInverterStats(int64_t id, Stats & stats)
{
    prepare("SELECT wattpeak, total_power, operation_time, feed_in_time"
                   "FROM string_inverter WHERE id = :id;");
    bindValueAdd(id);
    exec();

    Query::Value value;

    value = getValue(0);
    if (value.isNull()) stats.energieTotal = -1;
    else  stats.energieTotal = value.getLongLong();

    value = getValue(1);
    if (value.isNull()) stats.operationTime = -1;
    else  stats.operationTime = value.getLongLong();

    value = getValue(2);
    if (value.isNull()) stats.feedInTime = -1;
    else  stats.feedInTime = value.getLongLong();

    stats.id   = id;
    stats.time = static_cast<time_t>(-1);
}

void SqlDatabase::getMonthValues(int64_t id, int year, std::vector< std::pair<int, int32_t> > & power)
{
    prepare("SELECT month, SUM(power) FROM day_values WHERE id = :id, year = :year"
                   "GROUP BY month ORDER BY month ASC;");
    bindValueAdd(id);
    bindValueAdd(year);
    exec();

    while (next()) {
        std::pair<int, int32_t> pair(getValue(0).getInt(),
                                         getValue(1).getInt());
        power.push_back(pair);
    }
}

void SqlDatabase::getMonthValuesPlant(int id, int year, std::vector< std::pair<int, int32_t> > & power)
{
    prepare("SELECT month, SUM(power), FROM day_values"
                   "WHERE id IN(SELECT id FROM string_inverter WHERE plant = :id), year = :year"
                   "GROUP BY month ORDER BY month ASC;");
    bindValueAdd(id);
    bindValueAdd(year);
    exec();

    while (next()) {
        std::pair<int, int32_t> pair(getValue(0).getInt(),
                                     getValue(1).getInt());
        power.push_back(pair);
    }
}

void SqlDatabase::getYearValues(int64_t id, int fromYear, int toYear, std::vector< std::pair<int, int32_t> > & power)
{
    prepare("SELECT year, SUM(power), FROM day_values"
                   "WHERE id = :id, year >= :from_year AND year <= to_year"
                   "GROUP BY year ORDER BY year ASC;");
    bindValueAdd(id);
    bindValueAdd(fromYear);
    bindValueAdd(toYear);

    while (next()) {
        std::pair<int, int32_t> pair(getValue(0).getInt(),
                                     getValue(1).getInt());
        power.push_back(pair);
    }
}

void SqlDatabase::getYearValuesPlant(int id, int fromYear, int toYear, std::vector< std::pair<int, int32_t> > & power)
{
    prepare("SELECT year, SUM(power), FROM day_values"
                   "WHERE id IN(SELECT id FROM string_inverter WHERE plant = :id),"
                   "year >= :from_year AND year <= to_year"
                   "GROUP BY year ORDER BY year ASC;");
    bindValueAdd(id);
    bindValueAdd(fromYear);
    bindValueAdd(toYear);

    while (next()) {
        std::pair<int, int32_t> pair(getValue(0).getInt(),
                                     getValue(1).getInt());
        power.push_back(pair);
    }
}

int SqlDatabase::getPhaseCount(int64_t id)
{
    prepare("SELECT MAX(line) FROM ac_values WHERE string_inverter = :id");
    bindValueAdd(id);
    exec();

    return getValue(0).getInt();
}

void SqlDatabase::getAc(int64_t id, int phase, const Date & date, std::vector< std::pair<time_t, int32_t > > & ac, Type type)
{
    prepare("SELECT power, voltage, current FROM ac_values"
                   "WHERE string_inverter = :id, date >= :day_beginn AND <= :day_end,"
                   "line = :phase;");
    bindValueAdd(id);
    bindValueAdd(static_cast<int64_t>(date.getDayBeginn()));
    bindValueAdd(static_cast<int64_t>(date.getDayBeginn()) + 24 * 3600);
    bindValueAdd(phase);
    exec();

    while (next()) {
        std::pair<time_t, int32_t> pair(static_cast<time_t>(getValue(0).getLongLong()),
                                     getValue(1).getInt());
        ac.push_back(pair);
    }
}
*/
/*
void SqlDatabase::getAcPlant(int id, const Date & date, std::vector< std::pair<time_t, int32_t > > & ac, Type type)
{
    prepare("SELECT AVG(power), AVG(voltage), AVG(current)"
                   "FROM ac_values"
                   "INNER JOIN string_inverter ON ac_values.string_inverter = string_inverter.id"
                   "WHERE date >= :day_beginn AND <= :day_end, in = :id"
                   "GROUP BY string_inverter;");

    bindValueAdd(id);
    bindValueAdd(date.getDayBeginn());
    bindValueAdd(date.getDayBeginn() + 24 * 3600);
    exec();

    while (next()) {
        std::pair<time_t, int32_t> pair(static_cast<time_t>(getValue(0).getLongLong()),
                                     getValue(1).getInt());
        power.push_back(pair);
    }
}

*/
/*
void SqlDatabase::getDayValues(int64_t id, const Date & date, std::vector< std::pair<int, int> > values)
{
    prepare("SELECT ")
}
*/
