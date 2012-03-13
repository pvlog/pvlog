
#include <cctype>
#include <algorithm>
#include <iostream>

#include "ConfigReader.h"
#include "SqliteDatabase.h"
#include "SqliteQuery.h"
#include "Date.h"

DataStorage::DataStorage(const std::string & type) : databaseType(type)
{

    std::transform(databaseType.begin(), databaseType.end(),
                   databaseType.begin(), ::tolower);
    if (databaseType == "sqlite") {
        database = new SqliteDatabase();
        query    = new SqliteQuery(static_cast<SqliteDatabase*>(database));
    } else {
        PVLOG_EXCEPT("Unsupported database: " + type + "!");
    }
}

DataStorage::~DataStorage()
{
    delete database;
    delete query;
}

void DataStorage::openDatabase(const std::string & database,
                               const std::string & hostname,
                               const std::string & port,
                               const std::string & username,
                               const std::string & password)
{
    int flags = 0;
    if (databaseType == "sqlite")
        flags = SqliteDatabase::OPEN_CREATE | SqliteDatabase::OPEN_READWRITE;

    try {
        this->database->open(database, "", "", "", "", flags);
    }
    catch (PvlogException & exception) {
        PVLOG_EXCEPT(std::string("Failed opening database: ") + exception.what());
    }
}

void DataStorage::createSchema()
{
    query->beginTransaction();

    query->exec("CREATE TABLE settings(value VARCHAR(128) PRIMARY KEY,"
                "data varchar(128));");
    query->exec("CREATE TABLE plant(id BIGINT PRIMARY KEY, name VARCHAR(125),"
                "longitude DOUBLE, latitude DOUBLE);");
    query->exec("CREATE TABLE string_inverter(id INTEGER PRIMARY KEY,"
                "plant INTEGER NOT NULL REFERENCES plant(id), name VARCHAR(125),"
                "wattpeak INTEGER, total_power INTEGER, operation_time TIME,"
                "feed_in_time TIME);");
    query->exec("CREATE TABLE day_values(date TIMESTAMP, string_inverter BIGINT NOT NULL REFERENCES string_inverter(id),"
                "power INTEGER);");
    query->exec("CREATE TABLE errors(date BIGINT PRIMARY KEY,"
                "string_inverter BIGINT NOT NULL REFERENCES string_inverter(id),"
                "message VARCHAR(500), error_code INTEGER);");
    query->exec("CREATE TABLE ac_values(line INTEGER, string_inverter BIGINT NOT NULL REFERENCES string_inverter(id),"
                "date BIGINT, power INTEGER, voltage INTEGER, current INTEGER,"
                "PRIMARY KEY(line, string_inverter, date));");
    query->exec("CREATE TABLE tracker(num INTEGER,"
                "string_inverter BIGINT REFERENCES string_inverter(id), date BIGINT,"
                "voltage INTEGER, current INTEGER, power INTEGER, PRIMARY KEY(num, string_inverter, date))");

    query->commitTransaction();
}

void DataStorage::storeConfig(const std::string& value, const std::string& data)
{
	query->beginTransaction();
	query->prepare("INSERT INTO settings (value, data) Values(:value, :data);");
	query->bindValueAdd(value);
	query->bindValueAdd(data);
	query->exec();
	query->commitTransaction();
}

void DataStorage::addPlant(const std::string& name, const std::string& address, const std::string& password)
{
	query->beginTransaction();
	query->prepare("INSERT INTO plant (name, address, password) VALUES(:name, :address, :password);");
	query->bindValueAdd(name);
	query->bindValueAdd(address);
	query->bindValueAdd(password);
	query->exec();
	query->commitTransaction();
}

void DataStorage::addInverter(int plantId, uint32_t id, int32_t wattPeak)
{
	query->beginTransaction();
	query->prepare("INSERT INTO inverter (id, plant, wattpeak) VALUES(:id, :plant, :wattpeak);");
	query->bindValueAdd(static_cast<int64_t>(id));
	query->bindValueAdd(plantId);
	query->bindValueAdd(wattPeak);
	query->exec();
	query->commitTransaction();
}

void DataStorage::storeAc(const Ac& ac, uint32_t id)
{
    query->beginTransaction();

    for (int i = 0; i < ac.lineNum; ++i) {
        //struct tm * timeInfo = localtime(&ac.time);
        //char time[20];
        //strftime(time, 20, "%Y-%m-%d %H:%M:%s", timeInfo);

        query->prepare("INSERT INTO ac_values (line, string_inverter, data, power, voltage, current)"
		               "Values(:line, :id, :date, :power, :voltage, :current);");

        query->bindValueAdd(i);
        query->bindValueAdd(static_cast<int64_t>(id));
        query->bindValueAdd(static_cast<int32_t>(ac.time));

		if (Ac::isValid(ac.power[i])) query->bindValueAdd(ac.power[i]);
		else query->bindValueAdd();

        if (Ac::isValid(ac.voltage[i])) query->bindValueAdd(ac.voltage[i]);
        else query->bindValueAdd();

        if (Ac::isValid(ac.power[i])) query->bindValueAdd(ac.current[i]);
        else query->bindValueAdd();

        query->exec();
    }

    query->commitTransaction();
}

void DataStorage::storeDc(const Dc& dc, uint32_t id)
{
    query->beginTransaction();

    for (int i = 0; i < dc.trackerNum; i++) {
        //struct tm * timeInfo = localtime(&dc.time);
        //char time[20];
        //strftime(time, 20, "%Y-%m-%d %H:%M:%s", timeInfo);

        query->prepare("INSERT INTO ac_values, Values(:tracker, :id, :date, :power, :voltage, :current);");
        query->bindValueAdd(i);
        query->bindValueAdd(static_cast<int64_t>(id));
        query->bindValueAdd(static_cast<int32_t>(dc.time));


        if (Dc::isValid(dc.power[i])) query->bindValueAdd(dc.power[i]);
		else query->bindValueAdd();

		if (Dc::isValid(dc.voltage[i])) query->bindValueAdd(dc.voltage[i]);
		else query->bindValueAdd();

		if (Dc::isValid(dc.current[i])) query->bindValueAdd(dc.current[i]);
		else query->bindValueAdd();

        query->bindValueAdd(dc.current[i]);
        query->exec();
    }

    query->commitTransaction();
}

/*
std::vector<int, std::string> DataStorage::getPlants()
{
    query->exec("SELECT id, name FROM plant;");

    std::vector<int, std::string> inverters;

    while(query->next()) {
        std::pair<uint32_t, std::string> pair;
        pair.first  = static_cast<uint32_t>(query->getValue(0).getInt64());
        pair.second = query->getValue(1).getString();

        inverters.push_back(pair);
    }

    return inverters;
}

std::vector<uint32_t, std::string> DataStorage::getInverters(int plantId)
{
    query->prepare("SELECT id, name FROM string_inverter WHERE id = :id");
    query->bindValueAdd(plantId);
    query->exec();

    std::vector<uint32_t, std::string>

    while(query->next()) {
        std::pair<uint32_t, std::string> pair;
        pair.first  = query->getValue(0).getInt64();
        pair.second = query->getValue(1).getString();

        inverters.push_back(pair);
    }

    return inverters;
}

void DataStorage::storeStats(const Stats & stats)
{
    query->beginTransaction();

    if (stats.energieTotal != 0xffffffff) {
        query->prepare("UPDATE string_inverter SET total_power = :total_power,"
                       "WHERE id = :id;");
        query->bindValueAdd(stats.energieTotal);
        query->bindValueAdd(stats.id);
        query->exec();
    }

    if (stats.operationTime != 0xffffffff) {
        query->prepare("UPDATE string_inverter SET operation_time = :time,"
                       "WHERE id = :id;");
        query->exec();
        query->bindValueAdd(stats.operationTime);
        query->bindValueAdd(stats.id);
        query->exec();
    }

    if (stats.feedInTime != 0xffffffff) {
        query->prepare("UPDATE string_inverter SET feed_in_time = : time,"
                       "WHERE id = :id;");
        query->bindValueAdd(stats.feedInTime);
        query->bindValueAdd(stats.id);
        query->exec();
    }

    query->prepare("IF EXISTS (SELECT date FROM day_values WHERE string_inverter = :id, date = :date)"
                   "UPDATE day_values SET power = :power WHERE string_inverter = :id, date = :date)"
                   "ELSE"
                   "INSERT INTO day_values(:date, :string_inverter, :power;");
    struct tm * timeInfo = localtime(&stats.time);
    char time[20];
    strftime(time, 20, "%Y-%m-%d", timeInfo);
    query->bindValueAdd(stats.id);
    query->bindValueAdd(time);
    query->bindValueAdd(stats.energieToday);
    query->bindValueAdd(stats.id);
    query->bindValueAdd(time);
    query->bindValueAdd(time);
    query->bindValueAdd(stats.id);
    query->bindValueAdd(stats.energieToday);
    query->exec();

    query->commitTransaction();
}

int DataStorage::numInverter()
{
	query->prepare("SELECT count(*) As inverter_num from string_inverter"
				   "WHERE plant = :id;");
	query->bindValueAdd(id);
	query->exec();
   // }

    return query->getValue(0).getInt();
}

int DataStorage::numInverter(int32_t plantId)
{
	query->prepare("SELECT count(*) As inverter_num from string_inverter"
				   "WHERE plant = :id;");
	query->bindValueAdd(id);
	query->exec();

    return query->getValue(0).getInt();
}

void DataStorage::getInverters(std::vector< std::pair<int32_t, std::string> > & inverters, int plantId)
{
    if (plantId < 0) {
        query->exec("SELECT id, name FROM string_inverter;");
    } else {
        query->prepare("SELECT id, name FROM string_inverter WHERE id = :id");
        query->bindValueAdd(plantId);
        query->exec();
    }
    while(query->next()) {
        std::pair<int64_t, std::string> pair;
        pair.first  = query->getValue(0).getLongLong();
        pair.second = query->getValue(1).getString();

        inverters.push_back(pair);
    }
}

void DataStorage::getInverterStats(int64_t id, Stats & stats)
{
    query->prepare("SELECT wattpeak, total_power, operation_time, feed_in_time"
                   "FROM string_inverter WHERE id = :id;");
    query->bindValueAdd(id);
    query->exec();

    Query::Value value;

    value = query->getValue(0);
    if (value.isNull()) stats.energieTotal = -1;
    else  stats.energieTotal = value.getLongLong();

    value = query->getValue(1);
    if (value.isNull()) stats.operationTime = -1;
    else  stats.operationTime = value.getLongLong();

    value = query->getValue(2);
    if (value.isNull()) stats.feedInTime = -1;
    else  stats.feedInTime = value.getLongLong();

    stats.id   = id;
    stats.time = static_cast<time_t>(-1);
}

void DataStorage::getMonthValues(int64_t id, int year, std::vector< std::pair<int, int32_t> > & power)
{
    query->prepare("SELECT month, SUM(power) FROM day_values WHERE id = :id, year = :year"
                   "GROUP BY month ORDER BY month ASC;");
    query->bindValueAdd(id);
    query->bindValueAdd(year);
    query->exec();

    while (query->next()) {
        std::pair<int, int32_t> pair(query->getValue(0).getInt(),
                                         query->getValue(1).getInt());
        power.push_back(pair);
    }
}

void DataStorage::getMonthValuesPlant(int id, int year, std::vector< std::pair<int, int32_t> > & power)
{
    query->prepare("SELECT month, SUM(power), FROM day_values"
                   "WHERE id IN(SELECT id FROM string_inverter WHERE plant = :id), year = :year"
                   "GROUP BY month ORDER BY month ASC;");
    query->bindValueAdd(id);
    query->bindValueAdd(year);
    query->exec();

    while (query->next()) {
        std::pair<int, int32_t> pair(query->getValue(0).getInt(),
                                     query->getValue(1).getInt());
        power.push_back(pair);
    }
}

void DataStorage::getYearValues(int64_t id, int fromYear, int toYear, std::vector< std::pair<int, int32_t> > & power)
{
    query->prepare("SELECT year, SUM(power), FROM day_values"
                   "WHERE id = :id, year >= :from_year AND year <= to_year"
                   "GROUP BY year ORDER BY year ASC;");
    query->bindValueAdd(id);
    query->bindValueAdd(fromYear);
    query->bindValueAdd(toYear);

    while (query->next()) {
        std::pair<int, int32_t> pair(query->getValue(0).getInt(),
                                     query->getValue(1).getInt());
        power.push_back(pair);
    }
}

void DataStorage::getYearValuesPlant(int id, int fromYear, int toYear, std::vector< std::pair<int, int32_t> > & power)
{
    query->prepare("SELECT year, SUM(power), FROM day_values"
                   "WHERE id IN(SELECT id FROM string_inverter WHERE plant = :id),"
                   "year >= :from_year AND year <= to_year"
                   "GROUP BY year ORDER BY year ASC;");
    query->bindValueAdd(id);
    query->bindValueAdd(fromYear);
    query->bindValueAdd(toYear);

    while (query->next()) {
        std::pair<int, int32_t> pair(query->getValue(0).getInt(),
                                     query->getValue(1).getInt());
        power.push_back(pair);
    }
}

int DataStorage::getPhaseCount(int64_t id)
{
    query->prepare("SELECT MAX(line) FROM ac_values WHERE string_inverter = :id");
    query->bindValueAdd(id);
    query->exec();

    return query->getValue(0).getInt();
}

void DataStorage::getAc(int64_t id, int phase, const Date & date, std::vector< std::pair<time_t, int32_t > > & ac, Type type)
{
    query->prepare("SELECT power, voltage, current FROM ac_values"
                   "WHERE string_inverter = :id, date >= :day_beginn AND <= :day_end,"
                   "line = :phase;");
    query->bindValueAdd(id);
    query->bindValueAdd(static_cast<int64_t>(date.getDayBeginn()));
    query->bindValueAdd(static_cast<int64_t>(date.getDayBeginn()) + 24 * 3600);
    query->bindValueAdd(phase);
    query->exec();

    while (query->next()) {
        std::pair<time_t, int32_t> pair(static_cast<time_t>(query->getValue(0).getLongLong()),
                                     query->getValue(1).getInt());
        ac.push_back(pair);
    }
}
*/
/*
void DataStorage::getAcPlant(int id, const Date & date, std::vector< std::pair<time_t, int32_t > > & ac, Type type)
{
    query->prepare("SELECT AVG(power), AVG(voltage), AVG(current)"
                   "FROM ac_values"
                   "INNER JOIN string_inverter ON ac_values.string_inverter = string_inverter.id"
                   "WHERE date >= :day_beginn AND <= :day_end, in = :id"
                   "GROUP BY string_inverter;");

    query->bindValueAdd(id);
    query->bindValueAdd(date.getDayBeginn());
    query->bindValueAdd(date.getDayBeginn() + 24 * 3600);
    query->exec();

    while (query->next()) {
        std::pair<time_t, int32_t> pair(static_cast<time_t>(query->getValue(0).getLongLong()),
                                     query->getValue(1).getInt());
        power.push_back(pair);
    }
}

*/
/*
void DataStorage::getDayValues(int64_t id, const Date & date, std::vector< std::pair<int, int> > values)
{
    query->prepare("SELECT ")
}
*/
