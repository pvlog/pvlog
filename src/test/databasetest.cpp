#include <datetime.h>
#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <fstream>
#include "SqliteDatabase.h"
#include <gtest/gtest.h>

static const int STORAGE_INTERVAL = 60 * 5; //10 minutes
static const int DAYS = 2;
static const int INVERTER_NUM = 10;
static const char* PATH = "/home/benjamin/test.db";

template <typename T>
Database *createDatabase();

template <typename T>
void destroyDatabase(Database* database);

template <>
Database* createDatabase<SqliteDatabase>()
{
	struct ExecWrapper : public SqliteDatabase {
		void exec(const std::string& str)
		{
			SqlDatabase::execQuery(str);
		}
	};

	ExecWrapper* database = new ExecWrapper();
	database->open(PATH, "", "", "", "");
	database->exec("PRAGMA synchronous = OFF");
	return database;
}

template <>
void destroyDatabase<SqliteDatabase>(Database* database)
{
	database->close();
	delete database;

	if (remove(PATH) < 0) {
		perror("remove");
	}
}

template <typename T>
class DatabaseTest : public ::testing::Test {
protected:
	std::string path;
	virtual void SetUp()
	{
		database = NULL;
		database = createDatabase<T>();
	}

	virtual void TearDown()
	{
		destroyDatabase<T>(database);
	}

	Database *database;
};

using testing::Types;
typedef Types<SqliteDatabase> Implementations;
TYPED_TEST_CASE(DatabaseTest, Implementations);

/*
 TYPED_TEST(DatabaseTest, DatabaseCheckVersion)
 {
 Database* database = this->database;
 database->createSchema();
 ASSERT_TRUE(database->checkDatabase());
 }
 */
TYPED_TEST(DatabaseTest, DatabaseConfigStorage)
{
	Database* database = this->database;
	database->createSchema();
	ASSERT_TRUE(database->checkDatabase());

	database->storeConfig("config1", "data1");
	database->storeConfig("config2", "data2");

	std::string data1 = database->readConfig("config1");
	std::string data2 = database->readConfig("config2");

	ASSERT_EQ(std::string("data1"), data1);
ASSERT_EQ(std::string("data2") , data2);
}

TYPED_TEST(DatabaseTest, DatabaseAcStorage)
{
	Database* database = this->database;
	database->createSchema();
	ASSERT_TRUE(database->checkDatabase());

	database->addPlant("testPlant", "", "", "", "", "");

	Database::Location location(0.0, 0.0);
	database->addLogicalPlant("testPlant", location, 45, 180);

	for (int i = 0; i < INVERTER_NUM; ++i) {
		database->addInverter(i, "inverter", "testPlant", "testPlant", 5000);
	}

	time_t t = time(NULL);
	struct tm* gmt = gmtime(&t);
	struct tm use;
	memcpy(&use, gmt, sizeof(use));

	use.tm_hour = 7;
	use.tm_min = 0;
	use.tm_sec = 0;

	time_t start = mktime(&use);
	time_t dayBeg = start;
	for (int i = 0; i < DAYS; i++) {
		for (time_t t = dayBeg; t <= dayBeg + 12 * 3600; t += STORAGE_INTERVAL) {
			for (int id = 0; id < INVERTER_NUM; ++id) {
				Database::Ac ac;

				ac.lineNum = 3;

				ac.time = t;

				//Fill with some values
				ac.power[0] = (i + t) % 5000;
				ac.power[1] = (i + t + 1) % 5000;
				ac.power[2] = (i + t + 2) % 5000;

				ac.voltage[0] = (i + t + 10) % 5000;
				ac.voltage[1] = (i + t + 11) % 5000;
				ac.voltage[2] = (i + t + 12) % 5000;

				ac.current[0] = (i + t + 20) % 5000;
				ac.current[1] = (i + t + 21) % 5000;
				ac.current[2] = (i + t + 22) % 5000;

				ac.totalPower = ac.power[0] + ac.power[1] + ac.power[2] + 1;
				ac.frequence = 49000 + (i + t) % 2000;
				database->storeAc(ac, id);
			}
		}
		dayBeg += 24 * 3600;
	}

	dayBeg = start;
	for (int i = 0; i < DAYS; i++) {
		time_t dayEnd = dayBeg + 12 * 3600;
		for (int id = 0; id < INVERTER_NUM; ++id) {
			std::vector< std::pair<uint32_t, uint32_t> > ac;

			int num = (dayEnd - dayBeg) / STORAGE_INTERVAL + 1;

			for (int line = 0; line < 3; line++) {
				int storageTime;

				ac = database->readAc(id, line, Database::POWER, DateTime(dayBeg), DateTime(dayEnd));
				ASSERT_EQ(num, ac.size());

				storageTime = dayBeg;
				for (std::vector< std::pair<uint32_t, uint32_t> >::const_iterator it = ac.begin(); it < ac.end(); ++it) {
					ASSERT_EQ(storageTime, it->first);
					ASSERT_EQ(it->second, (i + storageTime + line) % 5000);
					storageTime += STORAGE_INTERVAL;
				}

				ac = database->readAc(id, line, Database::VOLTAGE, DateTime(dayBeg), DateTime(dayEnd));
				ASSERT_EQ(num, ac.size());

				storageTime = dayBeg;
				for (std::vector< std::pair<uint32_t, uint32_t> >::const_iterator it = ac.begin(); it < ac.end(); ++it) {
					ASSERT_EQ(storageTime, it->first);
					ASSERT_EQ(it->second, (i + storageTime + line + 10) % 5000);
					storageTime += STORAGE_INTERVAL;
				}

				ac = database->readAc(id, line, Database::CURRENT, DateTime(dayBeg), DateTime(dayEnd));
				ASSERT_EQ(num, ac.size());

				storageTime = dayBeg;
				for (std::vector< std::pair<uint32_t, uint32_t> >::const_iterator it = ac.begin(); it < ac.end(); ++it) {
					ASSERT_EQ(storageTime, it->first);
					ASSERT_EQ(it->second, (i + storageTime + line + 20) % 5000);
					storageTime += STORAGE_INTERVAL;
				}

				//ASSERT_EQ(it->totalPower, it->power[0] + it->power[1] + it->power[2] + 1);

				//ASSERT_EQ(it->frequence, 49000 + (i + storageTime) % 2000);

			}
		}
		dayBeg += 24 * 3600;
	}
}

TYPED_TEST(DatabaseTest, DatabaseDcStorage)
{
	Database* database = this->database;
	database->createSchema();
	ASSERT_TRUE(database->checkDatabase());

	database->addPlant("testPlant", "", "", "", "", "");

	Database::Location location(0.0, 0.0);
	database->addLogicalPlant("testPlant", location, 45, 180);

	for (int i = 0; i < INVERTER_NUM; ++i) {
		database->addInverter(i, "inverter", "testPlant", "testPlant", 5000);
	}

	time_t t = time(NULL);
	struct tm* gmt = gmtime(&t);
	struct tm use;
	memcpy(&use, gmt, sizeof(use));

	use.tm_hour = 7;
	use.tm_min = 0;
	use.tm_sec = 0;

	time_t start = mktime(&use);
	time_t dayBeg = start;
	for (int i = 0; i < DAYS; i++) {
		for (time_t t = dayBeg; t <= dayBeg + 12 * 3600; t += STORAGE_INTERVAL) {
			for (int id = 0; id < INVERTER_NUM; ++id) {
				Database::Dc dc;

				dc.trackerNum = 3;

				dc.time = t;

				//Fill with some values
				dc.power[0] = (i + t) % 1500;
				dc.power[1] = (i + t + 1) % 1500;
				dc.power[2] = (i + t + 2) % 1500;

				dc.voltage[0] = (i + t + 10) % 1500;
				dc.voltage[1] = (i + t + 11) % 1500;
				dc.voltage[2] = (i + t + 12) % 1500;

				dc.current[0] = (i + t + 20) % 1500;
				dc.current[1] = (i + t + 21) % 1500;
				dc.current[2] = (i + t + 22) % 1500;

				dc.totalPower = dc.power[0] + dc.power[1] + dc.power[2];
				database->storeDc(dc, id);
			}
		}
		dayBeg += 24 * 3600;
	}

	dayBeg = start;
	for (int i = 0; i < DAYS; i++) {
		time_t dayEnd = dayBeg + 12 * 3600;
		for (int id = 0; id < INVERTER_NUM; ++id) {
			std::vector< std::pair<uint32_t, uint32_t> > dc;

			int num = (dayEnd - dayBeg) / STORAGE_INTERVAL + 1;

			for (int trackerNum = 0; trackerNum < 3; trackerNum++) {
				int storageTime;

				dc = database->readDc(id, trackerNum, Database::POWER, DateTime(dayBeg), DateTime(dayEnd));
				ASSERT_EQ(num, dc.size());

				storageTime = dayBeg;
				for (std::vector< std::pair<uint32_t, uint32_t> >::const_iterator it = dc.begin(); it < dc.end(); ++it) {
					ASSERT_EQ(storageTime, it->first);
					ASSERT_EQ(it->second, (i + storageTime + trackerNum) % 1500);
					storageTime += STORAGE_INTERVAL;
				}

				dc = database->readDc(id, trackerNum, Database::VOLTAGE, DateTime(dayBeg), DateTime(dayEnd));
				ASSERT_EQ(num, dc.size());

				storageTime = dayBeg;
				for (std::vector< std::pair<uint32_t, uint32_t> >::const_iterator it = dc.begin(); it < dc.end(); ++it) {
					ASSERT_EQ(storageTime, it->first);
					ASSERT_EQ(it->second, (i + storageTime + trackerNum + 10) % 1500);
					storageTime += STORAGE_INTERVAL;
				}

				dc = database->readDc(id, trackerNum, Database::CURRENT, DateTime(dayBeg), DateTime(dayEnd));
				ASSERT_EQ(num, dc.size());

				storageTime = dayBeg;
				for (std::vector< std::pair<uint32_t, uint32_t> >::const_iterator it = dc.begin(); it < dc.end(); ++it) {
					ASSERT_EQ(storageTime, it->first);
					ASSERT_EQ(it->second, (i + storageTime + trackerNum + 20) % 1500);
					storageTime += STORAGE_INTERVAL;
				}

				//ASSERT_EQ(it->totalPower, it->power[0] + it->power[1] + it->power[2] + 1);

				//ASSERT_EQ(it->frequence, 49000 + (i + storageTime) % 2000);

			}
		}
		dayBeg += 24 * 3600;
	}
}

/*
 TEST_F(DatabaseTest, StoreStats)
 {
 Database::Stats stats;
 memset(&stats, 0, sizeof(stats));

 for (int i = 0; i < 10; i++) {
 stats.time += 60 * 5;
 database->storeStats(stats, 0x0f0f0f0f);
 }

 memset(&stats, 0, sizeof(stats));

 for (int i = 0; i < 10; i++) {
 stats.time += 60 * 5;
 database->storeStats(stats, 0x0f0f0f00);
 }
 }

 */
/*
 TYPED_TEST(DatabaseTest, complete30Year10InverterTest)
 {
 Database* database = this->database;
 database->createSchema();
 ASSERT_TRUE(database->checkDatabase());

 database->addPlant("testPlant", "", "", "", "", "");

 Database::Location location(0.0, 0.0);
 database->addLogicalPlant("testPlant", location, 45, 180);

 for (int i = 0; i < INVERTER_NUM; ++i) {
 database->addInverter(i, "inverter", "testPlant", "testPlant", 5000);
 }

 time_t t = time(NULL);
 struct tm* gmt = gmtime(&t);
 struct tm use;
 memcpy(&use, gmt, sizeof(use));

 use.tm_hour = 7;
 use.tm_min  = 0;
 use.tm_sec  = 0;

 time_t start = mktime(&use);
 time_t dayBeg = start;
 for (int i = 0; i < DAYS; i++) {
 for (time_t t = dayBeg; t <= dayBeg + 12 * 3600; t += STORAGE_INTERVAL) {
 for (int id = 0; id < INVERTER_NUM; ++id) {
 Database::Ac ac;
 Database::Dc dc;

 ac.lineNum = 3;

 ac.time = t;

 //Fill with some values
 ac.power[0] = (i + t) % 5000;
 ac.power[1] = (i + t + 1) % 5000;
 ac.power[2] = (i + t + 2) % 5000;

 std::cerr << "ins" << ac.power[0] << " " << ac.power[1] << " " << ac.power[2] << std::endl;

 ac.voltage[0] = (i + t + 10) % 5000;
 ac.voltage[1] = (i + t + 11) % 5000;
 ac.voltage[2] = (i + t + 12) % 5000;

 ac.current[0] = (i + t + 20) % 5000;
 ac.current[1] = (i + t + 21) % 5000;
 ac.current[2] = (i + t + 22) % 5000;

 ac.totalPower = ac.power[0] + ac.power[1] + ac.power[2] + 1;
 ac.frequence = 49000 + (i + t) % 2000;

 dc.trackerNum = 3;

 dc.time = t;

 //Fill with some values
 dc.power[0] = (i + t) % 1500;
 dc.power[1] = (i + t + 1) % 1500;
 dc.power[2] = (i + t + 2) % 1500;

 dc.voltage[0] = (i + t + 10) % 1500;
 dc.voltage[1] = (i + t + 11) % 1500;
 dc.voltage[2] = (i + t + 12) % 1500;

 dc.current[0] = (i + t + 20) % 1500;
 dc.current[1] = (i + t + 21) % 1500;
 dc.current[2] = (i + t + 22) % 1500;

 dc.totalPower = dc.power[0] + dc.power[1] + dc.power[2];

 database->storeAc(ac, id);
 database->storeDc(dc, id);
 }
 }
 dayBeg += 24 * 3600;
 }

 dayBeg = start;
 for (int i = 0; i < DAYS; i++) {
 time_t dayEnd = dayBeg + 12 * 3600;
 for (int id = 0; id < INVERTER_NUM; ++id) {
 std::vector<Database::Ac> ac;
 ac = database->getAc(id, DateTime(dayBeg), DateTime(dayEnd));

 int num = (dayEnd - dayBeg) / STORAGE_INTERVAL;
 ASSERT_EQ(num, ac.size());

 int storageTime = dayBeg;
 for (std::vector<Database::Ac>::const_iterator it = ac.begin(); it < ac.end(); ++it) {
 ASSERT_EQ(storageTime, it->time);
 ASSERT_EQ(it->lineNum, 3);

 ASSERT_EQ(it->power[0], (i + storageTime) % 5000);
 ASSERT_EQ(it->power[1], (i + storageTime + 1) % 5000);
 ASSERT_EQ(it->power[2], (i + storageTime + 2) % 5000);

 ASSERT_EQ(it->voltage[0], (i + storageTime + 10) % 5000);
 ASSERT_EQ(it->voltage[1], (i + storageTime + 11) % 5000);
 ASSERT_EQ(it->voltage[2], (i + storageTime + 12) % 5000);

 ASSERT_EQ(it->current[0], (i + storageTime + 20) % 5000);
 ASSERT_EQ(it->current[1], (i + storageTime + 21) % 5000);
 ASSERT_EQ(it->current[2], (i + storageTime + 22) % 5000);

 //ASSERT_EQ(it->totalPower, it->power[0] + it->power[1] + it->power[2] + 1);

 //ASSERT_EQ(it->frequence, 49000 + (i + storageTime) % 2000);

 storageTime += STORAGE_INTERVAL;

 }
 std::vector<Database::Dc> dc;
 dc = database->getDc(id, DateTime(dayBeg), DateTime(dayEnd));

 ASSERT_EQ(num, dc.size());

 storageTime = dayBeg;
 for (std::vector<Database::Dc>::const_iterator it = dc.begin(); it < dc.end(); ++it) {
 ASSERT_EQ(storageTime, it->time);
 ASSERT_EQ(it->trackerNum, 3);

 ASSERT_EQ(it->power[0], (i + storageTime) % 1500);
 ASSERT_EQ(it->power[1], (i + storageTime + 1) % 1500);
 ASSERT_EQ(it->power[2], (i + storageTime + 2) % 1500);

 ASSERT_EQ(it->voltage[0], (i + storageTime + 10) % 1500);
 ASSERT_EQ(it->voltage[1], (i + storageTime + 11) % 1500);
 ASSERT_EQ(it->voltage[2], (i + storageTime + 12) % 1500);

 ASSERT_EQ(it->current[0], (i + storageTime + 20) % 1500);
 ASSERT_EQ(it->current[1], (i + storageTime + 21) % 1500);
 ASSERT_EQ(it->current[2], (i + storageTime + 22) % 1500);

 storageTime += STORAGE_INTERVAL;
 }

 }

 dayBeg += 24 * 3600;
 }
 }
 */

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();

}
