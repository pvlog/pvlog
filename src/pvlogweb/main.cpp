#include <cstdlib>
#include <cstdio>
#include <string>

#include "Log.h"

int main(int argc, char *argv[]) 
{
	try {

	char *env = getenv("REQUEST_METHOD");
	if (env == NULL) {
		LOG(Error) << "Could not read environment variable REQUEST_METHOD";
		return EXIT_FAILURE;
	}

	std::string methode(env);
	if (method != "GET") {
		LOG(Error) << "Invalid request method: " << method;
		return EXIT_FAILURE;
	}

	env = getenv("QUERY_STRING");
	if (env == NULL) {
		LOG(Error) << "Could not read environment variable QUERY_STRING";
		return EXIT_FAILURE;
	}

	std::string queryString(env);

	UrlParser urlParser(queryString);
	std::string view = urlParser.get("view");

	switch (view) {
	case "day" :
		DayView(urlParser).handleRequest();
		break;
	case "month" :
//		MonthView(urlParser).handleRequest();
//		break;
	case "year" :
//		YearView(urlParser).handleRequest();
//		break;
	case "overview" :
//		Overview(urlParser).handleRequest();
//		break;
	default:
	}


	}
	catch (...) {
		LOG(Error) << "Error";
	}

}
