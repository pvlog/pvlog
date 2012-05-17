#include "DayView.h"

#include <string>
#include <sstream>
#include <climits>
#include <vector>
#include <cstdint>

#include <ctemplate/template.h>

#include "PvlogException.h"

using std::vector;
using std::string;
using ctemplate::TemplateDictionary;

template<typename T>
static inline T convertTo(const std::string& str,
                          std::ios_base& (*base)(std::ios_base &) = std::dec,
                          bool failIfLeftoverChars = true)
{
	std::istringstream i(str);
	T ret;
	char c;
	if (!(i >> ret) || (failIfLeftoverChars && i.get(c))) {
		PVLOG_EXCEPT("convert " + str + " to " + string(typeid(T).name()) + " failed.");
	}

	return ret;
}

DayView::DayView(UrlParser& urlParser, Database* database, TemplateDictionary* dict) :
		urlParser(urlParser), database(database), dict(dict)
{
	//nothing to do
}

DayView::Request DayView::parseRequest()
{
	Request request;

	if (urlParser.getOptions().size() == 0) {
		request.hasOptions = false;
		return request;
	}

	request.hasOptions = true;

	string year = urlParser.get("year");
	string month = urlParser.get("month");
	string day = urlParser.get("day");
	string side = urlParser.get("side");
	string display = urlParser.get("display");
	string line = urlParser.get("line");
	vector<string> inverter = urlParser.getRange("inverter");

	request.inverters.reserve(inverter.size());

	for (vector<string>::const_iterator it = inverter.begin(); it != inverter.end(); ++it) {
		uint32_t inv;
		try {
			inv = convertTo<uint32_t>(*it, std::hex);
		} catch (PvlogException& exc) {
			PVLOG_EXCEPT("Invalid inverter: " + *it + " inverter must be and hexadecimal number");
		}

		vector<string> lines = urlParser.getRange("*it");
		vector<int> linesDec;
		linesDec.reserve(lines.size());
		for (vector<string>::const_iterator l = lines.begin(); l != lines.end(); ++l) {
			int line;
			try {
				line = convertTo<uint32_t>(*l);
			} catch (PvlogException& exc) {
				PVLOG_EXCEPT("Invalid inverter line: " + *it + " inverter line must be and decimal number");
			}
			linesDec.push_back(line);

		}

		request.inverters.push_back(std::make_pair(inv, linesDec));
	}

	request.year = convertTo<int>(year);
	request.month = convertTo<int>(month);
	request.day = convertTo<int>(day);
	request.line = convertTo<int>(line);

	if (side == "ac") {
		request.side = AC;
	} else if (side == "dc") {
		request.side = DC;
	} else {
		PVLOG_EXCEPT("Invalid side: " + side + " should be \"ac\" or \"dc\"");
	}

	if (display == "voltage") {
		request.type = Database::VOLTAGE;
	} else if (display == "power") {
		request.type = Database::POWER;
	} else if (display == "current") {
		request.type = Database::CURRENT;
	} else {
		PVLOG_EXCEPT("Invalid display: " + display + " should be \"power\", \"voltage\" or \"current\".");
	}

	return request;

}

void DayView::fillDictionary(const DayView::Data& data)
{

}

DayView::InverterData DayView::readInverterData(uint32_t id, int line, const DateTime& from,
                                                const DateTime& to, Side side, Database::Type type)
{
	Database::Inverter inverter;
	InverterData data;

	inverter = database->inverter(id);

	data.name = inverter.name;
	data.id = id;
	data.line = line;
	if (side == AC) {
		data.values = database->readAc(id, line, type, from, to);
	} else {
		data.values = database->readDc(id, line, type, from, to);
	}

	return data;
}

DayView::Data DayView::readData(const Request& request)
{
	Data data;
	data.side = request.side;

	DateTime from(request.year, request.month, request.day, 0, 0, 0);
	DateTime to(request.year, request.month, request.day, 23, 59, 59);

	for (vector< std::pair<uint32_t, std::vector<int > > >::const_iterator it = request.inverters.begin();
			it != request.inverters.end(); ++it) {
		for (vector<int>::const_iterator l = it->second.begin(); l != it->second.end(); ++l) { // iterate lines
			InverterData inverterData = readInverterData(it->first, *l, from, to, request.side, request.type);
			data.inverterData.push_back(inverterData);
		}
	}

	return data;
}

void DayView::handleRequest()
{
	Request request = parseRequest();

	//Database access
	Data data = readData(request);

	//fill template with data
	fillDictionary(data);

}
