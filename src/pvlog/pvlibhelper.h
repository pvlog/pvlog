#ifndef SRC_PVLOG_PVLIBHELPER_H_
#define SRC_PVLOG_PVLIBHELPER_H_

#include <unordered_map>
#include <unordered_set>
#include <cstdint>

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <iterator>
#include <iostream>
#include <type_traits>
#include <limits>

#include <pvlib/pvlib.h>

#include <utility.h>

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


struct Ac : public pvlib_ac {
	time_t time;

	Ac()  {
		totalPower = invalid<int32_t>();
		for (size_t i = 0; i < sizeof(power) / sizeof(power[0]); ++i) {
			power[i]   = invalid<int32_t>();
			voltage[i] = invalid<int32_t>();
			current[i] = invalid<int32_t>();
		}
		phaseNum   = 0;
		frequency  = invalid<int32_t>();
		time = 0;
	}

	friend std::ostream& operator <<(std::ostream& o, const Ac& ac) {
		o << "power: " << ac.totalPower << "W, frequency: " << ac.frequency
				<< "mHz\n";
		for (int i = 0; i < ac.phaseNum; ++i) {
			o << i << ": power: " << ac.power[i] << "W, voltage: "
					<< ac.voltage[i] << "mV, current: " << ac.current[i]
					<< "mA\n";
		}

		return o;
	}
};

struct Dc : public pvlib_dc {
	time_t time;

	Dc() {
		totalPower = invalid<int32_t>();
		for (size_t i = 0; i < sizeof(power) / sizeof(power[0]); ++i) {
			power[i]   = invalid<int32_t>();
			voltage[i] = invalid<int32_t>();
			current[i] = invalid<int32_t>();
		}
		trackerNum = 0;
		time       = 0;
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

struct Stats : public pvlib_stats {
	time_t time;

	Stats() {
		totalYield    = invalid<int64_t>();
		dayYield      = invalid<int64_t>();
		operationTime = invalid<int64_t>();
		feedInTime    = invalid<int64_t>();
		time = 0;
	}
};

struct Status : pvlib_status {
	Status() {
		number = -1;
	}

	friend std::ostream& operator <<(std::ostream& o, const Status& status)
	{
		o << "number: " << status.number << " status: " << status.status;
		return o;
	}
};
} //namespace pvlib {


struct pvlib_plant;

std::unordered_map<std::string, uint32_t> getConnections();

std::unordered_map<std::string, uint32_t> getProtocols();

std::unordered_set<int64_t> getInverters(pvlib_plant* plant);

pvlib_plant* connectPlant(const std::string& protocol,
                          const std::string& connection,
                          const std::string& connectionParam,
                          const std::string& protocolParam);

#endif /* SRC_PVLOG_PVLIBHELPER_H_ */
