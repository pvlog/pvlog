#ifndef SRC_PVLOG_PVLIBHELPER_H_
#define SRC_PVLOG_PVLIBHELPER_H_


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

#include "utility.h"

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

std::string to_string(const pvlib_ac& ac);

std::string to_string(const pvlib_dc& dc);

std::string to_string(const pvlib_status& status);

std::string to_string(const pvlib_stats& stats);

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
