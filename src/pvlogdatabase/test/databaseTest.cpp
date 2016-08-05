#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <fstream>
#include "SqliteDatabase.h"
#include "DateTime.h"
#include <gtest.h>

//TEST_DIR_PATH is defined by cmake
const static char* PATH = TEST_DIR_PATH"/testdatabase.db";

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

TYPED_TEST(DatabaseTest, DatabaseAddPlant)
{
	Database* database = this->database;
	database->createSchema();
	ASSERT_TRUE(database->checkDatabase());

	database->addPlant("plant1", "connection1", "conParam11", "conParam12", "protocol1", "123");
	database->addPlant("plant2", "connection2", "conParam12", "conParam22", "protocol2", "1234");

	std::vector<Database::Plant> plants = database->plants();
	ASSERT_EQ(plants.size(), 2u);

	if (plants.at(0).name ==  "plant1") {
		ASSERT_EQ("connection1", plants.at(0).connection);
		ASSERT_EQ("conParam11", plants.at(0).conParam1);
		ASSERT_EQ("conParam12", plants.at(0).conParam2);
		ASSERT_EQ("protocol1", plants.at(0).protocol);
		ASSERT_EQ("123", plants.at(0).password);
	} else {
		ASSERT_EQ("plant2", plants.at(1).name);
		ASSERT_EQ("connection2", plants.at(1).connection);
		ASSERT_EQ("conParam21", plants.at(1).conParam1);
		ASSERT_EQ("conParam22", plants.at(1).conParam2);
		ASSERT_EQ("protocol2", plants.at(1).protocol);
		ASSERT_EQ("1234", plants.at(1).password);
	}
}

TYPED_TEST(DatabaseTest, DatabaseAddLogicalPlant)
{
	Database* database = this->database;
	database->createSchema();
	ASSERT_TRUE(database->checkDatabase());

	database->addLogicalPlant("lplant1", 0, 30, 90, 180);
	database->addLogicalPlant("lplant2",12, 40, 45, 0);

	std::vector<Database::LogicalPlant> plants = database->logicalPlants();
	ASSERT_EQ(plants.size(), 2u);

	if (plants.at(0).name == "lplants1") {
		ASSERT_EQ(0, plants.at(0).longitude);
		ASSERT_EQ(30, plants.at(0).latitude);
		ASSERT_EQ(90, plants.at(0).declination);
		ASSERT_EQ(180, plants.at(0).orientation);
	} else {
		ASSERT_EQ(12, plants.at(1).longitude);
		ASSERT_EQ(40, plants.at(1).latitude);
		ASSERT_EQ(45, plants.at(1).declination);
		ASSERT_EQ(0, plants.at(1).orientation);
	}


}

TYPED_TEST(DatabaseTest, DatabaseAddInverter)
{
	Database* database = this->database;
	database->createSchema();
	ASSERT_TRUE(database->checkDatabase());

	database->addPlant("plant1", "connection1", "conParam11", "conParam21", "protocol1", "123");
	database->addLogicalPlant("lplant1", 10, 30, 90, 180);

	database->addInverter(0, "inverter1", "plant1", "lplant1", 5000, 1, 2);
	database->addInverter(0xffffffff, "inverter1", "plant1", "lplnat1", 4000, 3, 4);

	std::vector<Database::Inverter> inverters = database->inverters();

	ASSERT_EQ(inverters.size(), 2u);
	if (inverters.at(0).id == 0) {
		ASSERT_EQ(0u, inverters.at(0).id);
		ASSERT_EQ("lplant1", inverters.at(0).logicalPlant);
		ASSERT_EQ("inverter1", inverters.at(0).name);
		ASSERT_EQ("plant1", inverters.at(0).plant);
		ASSERT_EQ(5000, inverters.at(0).wattpeak);
		ASSERT_EQ(1, inverters.at(0).phaseCount);
		ASSERT_EQ(2, inverters.at(0).trackerCount);
	} else {
		ASSERT_EQ(0xffffffffu, inverters.at(1).id);
		ASSERT_EQ("lplant1", inverters.at(1).logicalPlant);
		ASSERT_EQ("inverter2", inverters.at(1).name);
		ASSERT_EQ("plant1", inverters.at(1).plant);
		ASSERT_EQ(4000, inverters.at(1).wattpeak);
		ASSERT_EQ(3, inverters.at(1).phaseCount);
		ASSERT_EQ(5, inverters.at(1).trackerCount);
	}
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();

}
