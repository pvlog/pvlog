#include <cstdlib>
#include <cstdio>
#include <string>

#include <ctemplate/template.h>

#include "Log.h"
#include "DayView.h"
#include "ConfigReader.h"

#define DAY_VIEW_FILE ""

using std::string;
using ctemplate::TemplateDictionary;

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

		//read config file
		ConfigReader configReader("");
		configReader.parse();

		string databaseType = configReader.getValue("database_type");
		string databaseName = configReader.getValue("database_name");
		string hostname = configReader.getValue("hostname");
		string port = configReader.getValue("port");
		string username = configReader.getValue("username");
		string password = configReader.getValue("password");

		string templateDirectoy = configReader.getValue("template_directory");

		//initialize ctemplate
		TemplateDictionary* dict = new TemplateDictionary(templateDirectoy + "/" + DAY_VIEW_FILE);

		//some global values
		dict->SetValue("OVERVIEW", "Overview");
		dict->SetValue("DAY", "Day");
		dict->SetValue("MONTH", "Month");
		dict->SetValue("YEAR", "Year");
		dict->SetValue("EVENTS", "Events");
		dict->SetValue("SETUP", "Setup");

		Database* database;


		if (view == "day") {
			urlParser.remove("day");
			DayView(urlParser, database, dict).handleRequest();
		} else if (view == "month") {
			urlParser.remove("month");
			//MonthView(urlParser).handleRequest();
		}


	}
	catch (...) {
		LOG(Error) << "Error";
	}

}
