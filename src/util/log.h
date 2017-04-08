#ifndef LOG_H
#define LOG_H

//#include <utility.h>
//#include <sstream>

#include <boost/log/trivial.hpp>
#include <boost/log/attributes/mutable_constant.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

#define Error boost::log::trivial::error
#define Info boost::log::trivial::info
#define Warning boost::log::trivial::warning
#define Debug boost::log::trivial::debug
#define Trace boost::log::trivial::trace

#ifndef PVLOG_LOG_MODULE
#	define PVLOG_LOG_MODULE "global"
#endif

#define LOG(sev) \
		BOOST_LOG_STREAM_WITH_PARAMS( \
				(boost::log::trivial::logger::get()), \
				(logging::setGetAttrib("File", logging::pathToFilename(__FILE__))) \
				(logging::setGetAttrib("Line", __LINE__)) \
				(logging::setGetAttrib("Module", PVLOG_LOG_MODULE)) \
				(::boost::log::keywords::severity = (sev)) \
		)

namespace logging {
template<typename ValueType>
ValueType setGetAttrib(const char* name, ValueType value) {
	auto attr = boost::log::attribute_cast < boost::log::attributes::mutable_constant<ValueType>>(
			boost::log::core::get()->get_thread_attributes()[name]
	);
	attr.set(value);
	return attr.get();
}

// Convert file path to only filename
inline std::string pathToFilename(std::string path) {
	return path.substr(path.find_last_of("/\\") + 1);
}
}

#endif /* #ifndef LOG_H */
