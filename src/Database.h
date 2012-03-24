#ifndef DATA_STORAGE_H
#define DATA_STORAGE_H

#include <ctime>
#include <string>
#include <stdint.h>
#include <cstring>
#include <vector>
#include <map>

#include "Pvlib.h"
#include "Utility.h"
#include "DateTime.h"


class Database {
	//DISABLE_COPY(Database)
public:
	typedef Pvlib::Dc Dc;
	typedef Pvlib::Ac Ac;
	typedef Pvlib::Stats Stats;

	enum Type {
		POWER,
		VOLTAGE,
		CURRENT
	};


	enum Resolution {
		MINUTE,
		MONTH,
		DAY,
		YEAR
	};

	struct Plant {
		std::string name;
		std::string connection;
		std::string conParam1;
		std::string conParam2;
		std::string protocol;
		std::string password;
	};

	struct Location {
		Location(float longitude, float latitude) :
			longitude(longitude),
			latitude(latitude) { /* nothing to do */}
		float longitude;
		float latitude;
	};

	Database() { /* nothing to do */ }

    virtual ~Database() { /* nothing to do */ }

    virtual void open(const std::string& database,
	                  const std::string& hostname,
	                  const std::string& port,
	                  const std::string& username,
	                  const std::string& password) = 0;

	virtual void close() = 0;

	/**
	 * Create Database schema.
	 */
    virtual void createSchema() = 0;

    /**
     * Check Database version
     *
     * @return true if everything ok, else false.
     */
     virtual bool checkDatabase() = 0;

    /**
     * Stores Configurations in database
     *
     * @param value
     * @param data
	 */
    virtual void storeConfig(const std::string& key, const std::string& data) = 0;

    virtual std::string readConfig(const std::string& key) = 0;

    /**
     * Retrive logical plant location.
     *
     * @return Location of logical plant
     */
     virtual Location location(const std::string& logicalPlant) = 0;

    /**
     * Add plant to Database
     */
	virtual void addPlant(const std::string& name,
	                      const std::string& connection,
	                      const std::string& conParam1,
	                      const std::string& conParam2,
	                      const std::string& protocol,
	                      const std::string& password) = 0;

	/**
	 * Returns all plants in database.
	 */
	virtual std::vector<Plant> plants() = 0;

	virtual void addLogicalPlant(const std::string& name,
								 const Location& location,
								 float declination,
								 float orientation) = 0;

    /**
	 * Add a new inverter to plant associated with plant specified by plantId.
	 *
	 */
	virtual void addInverter(uint32_t id,
	                         const std::string& name,
	                         const std::string& plant,
	                         const std::string& logicalPlant,
	                         int32_t wattPeak) = 0;

	/**
	 * Store ac values.
	 */
    virtual void storeAc(const Ac& ac, uint32_t id) = 0;

	/**
	 * Store dc side values.
	 */
    virtual void storeDc(const Dc& dc, uint32_t id) = 0;

    /**
     * Store statistics of inverter.
     */
    virtual void storeStats(const Stats& stats, uint32_t id) = 0;

    virtual std::vector< std::pair<uint32_t, uint32_t> > readAc(uint32_t id,
															    int line,
															    Type type,
															    const DateTime& from,
															    const DateTime& to) = 0;

    virtual std::vector< std::pair<uint32_t, uint32_t> > readDc(uint32_t id,
															    int trackerNum,
															    Type type,
															    const DateTime& from,
															    const DateTime& to) = 0;

    //virtual std::vector<Stats> getStats(uint32_t id, const Time& time) = 0;


  /**
     * Returns all plants in our database.
     *
     * @param[out] plant map with inverter id as key.
     */
    //virtual std::vector<int, std::string> getPlants() = 0;


#if 0
    void storeStats(const Stats& stats, uint32_t id);


	/**
     * Returns ac values for given date and id.
     */
    void getAc(int64_t id, int phase, const Date & date, std::vector< std::pair<time_t, int32_t > > & ac, Type type);


    /**
     * Retrives total nummber of string inverter.
     *
     * @return number of inverter.
     */
    int numInverter();

    /**
     * Returns id and list of all inverters.
     *
     * @param[out] inverters list of inverters.
     */
    std::map<uint32_t, std::string> getInverters();

    /**
     * Return statistics of inverter.
     *
     * @param id inverter serial/id.
     * @param[out] stats statistic of inverter.
     */
    void getInverterStats(uint32_t id, Stats& stats);


  //  void addInverter(int64_t id, std::string & name, int32_t wattpeak);

    /**
     * Returns energie per month for given year and string inverter.
     *
     * @param[out] power
     */
    void getMonthValues(uint32_t id, int year, std::vector< std::pair<int, int32_t> > & power);

    /**
     * Returns energie per month for given year and plant.
     *
     * @param[out] power
     */
    void getMonthValuesPlant(int id, int year, std::vector< std::pair<int, int32_t> > & power);


    /**
     * Returns energie per year for givem string inverter.
     *
     * @param[out] power
     */
    void getYearValues(int64_t id, int fromYear, int toYear, std::vector< std::pair<int, int32_t> > & power);

    /**
     * Returns energie per year for given plant.
     *
     * @param[out] power
     */
    void getYearValuesPlant(int id, int fromYear, int toYear, std::vector< std::pair<int, int32_t> > & power);

    /**
     * Returns line outs of a given inverter.
     *
     * @return number of phases.
     */
     int getPhaseCount(int64_t id);
#endif

       // void storeStatus(const );

        //void storeValues(const Values<std::string> & values);

        //const std::vector<std::string> & getStorabelValues() ;
};

#endif // #ifndef DATA_STORAGE_H
