#include "Overview.h"

#include <Database.h>

#include <vector>

#include <ctemplate/template.h>

using std::vector;
using std::string;
using std::map;
using namespace ctemplate;

Overview::Overview(UrlParser urlPrser, Database* database, TemplateDictionary* content) :
	urlParser(urlParser),
	database(database),
	content(content)
{
	//nothing to do
}

Overview::Data Overview::readData()
{
	Data data;

	vector<Database::Inverter> inverters = database->inverters();
	for (vector<Database::Inverter>::const_iterator it = inverters.begin();
			it != inverters.end(); ++it) {
		Data::LogicalPlants::iterator logicalPlantIt = data.logicalPlants.find(it->logicalPlant);
		if (logicalPlantIt == data.logicalPlants.end()) {
			vector<Database::Inverter> inverters;
			inverters.push_back(*it);
			data.logicalPlants.insert(std::make_pair(it->logicalPlant, inverters));
		} else {
			logicalPlantIt->second.push_back(*it);
		}

		data.plants.insert(it->plant);
	}

	//TODO: insert plants without inverters

	return data;
}

void Overview::fillTemplate(const Data& data)
{
	for (Data::LogicalPlants::const_iterator plantIt = data.logicalPlants.begin();
			plantIt != data.logicalPlants.end(); ++plantIt) {
		TemplateDictionary* logicalPlant = content->AddSectionDictionary("LOGICAL_PLANTS");

		logicalPlant->SetValue("LOGICAL_PLANT_ID", plantIt->first);
		logicalPlant->SetValue("LOGICAL_PLANT_NAME", plantIt->first); //id = name

		for (vector<Database::Inverter >::const_iterator invIt = plantIt->second.begin(); invIt != plantIt->second.end(); ++invIt) {
			TemplateDictionary* inverter = logicalPlant->AddSectionDictionary("INVERTERS");
			inverter->SetIntValue("INVERTER_ID", invIt->id);
			inverter->SetValue("INVERTER_NAME", invIt->name);
			inverter->SetValue("PLANT_NAME",invIt->plant);
		}
	}

	for (Data::Plants::const_iterator it = data.plants.begin(); it != data.plants.end(); ++it) {
		TemplateDictionary* plant = content->AddSectionDictionary("PLANTS");
		plant->SetValue("PLANT_ID", *it);
		plant->SetValue("PLANT_NAME", *it); //id = name
	}
}

void Overview::handleRequest() {
	Data data;
	data = readData();
	fillTemplate(data);
}
