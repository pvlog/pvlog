#include "Pvlib.h"

#include <utility>

#include "Utility.h"
#include "PvlogException.h"
#include "Log.h"

namespace pvlib {

Pvlib::Pvlib(FILE *fd) {
	pvlib_init(fd);

	int conNum = pvlib_connection_num();
	int protNum = pvlib_protocol_num();

	//cache supported protocols and connections
	if (conNum != 0) {
		std::vector<uint32_t> conIds(conNum);
		pvlib_connections(&conIds[0], conNum);

		for (std::vector<uint32_t>::const_iterator i = conIds.begin();
				i != conIds.end(); ++i) {
			LOG(Info) << "pvlib connection: " << pvlib_connection_name(*i);
			connections.insert(std::make_pair(pvlib_connection_name(*i), *i));
		}
	}
	if (protNum != 0) {
		std::vector<uint32_t> protIds(protNum);
		pvlib_protocols(&protIds[0], protNum);

		for (std::vector<uint32_t>::const_iterator i = protIds.begin();
				i != protIds.end(); ++i) {
			LOG(Info) << "pvlib protocol: " << pvlib_protocol_name(*i);
			protocols.insert(std::make_pair(pvlib_protocol_name(*i), *i));
		}
	}
}

Pvlib::~Pvlib() {
	close();
	pvlib_shutdown();
}

pvlib_plant_t* Pvlib::plantHandle(const std::string& plantName,
		uint32_t inverterId) const
{
	PlantMap::const_iterator it = plants.find(plantName);

	if (it == plants.end())
		return NULL;

	return it->second;
}

std::unordered_set<std::string> Pvlib::supportedConnections() const {
	NameIdMap::const_iterator beg = connections.begin();
	NameIdMap::const_iterator end = connections.end();

	return std::unordered_set<std::string>(
			util::const_key_iterator<NameIdMap>(beg),
			util::const_key_iterator<NameIdMap>(end));
}

std::unordered_set<std::string> Pvlib::supportedProtocols() const {
	NameIdMap::const_iterator beg = protocols.begin();
	NameIdMap::const_iterator end = protocols.end();

	return std::unordered_set<std::string>(
			util::const_key_iterator<NameIdMap>(beg),
			util::const_key_iterator<NameIdMap>(end));
}

void Pvlib::openPlant(const std::string& plantName,
                      const std::string& connection,
                      const std::string& protocol,
                      const void* connectionParam, const void* protocolParam)
{
	if (plants.count(plantName) == 1)
		PVLOG_EXCEPT("plant name already in use!");

	NameIdMap::const_iterator con;
	NameIdMap::const_iterator prot;

	if ((con = connections.find(connection)) == connections.end())
		PVLOG_EXCEPT(std::string("Invalid connection: ") + connection);

	if ((prot = protocols.find(protocol)) == protocols.end())
		PVLOG_EXCEPT(std::string("Invalid protocol: ") + protocol);

	pvlib_plant_t *plant = pvlib_open(con->second, prot->second,
			connectionParam, protocolParam);
	if (plant == NULL)
		PVLOG_EXCEPT("Error opening plant!");

	plants.emplace(plantName, plant);
}

void Pvlib::connect(const std::string& plantName,
                    const std::string& address,
                    const std::string& passwd,
                    const void* connectionParam,
                    const void* protocolParam)
{
	const auto& plantIt = plants.find(plantName);
	if (plantIt == plants.end()) {
		PVLOG_EXCEPT("Invalid plantName");
	}

	if (pvlib_connect(plantIt->second, address.c_str(), passwd.c_str(),
			connectionParam, protocolParam) < 0) {
		PVLOG_EXCEPT("Error connecting to plant");
	}

	int inverterNum;
	if ((inverterNum = pvlib_num_string_inverter(plantIt->second)) < 0) {
		PVLOG_EXCEPT("Error retrieving inverter number!");
	}

	std::vector<uint32_t> inverters;
	inverters.resize(inverterNum);

	if (pvlib_device_handles(plantIt->second, inverters.data(),
			inverters.size()) < 0) {
		PVLOG_EXCEPT("Error retrieving inverters!");
	}

	std::unordered_set<int64_t> inverterSet(inverters.begin(), inverters.end());

	connectedPlants.emplace(plantIt->second, inverterSet);
}

void Pvlib::disconnect() {
	for (const auto& connectedPlant : connectedPlants) {
		pvlib_disconnect(connectedPlant.first);
	}
	connectedPlants.clear();
}

std::vector<std::string> Pvlib::openPlants() {
	PlantMap::const_iterator beg = plants.begin();
	PlantMap::const_iterator end = plants.end();
	return std::vector<std::string>(util::const_key_iterator<PlantMap>(beg),
			util::const_key_iterator<PlantMap>(end));
}

const Pvlib::Inverters& Pvlib::getInverters(const std::string& plantName) const {
	PlantMap::const_iterator it = plants.find(plantName);
	if (it == plants.end()) {
		PVLOG_EXCEPT(std::string("No plant with plantId: ") + plantName);
	}

	InverterMap::const_iterator inverterIt = connectedPlants.find(it->second);
	if (inverterIt == connectedPlants.end()) {
		PVLOG_EXCEPT(std::string("Plant not connected") + plantName);
	}

	return inverterIt->second;
}

void Pvlib::closePlant(const std::string& plantName) {
	PlantMap::iterator it;
	it = plants.find(plantName);
	if (it == plants.end())
		PVLOG_EXCEPT("Unknown plant id.");

	pvlib_close(it->second);
	plants.erase(it);
}

void Pvlib::close() {
	disconnect();

	for (PlantMap::const_iterator i = plants.begin(); i != plants.end(); ++i) {
		pvlib_close(i->second);
	}
	plants.clear();
}

void Pvlib::getDc(Dc* dc, const std::string& plantName, uint32_t inverterId) {
	pvlib_plant_t* plant = plantHandle(plantName, inverterId);
	if (plant == NULL)
		PVLOG_EXCEPT("Could not find plant!");

	if (pvlib_get_dc_values(plant, inverterId, (pvlib_dc_t*) dc) < 0)
		PVLOG_EXCEPT("Error reading dc parameters!");
}

void Pvlib::getDc(Dc *dc, const Pvlib::const_iterator& iterator) {
	pvlib_plant_t* plant = iterator.plant();
	uint32_t inverter = iterator.inverter();
	if (pvlib_get_dc_values(plant, inverter, (pvlib_dc_t*) dc) < 0)
		PVLOG_EXCEPT("Error reading dc parameters!");
}

void Pvlib::getAc(Ac* ac, const std::string& plantName, uint32_t inverterId) {
	pvlib_plant_t* plant = plantHandle(plantName, inverterId);
	if (plant == NULL)
		PVLOG_EXCEPT("Could not find plant!");

	if (pvlib_get_ac_values(plant, inverterId, (pvlib_ac_t*) ac) < 0)
		PVLOG_EXCEPT("Error reading ac parameters");
}

void Pvlib::getAc(Ac *ac, const Pvlib::const_iterator& iterator) {
	pvlib_plant_t* plant = iterator.plant();
	uint32_t inverter = iterator.inverter();
	if (pvlib_get_ac_values(plant, inverter, (pvlib_ac_t*) ac) < 0)
		PVLOG_EXCEPT("Error reading dc parameters!");
}

void Pvlib::getStats(Stats* stats, const std::string& plantName, uint32_t inverterId) {
	pvlib_plant_t* plant = plantHandle(plantName, inverterId);
	if (plant == NULL)
		PVLOG_EXCEPT("Could not find plant!");

	if (pvlib_get_stats(plant, inverterId, (pvlib_stats_t*) stats) < 0)
		PVLOG_EXCEPT("Error reading statistics of inverter");
}

void Pvlib::getStats(Stats* stats, const Pvlib::const_iterator& iterator) {
	pvlib_plant_t* plant = iterator.plant();
	uint32_t inverter = iterator.inverter();
	if (pvlib_get_stats(plant, inverter, (pvlib_stats_t*) stats) < 0)
		PVLOG_EXCEPT("Error reading statistics of inverter");
}

void Pvlib::getStatus(Status* status, const std::string& plantName, uint32_t inverterId) {
	pvlib_plant_t* plant = plantHandle(plantName, inverterId);
	if (plant == NULL)
		PVLOG_EXCEPT("Could not find plant!");

	pvlib_status_t pvlib_status;

	if (pvlib_get_status(plant, inverterId, &pvlib_status) < 0)
		PVLOG_EXCEPT("Error reading status of inverter");

	status->number = pvlib_status.number;
	status->status = pvlib_status.status;
}

void Pvlib::getStatus(Status* status, const Pvlib::const_iterator& iterator) {
	pvlib_plant_t* plant = iterator.plant();
	uint32_t inverter = iterator.inverter();

	pvlib_status_t pvlib_status;
	if (pvlib_get_status(plant, inverter, &pvlib_status) < 0)
		PVLOG_EXCEPT("Error reading statistics of inverter");

	status->number = pvlib_status.number;
	status->status = pvlib_status.status;
}

} //namespace pvlib {

