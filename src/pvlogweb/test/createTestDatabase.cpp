#include <cstdlib>

#include <SqliteDatabase.h>
#include <Log.h>

#define INVERTER_NUM 2
#define DAYS 60
#define STORAGE_INTERVAL (10 * 60)

int main(int argc, char* argv[])
{
	if (argc != 2) {
		LOG(Error) << "Usage: createTestDatabase <path>";
		return EXIT_FAILURE;
	}

	struct ExecWrapper : public SqliteDatabase {
		void exec(const std::string& str)
		{
			SqlDatabase::exec(str);
		}
	};

	Database* database = new ExecWrapper();
	database->open(argv[1], "", "", "", "");
	static_cast<ExecWrapper*>(database)->exec("PRAGMA synchronous = OFF");
	database->createSchema();

	database->addPlant("testPlant", "", "", "", "", "");

	Database::Location location(0.0, 0.0);
	database->addLogicalPlant("testPlant", 10, 10, 45, 180);

	for (int i = 0; i < INVERTER_NUM; ++i) {
		database->addInverter(i, "inverter", "testPlant", "testPlant", 5000, 1 + i, i + 1);
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
	start = mktime(&use);
	 dayBeg = start;
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

}
