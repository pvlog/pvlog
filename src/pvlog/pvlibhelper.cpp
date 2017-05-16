/*
 * This file is part of Pvlog.
 *
 * Copyright (C) 2017 pvlogdev@gmail.com
 *
 * Pvlog is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Pvlog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Pvlog.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "pvlibhelper.h"

#include <vector>

#include "log.h"
#include "pvlogexception.h"

std::string to_string(const pvlib_ac& ac) {
	std::ostringstream o;

	o << "power: " << ac.totalPower << "W, frequency: " << ac.frequency
			<< "mHz\n";
	for (int i = 0; i < ac.phaseNum; ++i) {
		o << i << ": power: " << ac.power[i] << "W, voltage: "
				<< ac.voltage[i] << "mV, current: " << ac.current[i]
				<< "mA\n";
	}

	return o.str();
}

std::string to_string(const pvlib_dc& dc) {
	std::ostringstream o;

	o << "power: " << dc.totalPower << "W\n";
	for (int i = 0; i < dc.trackerNum; ++i) {
		o << i << ": power: " << dc.power[i] << "W, voltage: "
				<< dc.voltage[i] << "mV, current: " << dc.current[i]
				<< "mA\n";
	}
	return o.str();
}

std::string to_string(const pvlib_status& status) {
	std::ostringstream o;
	o << "number: " << status.number << " status: " << status.status;
	return o.str();
}

std::string to_string(const pvlib_stats& stats) {
	std::ostringstream o;

	o << "totalYield: " << stats.totalYield << " dayYield: " << stats.dayYield
			<< " operationTime: " << stats.operationTime << " feedinTime: " << stats.feedInTime;

	return o.str();
}


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

//pvlib_plant* openPlant(const std::string& protocol, const std::string& connection) {
//	std::unordered_map<std::string, uint32_t> connections = getConnections();
//	std::unordered_map<std::string, uint32_t> protocols   = getProtocols();
//
//	LOG(Info) << "Opening plant " << plant.name << " ["
//					<< plant.connection << ", " << plant.protocol << "]";
//
//	auto connectionIt = connections.find(plant.connection);
//	if (connectionIt == connections.end()) {
//		PVLOG_EXCEPTION("Unsupported connection: " + plant.connection);
//	}
//
//	auto protocolIt = protocols.find(plant.protocol);
//	if (protocolIt == protocols.end()) {
//		PVLOG_EXCEPTION("Unsupported Protocol: " + plant.protocol);
//	}
//	//pvlib->openPlant(plant.name, plant.connection, plant.protocol);
//	pvlib_plant* pvlibPlant = pvlib_open(connectionIt->second, protocolIt->second, nullptr, nullptr);
//	if (pvlibPlant  == nullptr) {
//		PVLOG_EXCEPTION("Error opening plant!");
//	}
//
//	return plant;
//}

pvlib_plant* connectPlant(const std::string& connection,
                          const std::string& protocol,
                          const std::string& connectionParam,
                          const std::string& protocolParam) {
	std::unordered_map<std::string, uint32_t> connections = getConnections();
	std::unordered_map<std::string, uint32_t> protocols   = getProtocols();

//	LOG(Info) << "Opening plant " << plant.name << " ["
//					<< plant.connection << ", " << plant.protocol << "]";

	auto connectionIt = connections.find(connection);
	if (connectionIt == connections.end()) {
		PVLOG_EXCEPT("Unsupported connection: " + connection);
	}

	auto protocolIt = protocols.find(protocol);
	if (protocolIt == protocols.end()) {
		PVLOG_EXCEPT("Unsupported Protocol: " + protocol);
	}
	//pvlib->openPlant(plant.name, plant.connection, plant.protocol);
	pvlib_plant* pvlibPlant = pvlib_open(connectionIt->second, protocolIt->second, nullptr, nullptr);
	if (pvlibPlant  == nullptr) {
		PVLOG_EXCEPT("Error opening plant!");
	}

	if (pvlib_connect(pvlibPlant, connectionParam.c_str(), protocolParam.c_str(), nullptr, nullptr) < 0) {
		PVLOG_EXCEPT("Error connecting to plant!");
	}

	return pvlibPlant;
}
