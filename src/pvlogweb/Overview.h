#ifndef OVERVIEW_H
#define OVERVIEW_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include <cstdint>

#include <Database.h>
#include <ctemplate/template.h>

#include "UrlParser.h"


class Database;

class Overview {
private:
	UrlParser urlParser;
	Database* database;
	ctemplate::TemplateDictionary* content;

	struct Data {
		typedef typename std::set<std::string> Plants; //key: = plant, value = Inverter
		typedef typename std::map<std::string, std::vector<Database::Inverter > > LogicalPlants; //key = logicalPlant, value = Inverter

		Plants plants; //key: plant value: inverters
		LogicalPlants logicalPlants; //key: logical plant value: inverters
	};

	Data readData();

	void fillTemplate(const Data& data);

public:
	Overview(UrlParser urlPrser, Database* database, ctemplate::TemplateDictionary* content);

	void handleRequest();
};

#endif //#ifndef OVERVIEW_H
