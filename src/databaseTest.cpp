#include <iostream>
#include <vector>
#include "SqliteDatabase.h"
#include "SunriseSunset.h"
#include <gtest/gtest.h>


static void printUsage()
{
    std::cout << " pvlog [config file]" << std::endl;
}




class DatabaseTest : public ::testing::Test {
protected:
	std::string path;
	virtual void SetUp()
	{
		path = "/home/benjamin/test.db";
		database = NULL;
		database = new SqliteDatabase();
		database->open(path, "", "", "", "");
	}

	virtual void TearDown()
	{
		database->close();
		delete database;
		if (remove(path.c_str()) < 0) {
			perror("remove");

		}
	}

	Database *database;
};

TEST_F(DatabaseTest, DatbaseOpen) {
	ASSERT_TRUE(database != NULL);
}

TEST_F(DatabaseTest, createSchema)
{
	database->createSchema();
}

TEST_F(DatabaseTest, DatabaseCheckVersion)
{
	database->createSchema();
	ASSERT_TRUE(database->checkDatabase());
}


TEST_F(DatabaseTest, StoreAc) {
	database->createSchema();
	ASSERT_TRUE(database->checkDatabase());

	Database::Ac ac;
	memset(&ac, 0, sizeof(ac));

	for (int i = 0; i < 10; i++) {
		ac.time += 60 * 5;
		database->storeAc(ac, 0x0f0f0f0f);
	}

	memset(&ac, 0, sizeof(ac));

	for (int i = 0; i < 10; i++) {
		ac.time += 60 * 5;
		database->storeAc(ac, 0x0f0f0f00);
	}
}

TEST_F(DatabaseTest, StoreDc) {
	database->createSchema();
	ASSERT_TRUE(database->checkDatabase());

	Database::Dc dc;
	memset(&dc, 0, sizeof(dc));

	for (int i = 0; i < 10; i++) {
		dc.time += 60 * 5;
		database->storeDc(dc, 0x0f0f0f0f);
	}

	memset(&dc, 0, sizeof(dc));

	for (int i = 0; i < 10; i++) {
		dc.time += 60 * 5;
		database->storeDc(dc, 0x0f0f0f00);
	}
}


TEST_F(DatabaseTest, StoreStats) {
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

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();

/*	Database* database = new SqliteDatabase();
	database->open("/home/benjamin/test.db", "", "", "", "");
	database->createSchema();
	Database::Ac ac;
	memset(&ac, 0, sizeof(ac));

	for (int i = 0; i < 10; i++) {
		ac.time += 60 * 5;
		database->storeAc(ac, 0x0f0f0f0f);
	}

	memset(&ac, 0, sizeof(ac));

	for (int i = 0; i < 10; i++) {
		ac.time += 60 * 5;
		database->storeAc(ac, 0x0f0f0f00);
	}
*/

}
