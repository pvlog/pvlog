#ifndef SRC_UTIL_TIMEUTIL_H_
#define SRC_UTIL_TIMEUTIL_H_

#include <cstdint>

#include <boost/date_time/posix_time/posix_time.hpp>

namespace util {

/**
 * Round time up to next multiple of duration
 */
boost::posix_time::ptime roundUp(const boost::posix_time::ptime& time,
                                 const boost::posix_time::time_duration& duration) {
	int64_t d = duration.total_seconds();
	return boost::posix_time::from_time_t((to_time_t(time) + d) / d * d);
}


boost::posix_time::ptime roundDown(const boost::posix_time::ptime& time,
                                 const boost::posix_time::time_duration& duration) {
	int64_t d = duration.total_seconds();
	return boost::posix_time::from_time_t(to_time_t(time) / d * d);
}


} //namespace util {

#endif /* SRC_UTIL_TIMEUTIL_H_ */
