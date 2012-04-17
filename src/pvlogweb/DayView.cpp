#include "DayView.h"

#include <string>
#include <sstream>

#include "PvlogException.h"

int str2int (const std::string& str, int base = 0)
{
    char *end;
    long  l;
    errno = 0;
    l = strtol(str.c_str(), &end, base);
    if ((errno == ERANGE && l == LONG_MAX) || l > INT_MAX) {
    	PVLOG_EXCCEPT(string("Not convertible to int (overflow): ") + str);
    }
    if ((errno == ERANGE && l == LONG_MIN) || l < INT_MIN) {
    	PVLOG_EXCCEPT(string("Not convertible to int (underflow): ") + str);
    }
    if (*s == '\0' || *end != '\0') {
    	PVLOG_EXCCEPT(string("Not convertible to int : ") + str);
    }

    return l;
}


using std::string;

DayView::DayView(UrlParser& urlParser) : urlParser(urlParser)
{
	//nothing to do
}

Request DayView::parseRequest()
{
	string year = urlParser.get("year");
	string month = urlParser.get("month");
	string day = urlParser.get("day");
	string type = urlParser.get("type");
	string display = urlParser.get("display");

	Request request;

	int ret;
	ret = str2int(request.year, year.c_str());
	if (ret < 0) PVLOG_EXCEPT("Invalid year: ") << year;

	//ret = str2int(request.month, year.c_str())


	//request.year =
	return Request;

}
void DayView::handleRequest()
{


}
