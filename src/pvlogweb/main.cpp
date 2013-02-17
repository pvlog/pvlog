#include <cstdlib>
#include <cstdio>
#include <string>
#include <fstream>

#include <ctemplate/template.h>
#include <ctemplate/template_cache.h>

#include "Log.h"
#include "DayView.h"
#include "ConfigReader.h"
#include "SqliteDatabase.h"
#include "Util.h"

#define DAY_VIEW_TPL "dayview.tpl"
#define LAYOUT_TPL "layout.tpl"

using std::string;
using ctemplate::TemplateDictionary;
using ctemplate::TemplateCache;

int main(int argc, char *argv[]) {
	try {
		string requestMethod = Util::readEnvVar("REQUEST_METHOD");
		if (requestMethod != "GET") {
			LOG(Error) << "Invalid request method: " << requestMethod;
			return EXIT_FAILURE;
		}

		std::string queryString = Util::readEnvVar("QUERY_STRING");
		std::string homeDir =  Util::readEnvVar("HOME");

		//parse config file
		std::string configFilePath = homeDir + "/" + ".pvlog/pvlog.conf";
		std::ifstream configFile(configFilePath);
		if (!configFile.is_open()) {
			configFilePath = "/etc/pvlog/pvlog.conf";
			configFile.open(configFilePath.c_str());
		}

		if (!configFile.is_open()) {
			LOG(Error) << "Could not open pvlog.conf configuration file ";
			return EXIT_FAILURE;
		}
		configFile.close();

		LOG(Debug) << "Opening database.";
		Database* database = Util::openDatabase(configFilePath);


		//setup ctemplate
		std::string templateDir = database->readConfig("template_dir");
		TemplateCache* templateCache = ctemplate::mutable_default_template_cache();
		templateCache->SetTemplateRootDirectory(templateDir);


		//create fill layout template
		TemplateDictionary* dict = new TemplateDictionary(LAYOUT_TPL);
		dict->SetValue("OVERVIEW", "Overview");
		dict->SetValue("DAY", "Day");
		dict->SetValue("MONTH", "Month");
		dict->SetValue("YEAR", "Year");
		dict->SetValue("EVENTS", "Events");
		dict->SetValue("SETUP", "Setup");


		//create different pages
		TemplateDictionary* content = dict->AddIncludeDictionary("CONTENT");

		UrlParser urlParser(queryString);
		string view = urlParser.get("view");

		LOG(Debug) << "view=" << view;

		if (view == "day") {
			urlParser.remove("day");
			content->SetFilename(DAY_VIEW_TPL);
			DayView(urlParser, database, content).handleRequest();
		} else if (view == "month") {
			urlParser.remove("month");
			//MonthView(urlParser).handleRequest();
		}

	}
	catch (const std::exception& except) {
		LOG(Error) << except.what();
		//error HTTP
	}

}
