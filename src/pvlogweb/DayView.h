#ifndef DAYVIEW_H
#define DAYVIEW_H

#include <map>

#include "UrlParser.h"
#include <Database.h>
#include <ctemplate/template.h>

class DayView {
private:
	enum Side {
		AC, DC
	};

	struct Request {
		bool hasOptions; // we got options

		int year;
		int month;
		int day;
		Side side;
		Database::Type type;
		int line;
		std::vector<std::pair<uint32_t, std::vector<int> > > inverters; //inverter and tracker/phase
	};

	struct InverterData {
		uint32_t id;
		int line;
		std::string name;
		std::vector<std::pair<uint32_t, uint32_t> > values; //vector of (time, data) pairs
	};

	struct Data {
		Side side;
		std::vector<InverterData> inverterData;
	};

	UrlParser urlParser;

	Database* database;

	ctemplate::TemplateDictionary* dict;


	Request parseRequest();

	Data readData(const Request& request);

	InverterData readInverterData(uint32_t id, int line, const DateTime& from, const DateTime& to,
	                              Side side, Database::Type type);

	void fillDictionary(const Data& data);

public:
	DayView(UrlParser& urlParser, Database* database, ctemplate::TemplateDictionary* dict);

	void handleRequest();
};

#endif // #ifndef DAYVIEW_H
