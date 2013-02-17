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
using std::ostringstream;
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

		vector<string> lines = urlParser.getRange(*it);
		vector<int> linesDec;
		linesDec.reserve(lines.size());
		for (vector<string>::const_iterator l = lines.begin(); l != lines.end(); ++l) {
			int line;
			try {
				line = convertTo<uint32_t>(*l);
			} catch (PvlogException& exc) {
				PVLOG_EXCEPT("Invalid inverter line: " + *it + " inverter line must be an decimal number");
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

void DayView::fillInverterDictionary(const vector< std::pair<string, int> >& inverters)
{
	for (vector< std::pair<string, int> >::const_iterator it = inverters.begin(); it != inverters.end(); ++it) {
		TemplateDictionary* invDic = dict->AddSectionDictionary("INVERTERS");
		invDic->SetValue("INVERTER", it->first);
		for (int i = 0; i < it->second; ++i) {
			TemplateDictionary* invLine = invDic->AddSectionDictionary("LINES");
			invLine->SetIntValue("LINE", i);
		}
	}
}

void DayView::fillChartDictionary(const vector<InverterData>& inverterData)
{
	for (vector<InverterData>::const_iterator it = inverterData.begin();
			it != inverterData.end(); ++it) {
		std::ostringstream ss;
		ss << it->name << " (" << it->line << ")";
		string label = ss.str();

		TemplateDictionary* chartData = dict->AddSectionDictionary("CHART_DATA_SERIES");
		chartData->SetValue("LABEL", label);
		for (vector< std::pair<uint32_t,uint32_t> >::const_iterator valueIt;
				valueIt != it->values.end(); ++valueIt) {
			TemplateDictionary* chartValues = chartData->AddSectionDictionary("CHART_DATA_VALUES");
			chartValues->SetIntValue("CHART_DATA_X", valueIt->first);
			chartValues->SetIntValue("CHART_DATA_VALUE_Y", valueIt->second);
		}
	}
}

void DayView::fillDictionary(const DayView::Data& data)
{
	fillInverterDictionary(data.inverters);
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

	//read data for chart
	for (vector< std::pair<uint32_t, std::vector<int > > >::const_iterator it = request.inverters.begin();
			it != request.inverters.end(); ++it) {
		for (vector<int>::const_iterator l = it->second.begin(); l != it->second.end(); ++l) { // iterate lines
			InverterData inverterData = readInverterData(it->first, *l, from, to, request.side, request.type);
			data.inverterData.push_back(inverterData);
		}
	}

	//read data of all inverters
	vector<Database::Inverter> inverters = database->inverters();
	data.inverters.reserve(inverters.size());
	for (vector<Database::Inverter>::const_iterator it = inverters.begin(); it != inverters.end(); ++it) {
		string name;
		if (it->name.empty()) {
			ostringstream ss;
			ss << it->id;
			name = ss.str();
		} else {
			name = it->name;
		}

		int lines;
		if (request.side == AC) {
			lines = it->phaseCount;
		} else {
			lines = it->trackerCount;
		}

		data.inverters.push_back(std::make_pair(name, lines));
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
