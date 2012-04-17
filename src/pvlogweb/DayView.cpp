#include "DayView.h"

#include <string>
#include <sstream>
#include <climits>
#include <vector>

using std::string;
using std::vector;

#include "PvlogException.h"

bool str2int(int32_t& out, const std::string& str, int base = 10)
{
    char *end;
    long  l;
    errno = 0;
    l = strtol(str.c_str(), &end, base);
    if ((errno == ERANGE && l == LONG_MAX) || l > INT_MAX) {
    	return false;
    }
    if ((errno == ERANGE && l == LONG_MIN) || l < INT_MIN) {
    	return false;
    }
    if (str.empty() || *end != '\0') {
    	return false;
    }

    out = static_cast<int32_t>(l);
    return true;
}


using std::string;

DayView::DayView(UrlParser& urlParser) : urlParser(urlParser)
{
	//nothing to do
}

DayView::Request DayView::parseRequest()
{
	string year = urlParser.get("year");
	string month = urlParser.get("month");
	string day = urlParser.get("day");
	string side = urlParser.get("side");
	string display = urlParser.get("display");
	vector<string> inverter = urlParser.getRange("inverter");

	Request request;

	for (vector<string>::const_iterator it = inverter.begin(); it != inverter.end(); ++it) {
		int32_t inv;
		if (str2int(inv, *it, 16) == false) PVLOG_EXCEPT(string("Invalid inverter: ") + *it +
				" inverter must be and hexadecimal number");
		request.inverters.push_back(inv);
	}

	if (str2int(request.year, year) == false) PVLOG_EXCEPT(string("Invalid year: ") + year);
	if (str2int(request.month, month) == false) PVLOG_EXCEPT(string("Invalid month: ") + month);
	if (str2int(request.day, day) == false) PVLOG_EXCEPT(string("Invalid day: ") + day);

	if (side == "ac") {
		request.side = AC;
	} else if (side == "dc") {
		request.side = DC;
	} else {
		PVLOG_EXCEPT(string("Invalid side: ") + side + " should be \"ac\" or \"dc\"");
	}

	if (display == "voltage") {
		request.type = Database::VOLTAGE;
	} else if (display == "power") {
		request.type = Database::POWER;
	} else if (display == "current") {
		request.type = Database::CURRENT;
	} else {
		PVLOG_EXCEPT(std::string("Invalid display: ") + display +
				" should be \"power\", \"voltage\" or \"current\".");
	}


	return request;

}
void DayView::handleRequest()
{

}
