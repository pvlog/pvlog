#include "Pvlib.h"

#include <utility>

#include "Utility.h"
#include "PvlogException.h"

Pvlib::Pvlib(FILE *fd)
{
    pvlib_init(fd);

	int conNum  = pvlib_connection_num();
	int protNum = pvlib_protocol_num();

	//cache supported protocols and connections
	if (conNum != 0) {
		std::vector<uint32_t> conIds(conNum);
		pvlib_connections(&conIds[0], conNum);

		for (std::vector<uint32_t>::const_iterator i = conIds.begin(); i != conIds.end(); ++i) {
			connections.insert(std::make_pair(pvlib_connection_name(*i), *i));
		}
	}
	if (protNum != 0) {
		std::vector<uint32_t> protIds(protNum);
		pvlib_protocols(&protIds[0], protNum);

		for (std::vector<uint32_t>::const_iterator i = protIds.begin(); i != protIds.end(); ++i) {
			protocols.insert(std::make_pair(pvlib_protocol_name(*i), *i));
		}
	}
}

Pvlib::~Pvlib()
{
	close();
    pvlib_shutdown();
}

pvlib_plant_t* Pvlib::plantHandle(const std::string& plantName, uint32_t inverterId) const
{
	PlantMap::const_iterator it = plants.find(plantName);

	if (it == plants.end()) return NULL;
	if (it->second.second.count(inverterId) == 0) return NULL;

	return it->second.first;
}

std::set<std::string> Pvlib::supportedConnections() const
{
	std::map<std::string, uint32_t>::const_iterator beg = connections.begin();
	std::map<std::string, uint32_t>::const_iterator end = connections.end();

	return std::set<std::string>(const_key_iterator< std::map<std::string, uint32_t> >(beg),
	                             const_key_iterator< std::map<std::string, uint32_t> >(end));
}

std::set<std::string> Pvlib::supportedProtocols() const
{
	std::map<std::string, uint32_t>::const_iterator beg = protocols.begin();
	std::map<std::string, uint32_t>::const_iterator end = protocols.end();

	return std::set<std::string>(const_key_iterator< std::map<std::string, uint32_t> >(beg),
	                             const_key_iterator< std::map<std::string, uint32_t> >(end));
}

void Pvlib::openPlant(const std::string& plantName,
	                  const std::string& connection,
                      const std::string& protocol,
                      const std::string& addr,
                      const std::string& passwd,
                      const void* conArg,
                      const void* protocolArg)
{
	if (plants.count(plantName) == 1)
		PVLOG_EXCEPT("plant allready in use!");

	std::map<std::string, uint32_t>::const_iterator con;
	std::map<std::string, uint32_t>::const_iterator prot;

	if ((con = connections.find(connection)) == connections.end())
		PVLOG_EXCEPT(std::string("Invalid connection: ") + connection);

	if ((prot = protocols.find(protocol)) == protocols.end())
		PVLOG_EXCEPT(std::string("Invalid protocol: ") + protocol);

	pvlib_plant_t *plant = pvlib_open(con->second, addr.c_str(), conArg, prot->second);
	if (plant == NULL)
		PVLOG_EXCEPT("Error opening plant!");

	if (pvlib_connect(plant, passwd.c_str(), protocolArg) < 0) {
		pvlib_close(plant);
		PVLOG_EXCEPT("Error connectiong to plant");
	}

	plants.insert(std::make_pair(plantName, std::make_pair(plant, std::set<uint32_t>())));
}

std::vector<std::string> Pvlib::openPlants()
{
	PlantMap::const_iterator beg = plants.begin();
	PlantMap::const_iterator end = plants.end();
	return std::vector<std::string>(const_key_iterator<PlantMap>(beg),
	                                const_key_iterator<PlantMap>(end));
}

const std::set<uint32_t>& Pvlib::inverters(const std::string& plantName) const
{
	PlantMap::const_iterator it = plants.find(plantName);
	if (it == plants.end())
		PVLOG_EXCEPT("No plant with plantId: "/* + plantName */);

	return it->second.second;
}

void Pvlib::closePlant(const std::string& plantName)
{
    PlantMap::iterator it;
    it = plants.find(plantName);
    if (it == plants.end())
        PVLOG_EXCEPT("Unknown plant id.");

    pvlib_close(it->second.first);
    plants.erase(it);
}

void Pvlib::close()
{
	for (PlantMap::const_iterator i = plants.begin(); i != plants.end(); ++i) {
		pvlib_close(i->second.first);
	}
	plants.clear();
}

void Pvlib::getDc(Dc* dc, const std::string& plantName, uint32_t inverterId)
{
	pvlib_plant_t* plant = plantHandle(plantName, inverterId);
	if (plant == NULL) PVLOG_EXCEPT("Could not find plant!");

	if (pvlib_get_dc_values(plant, inverterId, (pvlib_dc_t*)dc) < 0)
		PVLOG_EXCEPT("Error reading dc parameters!");
}

void Pvlib::getDc(Dc *dc, const Pvlib::const_iterator& iterator)
{
	pvlib_plant_t* plant = iterator.plant();
	uint32_t inverter    = iterator.inverter();
	if (pvlib_get_dc_values(plant, inverter, (pvlib_dc_t*)dc) < 0)
		PVLOG_EXCEPT("Error reading dc parameters!");
}

void Pvlib::getAc(Ac* ac, const std::string& plantName, uint32_t inverterId)
{
	pvlib_plant_t* plant = plantHandle(plantName, inverterId);
	if (plant == NULL) PVLOG_EXCEPT("Could not find plant!");

	if (pvlib_get_ac_values(plant, inverterId, (pvlib_ac_t*)ac) < 0)
		PVLOG_EXCEPT("Error reading ac parameters");
}

void Pvlib::getAc(Ac *ac, const Pvlib::const_iterator& iterator)
{
	pvlib_plant_t* plant = iterator.plant();
	uint32_t inverter    = iterator.inverter();
	if (pvlib_get_ac_values(plant, inverter, (pvlib_ac_t*)ac) < 0)
		PVLOG_EXCEPT("Error reading dc parameters!");
}

void Pvlib::getStats(Stats* stats, const std::string& plantName, uint32_t inverterId)
{
	pvlib_plant_t* plant = plantHandle(plantName, inverterId);
	if (plant == NULL) PVLOG_EXCEPT("Could not find plant!");

	if (pvlib_get_stats(plant, inverterId, (pvlib_stats_t*)stats) < 0)
		PVLOG_EXCEPT("Error reading statistics of inverter");
}

void Pvlib::getStats(Stats* stats, const Pvlib::const_iterator& iterator)
{
	pvlib_plant_t* plant = iterator.plant();
	uint32_t inverter    = iterator.inverter();
	if (pvlib_get_stats(plant, inverter, (pvlib_stats_t*)stats) < 0)
		PVLOG_EXCEPT("Error reading statistics of inverter");
}
