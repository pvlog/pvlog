#ifndef SRC_PVLOG_PVLIBHELPER_H_
#define SRC_PVLOG_PVLIBHELPER_H_

#include <unordered_map>
#include <unordered_set>
#include <cstdint>

struct pvlib_plant;

std::unordered_map<std::string, uint32_t> getConnections();

std::unordered_map<std::string, uint32_t> getProtocols();

std::unordered_set<int64_t> getInverters(pvlib_plant* plant);

pvlib_plant* connectPlant(const std::string& protocol,
                          const std::string& connection,
                          const std::string& connectionParam,
                          const std::string& protocolParam);

#endif /* SRC_PVLOG_PVLIBHELPER_H_ */
