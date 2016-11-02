#include "pvlibhelper.h"

#include <vector>
#include <pvlib.h>

#include "Log.h"
#include "PvlogException.h"


std::unordered_map<std::string, uint32_t>  getConnections() {
	std::unordered_map<std::string, uint32_t> connections;
	int conNum = pvlib_connection_num();

	if (conNum != 0) {
		std::vector<uint32_t> conIds(conNum);
		pvlib_connections(&conIds[0], conNum);

		for (std::vector<uint32_t>::const_iterator i = conIds.begin();
				i != conIds.end(); ++i) {
			LOG(Info) << "pvlib connection: " << pvlib_connection_name(*i);
			connections.insert(std::make_pair(pvlib_connection_name(*i), *i));
		}
	}

	return connections;
}

std::unordered_map<std::string, uint32_t> getProtocols() {
	std::unordered_map<std::string, uint32_t> protocols;
	int protNum = pvlib_protocol_num();

	if (protNum != 0) {
		std::vector<uint32_t> protIds(protNum);
		pvlib_protocols(&protIds[0], protNum);

		for (std::vector<uint32_t>::const_iterator i = protIds.begin();
				i != protIds.end(); ++i) {
			LOG(Info) << "pvlib protocol: " << pvlib_protocol_name(*i);
			protocols.insert(std::make_pair(pvlib_protocol_name(*i), *i));
		}
	}

	return protocols;
}

std::unordered_set<int64_t> getInverters(pvlib_plant* plant) {
	int inverterNum;
	if ((inverterNum = pvlib_num_string_inverter(plant)) < 0) {
		PVLOG_EXCEPT("Error retrieving inverter number!");
	}

	std::vector<uint32_t> inverters;
	inverters.resize(inverterNum);

	if (pvlib_device_handles(plant, inverters.data(),
			inverters.size()) < 0) {
		PVLOG_EXCEPT("Error retrieving inverters!");
	}

	return std::unordered_set<int64_t>(inverters.begin(), inverters.end());
}
