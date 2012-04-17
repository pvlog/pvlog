#include <cstdlib>
#include <cstdio>
#include <string>

#include "Log.h"
#include "DayView.h"

int main(int argc, char *argv[]) {
	try {

		char *env = getenv("REQUEST_METHOD");
		if (env == NULL) {
			return EXIT_FAILURE;
		}

		std::string method(env);
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

		if (view == "day") {
			DayView(urlParser).handleRequest();
		} else if (view == "month") {
			//MonthView(urlParser).handleRequest();
		}


	}
	catch (...) {
		LOG(Error) << "Error";
	}

}
