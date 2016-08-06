#include "SqlDatabase.h"

#include <cctype>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <cassert>

#include "ConfigReader.h"
#include "DateTime.h"

using std::vector;
using std::string;

SqlDatabase::SqlDatabase()
{ /* nothing to do */
}

SqlDatabase::~SqlDatabase()
{ /* nothing to do */
}

void SqlDatabase::createSchema()
{
	try {
		beginTransaction();

		execQuery("CREATE TABLE settings(value VARCHAR(128) PRIMARY KEY,"
			 "data VARCHAR(128));");
		execQuery("CREATE TABLE plant(name VARCHAR(32) PRIMARY KEY,"
			 "connection VARCHAR(32), con_param1 VARCHAR(32) NOT NULL, con_param2 VARCHAR(32),"
			 "protocol VARCHAR(32) NOT NULL, password VARCHAR(32));");
		execQuery("CREATE TABLE logical_plant(name VARCHAR(32) PRIMARY KEY, longitude FLOAT NOT NULL, latitude FLOAT NOT NULL,"
			"declination FLOAT NOT NULL, orientation FLOAT NOT NULL)");
		execQuery("CREATE TABLE inverter(id INTEGER PRIMARY KEY,"
			 "name VARCHAR(32), plant VARCHAR(32) REFERENCES plant(name) NOT NULL,"
			 "logical_plant VARCHAR(32) NOT NULL REFERENCES logical_plant(name),"
			 "wattpeak INTEGER NOT NULL, total_power INTEGER, operation_time INTEGER,"
			 "phase_count INTEGER NOT NULL, tracker_count INTEGER NOT NULL,"
			 "feed_in_time INTEGER);");
		execQuery("CREATE TABLE day_values(julian_day INTEGER NOT NULL,"
			"inverter INTEGER NOT NULL REFERENCES inverter(id),"
			"power INTEGER NOT NULL, PRIMARY KEY(julian_day, inverter));");
		execQuery("CREATE TABLE errors(date INTEGER,"
			"inverter INTEGER NOT NULL REFERENCES inverter(id),"
			"message VARCHAR(500), error_code INTEGER);");
		execQuery("CREATE TABLE ac_values(inverter INTEGER REFERENCES inverter(id) NOT NULL,"
		     "date INTEGER,  power INTEGER, frequency INTEGER,"
			 "PRIMARY KEY(inverter, date));");
		execQuery("CREATE TABLE line(inverter INTEGER, date INTEGER, line SMALLINT,"
			 "power INTEGER, current INTEGER, voltage INTEGER,"
			 "PRIMARY KEY(inverter, date, line),"
			 "FOREIGN KEY(inverter, date) REFERENCES ac_values(inverter, date));");
		execQuery("CREATE TABLE tracker(num INTEGER,"
			 "inverter INTEGER REFERENCES inverter(id), date INTEGER,"
			 "voltage INTEGER, current INTEGER, power INTEGER, PRIMARY KEY(num, inverter, date))");

		prepare("INSERT INTO settings (value, data) VALUES(\"version\", :version);");
		std::stringstream ss;
		ss << VERSION;
		bindValueAdd(ss.str());
		execQuery();

		commitTransaction();
	} catch (const PvlogException& exception) {
		PVLOG_EXCEPT(std::string("Create schema: ") + exception.what());
	}
}

bool SqlDatabase::checkDatabase()
{
	execQuery("SELECT data FROM settings WHERE value=\"version\";");
	if (!next()) return false;
	std::string str = getValue(0);
	std::stringstream ss(str);
	int version;
	ss >> version;

	return (version == VERSION);
}

void SqlDatabase::storeConfig(const std::string& value, const std::string& data)
{
	try {
		beginTransaction();
		prepare("INSERT INTO settings (value, data) Values(:value, :data);");
		bindValueAdd(value);
		bindValueAdd(data);
		execQuery();
		commitTransaction();
	} catch (const PvlogException& exception) {
		PVLOG_EXCEPT(std::string("Store config: ") + exception.what());
	}
}

std::string SqlDatabase::readConfig(const std::string& key)
{
	prepare("SELECT data FROM settings WHERE value = :key;");
	bindValueAdd(key);
	execQuery();
	if (!next()) PVLOG_EXCEPT(std::string("No data with key: ") + key);
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
	try {
		beginTransaction();
		prepare("INSERT INTO plant (name, connection, con_param1, con_param2, protocol, password)\n"
		        "VALUES(:name, :connection, :con_param1, :con_param2, :protocol, :password);");
		bindValueAdd(name);
		bindValueAdd(connection);
		bindValueAdd(conParam1);
		bindValueAdd(conParam2);
		bindValueAdd(protocol);
		bindValueAdd(password);
		execQuery();
		commitTransaction();
	} catch (const PvlogException& exception) {
		PVLOG_EXCEPT(std::string("addPlant: ") + exception.what());
	}
}

std::vector<Database::Plant> SqlDatabase::plants()
{
	execQuery("SELECT name, connection, con_param1, con_param2, protocol, password\n"
	     "FROM plant;");

	std::vector<Plant> plants;
	while (next()) {
		Plant plant;
		plant.name = getValue(0).getString();
		plant.connection = getValue(1).getString();
		plant.conParam1 = getValue(2).getString();
		plant.conParam2 = getValue(3).getString();
		plant.protocol = getValue(4).getString();
		plant.password = getValue(5).getString();

		plants.push_back(plant);
	}

	return plants;
}

void SqlDatabase::addLogicalPlant(const std::string& name,
                                  double longitude,
                                  double latitude,
                                  double declination,
                                  double orientation)
{
	try {
		beginTransaction();
		prepare("INSERT INTO logical_plant(name, longitude, latitude, declination, orientation)\n"
		        "VALUES(:name, :longitude, :latitude, :declination, :orientation);");
		bindValueAdd(name);
		bindValueAdd(longitude);
		bindValueAdd(latitude);
		bindValueAdd(declination);
		bindValueAdd(orientation);
		execQuery();
		commitTransaction();
	} catch (const PvlogException& exception) {
		PVLOG_EXCEPT(std::string("Add logicalPlant ") + exception.what());
	}
}

std::vector<Database::LogicalPlant> SqlDatabase::logicalPlants()
{
	std::vector<LogicalPlant> plants;
	try {
		execQuery("SELECT name, longitude, latitude, declination, orientation\n"
		     "FROM logical_plant;");
		while (next()) {
			LogicalPlant plant;
			plant.name        = getValue(0).getString();
			plant.longitude   = getValue(1).getDouble();
			plant.latitude    = getValue(2).getDouble();
			plant.declination = getValue(3).getDouble();
			plant.orientation = getValue(4).getDouble();

			plants.push_back(plant);
		}
	}
	catch (const PvlogException& ex) {
		PVLOG_EXCEPT(std::string("logicalPlants(): ") + ex.what());
	}

	return plants;
}

void SqlDatabase::addInverter(uint32_t id,
                              const std::string& name,
                              const std::string& plant,
                              const std::string& logicalPlant,
                              int32_t wattPeak,
                              int phaseCount,
                              int inverterCount)
{
	try {
		beginTransaction();
		prepare("INSERT INTO inverter (id, name, plant, logical_plant, wattpeak, phase_count, tracker_count)\n"
				"VALUES(:id, :name, :logical_plant, :plant, :wattpeak, :phase_count, :tracker_count);");
		bindValueAdd(static_cast<int64_t> (id));
		bindValueAdd(name);
		bindValueAdd(plant);
		bindValueAdd(logicalPlant);
		bindValueAdd(wattPeak);
		bindValueAdd(phaseCount);
		bindValueAdd(inverterCount);
		execQuery();
		commitTransaction();
	} catch (const PvlogException& exception) {
		PVLOG_EXCEPT(std::string("Add inverter ") + exception.what());
	}
}

vector<Database::Inverter> SqlDatabase::inverters()
{
	vector<Inverter> inverters;

	try {
		execQuery("SELECT id, name, plant, logical_plant, wattpeak, tracker_count, phase_count FROM inverter;");
		while(next()) {
			Inverter inverter;
			inverter.id   = static_cast<uint32_t>(getValue(0).getInt64());

			Value value = getValue(1);
			if (!value.isNull()) inverter.name = value.getString();

			value = getValue(2);
			if (!value.isNull()) inverter.plant = value.getString();

			value = getValue(3);
			if (!value.isNull()) inverter.logicalPlant = value.getString();

			value = getValue(4);
			if (!value.isNull()) inverter.wattpeak = value.getInt32();
			else inverter.wattpeak = 0;

			inverter.trackerCount = getValue(5).getInt32();
			inverter.phaseCount = getValue(6).getInt32();

			inverters.push_back(inverter);
		}
	}
	catch (PvlogException& exception) {
		PVLOG_EXCEPT(string("inverters ") + exception.what());
	}

	return inverters;
}

Database::Inverter SqlDatabase::inverter(uint32_t id)
{

   try {
           execQuery("SELECT id, name, plant, logical_plant, wattpeak, tracker_count, phase_count FROM inverter, WHERE id = :id;");
           bindValueAdd(static_cast<int64_t>(id));
   }
   catch (PvlogException& exception) {
           PVLOG_EXCEPT(string("inverter ") + exception.what());
   }
   Inverter inverter;

   inverter.id = id;

   Value value = getValue(1);
   if (!value.isNull()) inverter.name = value.getString();

   value = getValue(2);
   if (!value.isNull()) inverter.plant = value.getString();

   value = getValue(3);
   if (!value.isNull()) inverter.logicalPlant = value.getString();

   value = getValue(4);
   if (!value.isNull()) inverter.wattpeak = value.getInt32();
   else inverter.wattpeak = 0;

   inverter.trackerCount = getValue(5).getInt32();
   inverter.phaseCount = getValue(6).getInt32();

   return inverter;
}

void SqlDatabase::storeAc(const Ac& ac, uint32_t id)
{
	try {
		beginTransaction();

		prepare("INSERT INTO ac_values(inverter, date, frequency, power)\n"
		        "VALUES(:inverter, :date, :frequency, :power);");

		bindValueAdd(static_cast<int64_t> (id));
		bindValueAdd(static_cast<int32_t> (ac.time));

		if (Ac::isValid(ac.frequence)) bindValueAdd(ac.frequence);
		else bindValueAdd();

		if (Ac::isValid(ac.totalPower)) bindValueAdd(static_cast<int32_t> (ac.totalPower));
		else bindValueAdd();

		execQuery();

		for (int i = 0; i < ac.lineNum; ++i) {
			prepare("INSERT INTO line(inverter, date, line, power, voltage, current)\n"
			        "VALUES(:line, :id, :date, :power, :voltage, :current);");

			bindValueAdd(static_cast<int64_t> (id));
			bindValueAdd(static_cast<int32_t> (ac.time));
			bindValueAdd(i);

			if (Ac::isValid(ac.power[i])) bindValueAdd(ac.power[i]);
			else bindValueAdd();

			if (Ac::isValid(ac.voltage[i])) bindValueAdd(ac.voltage[i]);
			else bindValueAdd();

			if (Ac::isValid(ac.current[i])) bindValueAdd(ac.current[i]);
			else bindValueAdd();

			execQuery();
		}

		commitTransaction();
	} catch (const PvlogException& exception) {
		PVLOG_EXCEPT(std::string("Store ac data: ") + exception.what());
	}
}

void SqlDatabase::storeDc(const Dc& dc, uint32_t id)
{
	try {
		beginTransaction();

		for (int i = 0; i < dc.trackerNum; i++) {
			prepare("INSERT INTO tracker(num, inverter, date, power, voltage, current)\n"
			        "VALUES(:num, :inverter, :date, :power, :voltage, :current);");
			bindValueAdd(i);
			bindValueAdd(static_cast<int64_t> (id));
			bindValueAdd(static_cast<int32_t> (dc.time));

			if (Dc::isValid(dc.power[i])) bindValueAdd(dc.power[i]);
			else bindValueAdd();

			if (Dc::isValid(dc.voltage[i])) bindValueAdd(dc.voltage[i]);
			else bindValueAdd();

			if (Dc::isValid(dc.current[i])) bindValueAdd(dc.current[i]);
			else bindValueAdd();

			execQuery();
		}

		commitTransaction();
	} catch (const PvlogException& exception) {
		PVLOG_EXCEPT(std::string("Store dc data: ") + exception.what());
	}
}

std::vector<std::pair<uint32_t, uint32_t> > SqlDatabase::readAc(uint32_t id,
                                                                int line,
                                                                Type type,
                                                                const DateTime& from,
                                                                const DateTime& to)
{
	std::vector<std::pair<uint32_t, uint32_t> > acData;

	try {
		std::string select = "SELECT date, ";
		switch (type) {
		case POWER:
			select += "power\n";
			break;
		case VOLTAGE:
			select += "voltage\n";
			break;
		case CURRENT:
			select += "current\n";
			break;
		default:
			PVLOG_EXCEPT("Invalid Type!");
		}

		prepare(select + "FROM line\n"
		        "WHERE date >= :from AND date <= :to AND inverter = :id AND line = :line\n"
		        "ORDER BY date ASC;");
		bindValueAdd(static_cast<int32_t> (from.unixTime()));
		bindValueAdd(static_cast<int32_t> (to.unixTime()));
		bindValueAdd(static_cast<int64_t> (id));
		bindValueAdd(line);
		execQuery();

		while (next()) {
			uint32_t time = static_cast<uint32_t> (getValue(0).getInt32());
			Value value = getValue(1);

			if (!value.isNull()) {
				acData.push_back(std::make_pair(time, static_cast<uint32_t> (value.getInt32())));
			}
		}
	} catch (const PvlogException& exception) {
		PVLOG_EXCEPT(std::string("GetDc: ") + exception.what());
	}

	return acData;
}

std::vector<std::pair<uint32_t, uint32_t> > SqlDatabase::readDc(uint32_t id,
                                                                int trackerNum,
                                                                Type type,
                                                                const DateTime& from,
                                                                const DateTime& to)
{
	std::vector<std::pair<uint32_t, uint32_t> > dcData;

	try {
		std::string select = "SELECT date, ";
		switch (type) {
		case POWER:
			select += "power\n";
			break;
		case VOLTAGE:
			select += "voltage\n";
			break;
		case CURRENT:
			select += "current\n";
			break;
		default:
			PVLOG_EXCEPT("Invalid DcLineType!");
		}

		prepare(select + "FROM tracker\n"
		        "WHERE date >= :from AND date <= :to AND inverter = :id AND num = :num\n"
		        "ORDER BY date ASC;");
		bindValueAdd(static_cast<int32_t> (from.unixTime()));
		bindValueAdd(static_cast<int32_t> (to.unixTime()));
		bindValueAdd(static_cast<int64_t> (id));
		bindValueAdd(trackerNum);
		execQuery();

		while (next()) {
			uint32_t time = static_cast<uint32_t> (getValue(0).getInt32());
			Value value = getValue(1);

			if (!value.isNull()) {
				dcData.push_back(std::make_pair(time, static_cast<uint32_t> (value.getInt32())));
			}
		}
	} catch (const PvlogException& exception) {
		PVLOG_EXCEPT(std::string("GetDc: ") + exception.what());
	}

	return dcData;
}

void SqlDatabase::storeStats(const Stats& stats, uint32_t id)
{
    try {
        beginTransaction();
        prepare("UPDATE inverter\n"
                "SET\n"
                "total_power = coalesce(:total_power, total_power),\n"
                "operation_time = coalesce(:operation_time, operation_time),\n"
                "feed_in_time = coalesce(:feed_in_time, feed_in_time)\n"
                "WHERE id = :id;");
        if (Stats::isValid(stats.totalYield)) bindValueAdd(static_cast<int32_t> (stats.totalYield));
        else bindValueAdd();

        if (Stats::isValid(stats.operationTime)) bindValueAdd(
                static_cast<int32_t> (stats.operationTime));
        else bindValueAdd();

        if (Stats::isValid(stats.feedInTime)) bindValueAdd(static_cast<int32_t> (stats.feedInTime));
        else bindValueAdd();

        bindValueAdd(static_cast<int64_t> (id));
        execQuery();

        //day Value
        if (Stats::isValid(stats.dayYield)) {
            DateTime time;
            prepare("INSERT INTO day_values(inverter, julian_day, power)\n"
                    "VALUES(:inverter, :julian_day :power);");
            bindValueAdd(static_cast<int64_t> (id));
            bindValueAdd(static_cast<int32_t> (time.julianDay()));
            bindValueAdd(static_cast<int32_t> (stats.totalYield));

            execQuery();
        }

        commitTransaction();
    } catch (const PvlogException& exception) {
        PVLOG_EXCEPT(std::string("StoreStats: ") + exception.what());
    }

}

std::vector<std::pair<DateTime, uint32_t>> SqlDatabase::readDayPower(uint32_t id,
                                                                    const DateTime& fromDay,
                                                                    const DateTime& toDay) {
    std::vector<std::pair<DateTime, uint32_t> > dayValues;

    try {
        prepare("selectpower FROM day_values\n"
                "WHERE julian_day >= :from AND julian_day < :to AND inverter = :id\n"
                "ORDER BY date ASC;");
        bindValueAdd(static_cast<int32_t>(fromDay.julianDay()));
        bindValueAdd(static_cast<int32_t>(toDay.julianDay()));
        bindValueAdd(static_cast<int64_t> (id));
        execQuery();

        while (next()) {
            time_t time = static_cast<uint32_t> (getValue(0).getInt32());
            Value value = getValue(1);
            assert(!value.isNull());

            DateTime date(time);
            dayValues.push_back(std::make_pair(date, static_cast<uint32_t> (value.getInt32())));

        }
    } catch (const PvlogException& exception) {
        PVLOG_EXCEPT(std::string("GetDc: ") + exception.what());
    }

    return dayValues;
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
 execQuery();

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
 execQuery();
 }

 if (stats.operationTime != 0xffffffff) {
 prepare("UPDATE string_inverter SET operation_time = :time,"
 "WHERE id = :id;");
 execQuery();
 bindValueAdd(stats.operationTime);
 bindValueAdd(stats.id);
 execQuery();
 }

 if (stats.feedInTime != 0xffffffff) {
 prepare("UPDATE string_inverter SET feed_in_time = : time,"
 "WHERE id = :id;");
 bindValueAdd(stats.feedInTime);
 bindValueAdd(stats.id);
 execQuery();
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
 execQuery();

 commitTransaction();
 }

 int SqlDatabase::numInverter()
 {
 prepare("SELECT count(*) As inverter_num from string_inverter"
 "WHERE plant = :id;");
 bindValueAdd(id);
 execQuery();
 // }

 return getValue(0).getInt();
 }

 int SqlDatabase::numInverter(int32_t plantId)
 {
 prepare("SELECT count(*) As inverter_num from string_inverter"
 "WHERE plant = :id;");
 bindValueAdd(id);
 execQuery();

 return getValue(0).getInt();
 }

 void SqlDatabase::getInverters(std::vector< std::pair<int32_t, std::string> > & inverters, int plantId)
 {
 if (plantId < 0) {
 exec("SELECT id, name FROM string_inverter;");
 } else {
 prepare("SELECT id, name FROM string_inverter WHERE id = :id");
 bindValueAdd(plantId);
 execQuery();
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
 execQuery();

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
 execQuery();

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
 execQuery();

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
 execQuery();

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
 execQuery();

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
 execQuery();

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
