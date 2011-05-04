#ifndef DATA_STORAGE_H
#define DATA_STORAGE_H

#include <ctime>
#include <string>
#include <stdint.h>
#include <cstring>
#include <vector>
#include <map>

class Date;
class Database;
class Query;

class DataStorage {
private :
    Database        *database;
    Query           *query;
    std::string     databaseType;
public :
    struct Ac {
        int32_t    power[3];
        int32_t    voltage[3];
        int32_t    current[3];

        int8_t     lineCount;
        int64_t    id;
        time_t     time;
    };

    struct Dc {
        int32_t     power[3];
        int32_t     voltage[3];
        int32_t     current[3];

        int32_t     trackerCount;
        int64_t     id;
        time_t      time;
    };

    struct Stats {
        int32_t     energieToday;
        int64_t     energieTotal;
        int64_t     operationTime;
        int64_t     feedInTime;
        int64_t     id;

        time_t      time;
    };

    struct Status {
        std::string status;
        int64_t     id;
        time_t      time;
    };

    enum Type {
        POWER,
        CURRENT,
        VOLTAGE
    };

    DataStorage(const std::string & databaseType);

    ~DataStorage();

    void openDatabase(const std::string & database,
                      const std::string & hostname,
                      const std::string & port,
                      const std::string & username,
                      const std::string & password);

    void createSchema();

    /**
     * Returns all plants in our database.
     *
     * @param[out] plant map with inverter id as key.
     */
    void getPlants(std::map<int64_t, std::string> & plants);

    /**
     * Retrivies coordinates of plant.
     */
    bool getPlantCoordinate(int64_t id, double & longitude, double & latitude);

    /**
     * Retrives total nummber of string inverter.
     *
     * @return number of inverter.
     */
    int numInverter(int id = -1);

    /**
     * Returns id and list of all inverters.
     *
     * @param[out] inverters list of inverters.
     */
    void getInverters(std::vector< std::pair<int64_t, std::string> > & inverters, int plantId = -1);

    /**
     * Return statistics of inverter.
     *
     * @param[out] stats statistic of inverter, unknown value will be set t -1.
     */
    void getInverterStats(int64_t id, Stats & stats);


  //  void addInverter(int64_t id, std::string & name, int32_t wattpeak);

    /**
     * Returns energie per month for given year and string inverter.
     *
     * @param[out] power
     */
    void getMonthValues(int64_t id, int year, std::vector< std::pair<int, int32_t> > & power);

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

    /**
     * Returns ac values for given date and id.
     */
    void getAc(int64_t id, int phase, const Date & date, std::vector< std::pair<time_t, int32_t > > & ac, Type type);


    //void getAcPlant(int id, const Date & date, std::vector< std::pair<time_t, int32_t > > & ac, Type type);

    void storeAc(const Ac & ac);

    void storeDc(const Dc & dc);

    void storeStats(const Stats & stats);

       // void storeStatus(const );

        //void storeValues(const Values<std::string> & values);

        //const std::vector<std::string> & getStorabelValues() ;
};

#endif // #ifndef DATA_STORAGE_H
