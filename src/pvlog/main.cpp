#include <cstdlib>
#include <iostream>
#include <memory>

#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>

#include <odb/database.hxx>
#include <odb/sqlite/database.hxx>
#include <odb/schema-catalog.hxx>
#include <jsonrpccpp/server/connectors/httpserver.h>

#include "pvlogconfig.h"
#include "configreader.h"
#include "datalogger.h"
#include "jsonrpcadminserver.h"
#include "jsonrpcserver.h"
#include "log.h"
#include "emailnotification.h"
#include "daysummarymessage.h"

#include "models/config.h"
#include "models/config_odb.h"

using model::Config;

namespace btlog = boost::log;
namespace btattrs = boost::log::attributes;
namespace btexpr = boost::log::expressions;
namespace btkeywords = boost::log::keywords;
namespace bttrivial = boost::log::trivial;

namespace po = boost::program_options;


static std::unique_ptr<odb::core::database> openDatabase(const std::string& configFile)
{
	std::string filename;

	if (configFile.empty()) {
		char *home;
		home = getenv("HOME");
		if (home == NULL) {
			PVLOG_EXCEPT("Could not get environment variable \"HOME\"!");
		} else {
			filename = std::string(home) + "/.pvlog/pvlog.conf";
		}
	} else {
		filename = configFile;
	}

	LOG(Info) << "Reading database configuration file.";
	ConfigReader configReader(filename);
	configReader.parse();

	std::string databaseType = configReader.getValue("database_type");
	std::string databaseName = configReader.getValue("database_name");
	std::string hostname = configReader.getValue("hostname");
	std::string port = configReader.getValue("port");
	std::string username = configReader.getValue("username");
	std::string password = configReader.getValue("password");
	LOG(Info) << "Successfully parsed database configuration file.";

	return std::unique_ptr<odb::database>(new odb::sqlite::database(databaseName,
			SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, true));
}

static void createDefaultConfig(odb::database* db) {
	Config timeout("timeout", "300");
	Config longitude("longitude", "-10.970000");
	Config latitude("latitude", "49.710000");

	db->persist(timeout);
	db->persist(longitude);
	db->persist(latitude);
}

static void initLogging(const std::string& file,  bttrivial::severity_level severity) {
	btlog::core::get()->add_global_attribute("Module",
			btattrs::mutable_constant<const char *>("global"));
	btlog::core::get()->add_global_attribute("File",
			btattrs::mutable_constant<std::string> (""));
	btlog::core::get()->add_global_attribute("Line",
			btattrs::mutable_constant<int>(0));

	btlog::add_common_attributes();

	boost::log::core::get()->set_filter(
			boost::log::trivial::severity >= severity
	);

	auto fmtTimeStamp = btexpr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S.%f");
	auto fmtSeverity = btexpr::attr<bttrivial::severity_level>("Severity");
	auto fmtFile = btexpr::attr<std::string>("File");
	auto fmtLine = btexpr::attr<int>("Line");

	btlog::formatter logFmt = btexpr::format("%1%[%2% %3%:%4%] %5%")
			% fmtSeverity % fmtTimeStamp % fmtFile % fmtLine % btexpr::message;

	if (!file.empty()) {
		auto fsSink = btlog::add_file_log(btkeywords::file_name = file);
		fsSink->set_formatter(logFmt);
		fsSink->locked_backend()->auto_flush(true);
	} else {
		auto cSink = btlog::add_console_log();
		cSink->set_formatter(logFmt);
		cSink->locked_backend()->auto_flush(true);
	}
}

void pvlibLogFunc(const char* module, const char *file, int line, pvlib_log_level logLevel, const char* message) {
	bttrivial::severity_level sev;

	switch (logLevel) {
	case PVLIB_LOG_ERROR:
		sev = bttrivial::error;
		break;
	case PVLIB_LOG_WARNING:
		sev = bttrivial::warning;
		break;
	case PVLIB_LOG_INFO:
		sev = bttrivial::info;
		break;
	case PVLIB_LOG_DEBUG:
		sev = bttrivial::debug;
		break;
	case PVLIB_LOG_TRACE:
		sev = bttrivial::trace;
		break;
	default:
		sev = bttrivial::warning;
		assert("Invalid severity level!");
	}

	BOOST_LOG_STREAM_WITH_PARAMS(
			(boost::log::trivial::logger::get()),
			(logging::setGetAttrib("Module", module))
			(logging::setGetAttrib("File", std::string(file)))
			(logging::setGetAttrib("Line", line))
			(::boost::log::keywords::severity = (sev))
	);
}


int main(int argc, char **argv) {
	std::string logLevel;
	std::string logPath;
	std::string configPath;
	po::options_description desc("Allowed options");
	desc.add_options()
			("help,h", "produce help message")
			("loglevel,l",po::value<std::string>(&logLevel)->default_value("warning"), "log level can be error, warning, info, debug, trace")
			("logmodules,m",po::value<std::vector<std::string>>(), "modules logging is enabled default all modules")
			("logpath,p", po::value<std::string>(), "log file location")
			("configpath,c", po::value<std::string>(&configPath)->default_value(CONFIG_FILE), "config file location");

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if (vm.count("help")) {
		std::cout << desc << std::endl;
		return EXIT_SUCCESS;
	}

	bttrivial::severity_level logSeverity = bttrivial::warning;
	pvlib_log_level pvlibLogSeverity = PVLIB_LOG_WARNING;
	if (logLevel == "error") {
		logSeverity = bttrivial::error;
		pvlibLogSeverity = PVLIB_LOG_ERROR;
	} else if (logLevel == "warning") {
		logSeverity = bttrivial::warning;
		pvlibLogSeverity = PVLIB_LOG_WARNING;
	} else if (logLevel == "info") {
		logSeverity = bttrivial::info;
		pvlibLogSeverity = PVLIB_LOG_INFO;
	} else if (logLevel == "debug") {
		logSeverity = bttrivial::debug;
		pvlibLogSeverity = PVLIB_LOG_DEBUG;
	} else if (logLevel == "trace") {
		logSeverity = bttrivial::trace;
		pvlibLogSeverity = PVLIB_LOG_TRACE;
	} else {
		std::cerr << "Invalid loglevel" << std::endl;
		std::cerr << desc << std::endl;
		return EXIT_FAILURE;
	}

	initLogging(logPath, logSeverity);

	pvlib_init(pvlibLogFunc, nullptr, pvlibLogSeverity);

	LOG(Info) << "Opening database.";
	std::unique_ptr<odb::database> db = openDatabase(configPath);
	LOG(Info) << "Successfully opened database.";

	//check database schema if doesn't exists
	odb::transaction t (db->begin ());
	if (db->schema_version() == 0) {
		LOG(Info) << "Schema does not exist. Creating it!";
		odb::schema_catalog::create_schema(*db.get(), "", false);
		createDefaultConfig(db.get());
	}
	t.commit ();

	Datalogger datalogger(db.get());
	DaySummaryMessage daySummaryMessage(db.get());
	EmailNotification emailNotification(db.get());

	datalogger.dayEndSig.connect(std::bind(&DaySummaryMessage::generateDaySummaryMessage, &daySummaryMessage));
	daySummaryMessage.newDaySummarySignal.connect(std::bind(&EmailNotification::sendMessage,
			&emailNotification, std::placeholders::_1));


	//start json server
	jsonrpc::HttpServer httpserver(8383);
	JsonRpcServer server(httpserver, &datalogger, db.get());
	server.StartListening();

	jsonrpc::HttpServer adminHttpserver(8384);
	JsonRpcAdminServer adminServer(adminHttpserver, &datalogger, db.get());
	adminServer.StartListening();


	//start datalogger work
	datalogger.work();
}
