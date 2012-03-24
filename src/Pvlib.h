#ifndef PVLIB_H_
#define PVLIB_H_

#include <string>
#include <map>
#include <set>
#include <vector>
#include <iterator>

#include <pvlib/pvlib.h>

#include "Utility.h"

class Pvlib {
	DISABLE_COPY(Pvlib)
private:
	typedef std::map<std::string, std::pair<pvlib_plant_t*, std::set<uint32_t> > > PlantMap;
	PlantMap plants;
	std::map<std::string, uint32_t> protocols;
	std::map<std::string, uint32_t> connections;

	pvlib_plant_t* plantHandle(const std::string& plantName, uint32_t inverterId) const;

public:
	class const_iterator : public std::iterator<std::input_iterator_tag, std::pair<int, uint32_t> > {
		friend class Pvlib;

		PlantMap::const_iterator plantCurrent;
		PlantMap::const_iterator plantEnd;
		std::set<uint32_t>::const_iterator inverterCurrent;
		std::set<uint32_t>::const_iterator inverterEnd;

		const_iterator(const PlantMap::const_iterator& plantBegin, const PlantMap::const_iterator& plantEnd) :
			plantCurrent(plantBegin),
			plantEnd(plantEnd)
		{
			if (plantCurrent != plantEnd) inverterCurrent = plantCurrent->second.second.begin();
		}

		pvlib_plant_t *plant() const
		{
			return plantCurrent->second.first;
		}

		uint32_t inverter() const
		{
			return *inverterCurrent;
		}

	public:
		uint32_t operator*()
		{
			return *inverterCurrent;
		}

		const const_iterator& operator++()
		{
			if (plantCurrent == plantEnd) return *this;
			if (inverterCurrent != inverterEnd) {
				++inverterCurrent;
				return *this;
			}
			while (inverterCurrent == inverterEnd) {
				if (plantCurrent == plantEnd) return *this;
				inverterCurrent = (++plantCurrent)->second.second.begin();
			}
			return *this;
		}

		inline std::pair<std::string, uint32_t> operator*() const
		{
			return std::make_pair(plantCurrent->first, *inverterCurrent);
		}

		bool operator==(const const_iterator& it) const
		{
			if (plantCurrent == it.plantCurrent) {
				if ((plantCurrent == plantEnd) || (inverterCurrent == it.inverterCurrent)) //Fixme: inverterEnd check!
					return true;
			}
			return false;
		}

		bool operator !=(const const_iterator& it) const {
			return !operator==(it);
		}
	};

	struct Ac {
		static const int32_t INVALID = 0x80000000;

		static bool isValid(int32_t type) { return !(type & 0x80000000); } //higest bit set => invalid

		uint32_t totalPower;

		int32_t power[3];
		int32_t voltage[3];
		int32_t current[3];

		uint8_t lineNum;

		int32_t frequence;

		time_t   time;
	};

	struct Dc {
		static const int32_t INVALID = 0x80000000;

		static bool isValid(int32_t type) { return !(type & 0x80000000); } //higest bit set => invalid
		uint32_t totalPower;

		int32_t power[3];
		int32_t voltage[3];
		int32_t current[3];

		uint8_t trackerNum;

		time_t  time;
	};

	struct Stats {
		static bool isValid(int32_t type) { return !(type & 0x80000000); } //higest bit set => invalid
    	uint32_t    totalYield; ///<total produced power in  watt-hours
		uint32_t    dayYield;   ///<total produced power today in  watt-hours

    	uint32_t    operationTime; /// <operation time in seconds
    	uint32_t    feedInTime;   ///<feed in time in seconds

    	time_t      time;
	};

	//struct Stats {
	//	static bool
	//}

public:
	/**
	 * @param fd fd used for logging.
	 */
    explicit Pvlib(FILE *fd);

    ~Pvlib();

	/**
	 * Returns all supported connection types.
	 *
	 * @return connection with id and name.
	 */
    std::set<std::string> supportedConnections() const;

	/**
	 * Returns all supported protocoltypes.
	 *
	 * @return protocol with id and name.
	 */
    std::set<std::string> supportedProtocols() const;

	/**
	 * Opens a plant (one or multiple inverters).
	 *
	 * @param plantId artifical id to identify plant, must be different for every plant.
	 * @param connection one of the different ids returned by supportedConnections.
	 * @param address plant/inverter address (protocol dependant).
	 * @param param ConArg Additional argument(s) (connection dependant).
	 * @param protocol one of the different ids returned by supportedProtocols.
	 * @param passwd Password for plant.
	 * @param protocolArg Additional argument(s) (protocol depandant).
	 */
    void openPlant(const std::string& name,
				   const std::string& connection,
				   const std::string& protocol,
				   const std::string& address,
				   const std::string& passwd = "",
				   const void* conArg = NULL,
				   const void* protocolArg = NULL);

    /**
     * Close plant specified by id
     *
     * @param plantId id of plant see openPlant(int, uint32_t, const std::string&, const void*, uint32_t).
     */
	void closePlant(const std::string& name);

	/**
	 * Close all plants.
	 */
	void close();

	/**
	 * Returns all open plants.
	 *
	 * @return
	 */
    std::vector<std::string> openPlants();

    /**
     * Returns all inverter associated with plant.
     *
     * @param plantId id assigned to plant.
     * @return all inverters assigned to plant.
     */
     const std::set<uint32_t>& inverters(const std::string& plantName) const;

	/**
	 * Retrivies AC specific values.
	 *
	 * @param[out] ac
	 * @param plantId id assigned to plant, when opening it.
	 * @param inverterId serial/id of inverter.
	 */
    void getAc(Ac* ac, const std::string& plant, uint32_t inverterId);

    /**
     * Overloaded version of getAc.
     *
     * @param[out] ac
     * @param iterator
     */
     void getAc(Ac *ac, const Pvlib::const_iterator& iterator);

	/**
	 * Retrivies DC specific values.
	 *
	 * @param[out] dc
	 * @param plantId id assigned to plant, when opening it.
	 * @param inverterId serial/id of inverter.
	 */
    void getDc(Dc* dc, const std::string& plant, uint32_t inverterId);

	/**
	 * Overloaded version of getDc.
	 *
	 * @param[out] dc
	 * @param iterator
	 */
    void getDc(Dc *dc, const Pvlib::const_iterator& iterator);

	/**
	 * Retrivies Statistic specific values.
	 *
	 * @param[out] stats statistics of inverter.
	 * @param plantId plant id assigned to plant.
	 * @param inverterId serial/id of inverter.
	 */
    void getStats(Stats* stats, const std::string& plant, uint32_t inverterId);

    void getStats(Stats* stats, const Pvlib::const_iterator& iterator);

	/**
	 * Retrivies Status specific values.
	 *
	 * @param[out] status status of inverter.
	 * @param plantId plant id assigned to plant.
	 * @param inverterId serial/id of inverter.
	 */
//    void getStatus(Status* status, uint32_t & id, int num);

	//iterator interface
	const_iterator begin() const
	{
		return const_iterator(plants.begin(), plants.end());
	}

	const_iterator end() const
	{
		return const_iterator(plants.end(), plants.end());
	}
};

#endif /* #ifndef PVLIB_H_ */
