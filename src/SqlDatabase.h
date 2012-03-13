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

class SqlDatabase : public Database {
private:
	static const int VERSION = 0;
    std::string         statment;
    std::vector<Value>  values;
protected :
    virtual void exec(std::vector<Value> & values) = 0;

    inline void bindValueAdd(const Value& value)
    {
        values.push_back(value);
    }

    inline void bindValueAdd()
    {
        values.push_back(Value());
    }

    inline void exec()
    {
        exec(values);
    }

    virtual void exec(const std::string& statment)
    {
        prepare(statment);
        std::vector<Value> values;
        exec(values);
    }

    virtual void beginTransaction() = 0;

    virtual void commitTransaction() = 0;

    virtual void prepare(const std::string& statment) = 0;

    virtual bool next() = 0;

    virtual bool step() = 0;

    virtual Value getValue(int index) = 0;
public:

	SqlDatabase();

    virtual ~SqlDatabase();

    virtual Location location(const std::string& logicalPlant);

	virtual void open(const std::string& database,
	                  const std::string& hostname,
	                  const std::string& port,
	                  const std::string& username,
					  const std::string& password) = 0;

	virtual void close() = 0;

    virtual void createSchema();

    virtual bool checkDatabase();

    virtual void storeConfig(const std::string& value, const std::string& data);

    virtual std::string readConfig(const std::string& key);

	virtual void addPlant(const std::string& name,
	                      const std::string& connection,
	                      const std::string& conParam1,
	                      const std::string& conParam2,
	                      const std::string& protocol,
	                      const std::string& password);

	virtual std::vector<Plant> plants();

	virtual void addLogicalPlant(const std::string& name,
								 const Location& location,
								 float declination,
								 float orientation);


	virtual void addInverter(uint32_t id,
	                         const std::string& name,
	                         const std::string& plant,
	                         const std::string& logicalPlant,
	                         int32_t wattPeak);


    virtual void storeAc(const Ac& ac, uint32_t id);
/*
    virtual std::vector<Ac> getAc(uint32_t id, const Date& date) = 0;

    virtual std::vector< std::pair<Date, int32_t> > getEnergie(const std::string& logicalPlant, const Date& from, const Date& to, Resolution) = 0;
*/
    virtual void storeDc(const Dc& dc, uint32_t id);

    //virtual std::vector<Dc> getDc(uint32_t id, const Date& date) = 0;

    virtual void storeStats(const Stats& stats, uint32_t id);

  /**
     * Returns all plants in our database.
     *
     * @param[out] plant map with inverter id as key.
     */
   // virtual std::vector<int, std::string> getPlants();



};

#endif // DATABASE_H
