#ifndef PVLIB_H_
#define PVLIB_H_

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <iterator>
#include <iostream>
#include <type_traits>
#include <limits>

#include <pvlib.h>

#include "Utility.h"

namespace pvlib {

//declare invalid data

//unsigned data are invalid if value is maximum
template <typename T, typename std::enable_if<std::is_unsigned<T>::value, int>::type = 0>
T invalid() {
	return std::numeric_limits<T>::max();
}

//signed data is invalid if value is minimum
template <typename T, typename std::enable_if<std::is_signed<T>::value, int>::type = 0>
T invalid() {
	return std::numeric_limits<T>::min();
}

//check if pvlib data is valid
//unsigned data are invalid if value is maximum
template <typename T, typename std::enable_if<std::is_unsigned<T>::value, int>::type = 0>
T isValid(T value) {
	return value != invalid<T>();
}

//signed data is invalid if value is minimum
template <typename T, typename std::enable_if<std::is_signed<T>::value, int>::type = 0>
T isValid(T value) {
	return value != invalid<T>();
}


struct Ac {
	uint32_t totalPower;

	uint32_t power[3];
	uint32_t voltage[3];
	uint32_t current[3];

	uint8_t lineNum;
	uint32_t frequency;
	time_t time;

	Ac() :
		totalPower(invalid<uint32_t>()),
		power{invalid<uint32_t>()},
		voltage{invalid<uint32_t>()},
		current{invalid<uint32_t>()},
		lineNum(0),
		frequency(invalid<uint32_t>()),
		time(0) {
	}

	friend std::ostream& operator <<(std::ostream& o, const Ac& ac) {
		o << "power: " << ac.totalPower << "W, frequency: " << ac.frequency
				<< "mHz\n";
		for (int i = 0; i < ac.lineNum; ++i) {
			o << i << ": power: " << ac.power[i] << "W, voltage: "
					<< ac.voltage[i] << "mV, current: " << ac.current[i]
					<< "mA\n";
		}

		return o;
	}
};

struct Dc {
	uint32_t totalPower;

	uint32_t power[3];
	uint32_t voltage[3];
	uint32_t current[3];

	uint8_t trackerNum;
	time_t time;

	Dc() :
		totalPower(invalid<uint32_t>()),
		power{invalid<uint32_t>()},
		voltage{invalid<uint32_t>()},
		current{invalid<uint32_t>()},
		trackerNum(0),
		time(0) {
	}
	friend std::ostream& operator <<(std::ostream& o, const Dc& dc) {
		o << "power: " << dc.totalPower << "W\n";
		for (int i = 0; i < dc.trackerNum; ++i) {
			o << i << ": power: " << dc.power[i] << "W, voltage: "
					<< dc.voltage[i] << "mV, current: " << dc.current[i]
					<< "mA\n";
		}

		return o;
	}
};

struct Stats {
	uint32_t totalYield; ///<total produced power in  watt-hours
	uint32_t dayYield; ///<total produced power today in  watt-hours

	uint32_t operationTime; /// <operation time in seconds
	uint32_t feedInTime; ///<feed in time in seconds

	time_t time;

	Stats() {
		totalYield = -1;
		dayYield = -1;
		operationTime = -1;
		feedInTime = -1;
		time = 0;
	}
};

struct Status {
	int status;
	uint32_t number;

	Status() {
		number = -1;
	}

	friend std::ostream& operator <<(std::ostream& o, const Status& status)
	{
		o << "number: " << status.number << " status: " << status.status;
		return o;
	}
};


class Pvlib {
private:
	typedef std::unordered_map<std::string, pvlib_plant_t*> PlantMap;
    typedef std::unordered_set<int64_t> Inverters;
	typedef std::unordered_map<pvlib_plant_t*, Inverters> InverterMap;

	PlantMap plants;
	InverterMap connectedPlants;

	typedef std::unordered_map<std::string, uint32_t> NameIdMap;
	NameIdMap protocols;
	NameIdMap connections;

	pvlib_plant_t* plantHandle(const std::string& plantName, uint32_t inverterId) const;

public:
	class const_iterator: public std::iterator<std::input_iterator_tag, std::pair<int, uint32_t> > {
		friend class Pvlib;

		InverterMap::const_iterator plantCurrent;
		InverterMap::const_iterator plantEnd;
		Inverters::const_iterator inverterCurrent;
		Inverters::const_iterator inverterEnd;

		const_iterator(const InverterMap::const_iterator& plantBegin,
		               const InverterMap::const_iterator& plantEnd) :
			plantCurrent(plantBegin), plantEnd(plantEnd)
		{
			if (plantCurrent != plantEnd) {
				inverterCurrent = plantCurrent->second.begin();
				inverterEnd = plantCurrent->second.end();
			}
		}

		pvlib_plant_t *plant() const
		{
			return plantCurrent->first;
		}

		uint32_t inverter() const
		{
			return *inverterCurrent;
		}

	public:
		const const_iterator& operator++()
		{
			if (plantCurrent == plantEnd) {
				return *this;
			} else {
				if (inverterCurrent != inverterEnd) {
					++inverterCurrent;
				}
			}

			while (inverterCurrent == inverterEnd) {
				if (plantCurrent == plantEnd) {
					return *this;
				} else {
					++plantCurrent;
					if (plantCurrent == plantEnd) return *this;
					inverterCurrent = plantCurrent->second.begin();
				}
			}
			return *this;
		}

		inline uint32_t operator*() const
		{
			return *inverterCurrent;
		}
		/*
		 inline std::pair<std::string, uint32_t> operator*() const
		 {
		 return std::make_pair(plantCurrent->first, *inverterCurrent);
		 }
		 */
		bool operator==(const const_iterator& it) const
		{
			if (plantCurrent == it.plantCurrent) {
				if ((plantCurrent == plantEnd) || (inverterCurrent == it.inverterCurrent)) return true;
			}
			return false;
		}

		bool operator !=(const const_iterator& it) const
		{
			return !operator==(it);
		}
	};

public:
    DISABLE_COPY(Pvlib)
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
	std::unordered_set<std::string> supportedConnections() const;

	/**
	 * Returns all supported protocol types.
	 *
	 * @return protocol with id and name.
	 */
	std::unordered_set<std::string> supportedProtocols() const;

	/**
	 * Opens a plant (one or multiple inverters).
	 *
	 * @param plantId artificial id to identify plant, must be different for every plant.
	 * @param connection one of the different ids returned by supportedConnections.
	 * @param address plant/inverter address (protocol dependent).
	 * @param param ConArg Additional argument(s) (connection dependent).
	 * @param protocol one of the different ids returned by supportedProtocols.
	 * @param passwd Password for plant.
	 * @param protocolArg Additional argument(s) (protocol dependent).
	 */
	void openPlant(const std::string& name,
	               const std::string& connection,
	               const std::string& protocol,
	               const void* conArg = NULL,
	               const void* protocolArg = NULL);

	/**
	 * Connect to plant.
	 *
	 * @param plantName name of plane to connect to
	 * @param conParam param dependent on protocol.
	 */
	void connect(const std::string& plantName,
	             const std::string& address,
                 const std::string& passwd,
                 const void* connectionParam = NULL,
                 const void* protocolParam = NULL);

	/**
	 * Disconnect plant
	 */
	void disconnect(const std::string& plantName);

	/**
	 * Disconnect all plants
	 */
	void disconnect();

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
	const Inverters& getInverters(const std::string& plantName) const;

	/**
	 * Retrieves AC specific values.
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
	 * Retrieves DC specific values.
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
	 * Retrieves Statistic specific values.
	 *
	 * @param[out] stats statistics of inverter.
	 * @param plantId plant id assigned to plant.
	 * @param inverterId serial/id of inverter.
	 */
	void getStats(Stats* stats, const std::string& plant, uint32_t inverterId);

	void getStats(Stats* stats, const Pvlib::const_iterator& iterator);


	void getStatus(Status* status, const std::string& plant, uint32_t inverterId);

	void getStatus(Status* status, const Pvlib::const_iterator& iterator);

	/**
	 * Retrieves Status specific values.
	 *
	 * @param[out] status status of inverter.
	 * @param plantId plant id assigned to plant.
	 * @param inverterId serial/id of inverter.
	 */
	//    void getStatus(Status* status, uint32_t & id, int num);

public:
	//iterator interface
	const_iterator begin() const
	{
		return const_iterator(connectedPlants.begin(), connectedPlants.end());
	}

	const_iterator end() const
	{
		return const_iterator(connectedPlants.end(), connectedPlants.end());
	}
};

} //namespace pvlib{

#endif /* #ifndef PVLIB_H_ */
