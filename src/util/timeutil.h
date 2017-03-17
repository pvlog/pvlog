#ifndef SRC_UTIL_TIMEUTIL_H_
#define SRC_UTIL_TIMEUTIL_H_

#include <cstdint>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/version.hpp>

#if BOOST_VERSION < 105800
namespace boost {

namespace posix_time {

inline std::time_t to_time_t(ptime pt) {
	time_duration dur = pt - ptime(gregorian::date(1970, 1, 1));
	return std::time_t(dur.total_seconds());
}

} //namespace posix_time {

} //namespace boost {
#endif


namespace util {

/**
 * Round time up to next multiple of duration
 */
inline boost::posix_time::ptime roundUp(const boost::posix_time::ptime& time,
                                        const boost::posix_time::time_duration& duration) {
	namespace pt = boost::posix_time;

	int64_t d = duration.total_seconds();
	return pt::from_time_t((pt::to_time_t(time) + d) / d * d);
}


inline boost::posix_time::ptime roundDown(const boost::posix_time::ptime& time,
                                          const boost::posix_time::time_duration& duration) {
	namespace pt = boost::posix_time;

	int64_t d = duration.total_seconds();
	return pt::from_time_t(pt::to_time_t(time) / d * d);
}

inline boost::posix_time::time_duration abs(boost::posix_time::time_duration duration) {
	if (duration.is_negative()) {
		duration *= -1;
	}
	return duration;
}

inline boost::posix_time::ptime local_to_utc(const boost::posix_time::ptime& time) {
	boost::gregorian::date date              = time.date();
	boost::posix_time::time_duration dayTime = time.time_of_day();
	std::tm timeInfo;

	timeInfo.tm_year = date.year() - 1900;
	timeInfo.tm_mon  = date.month() - 1;
	timeInfo.tm_mday = date.day();
	timeInfo.tm_hour = dayTime.hours();
	timeInfo.tm_min  = dayTime.minutes();
	timeInfo.tm_sec  = dayTime.seconds();

	std::time_t utcTime = std::mktime(&timeInfo);

	return boost::posix_time::from_time_t(utcTime);
}

} //namespace util {

#endif /* SRC_UTIL_TIMEUTIL_H_ */
