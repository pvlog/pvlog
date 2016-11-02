#ifndef SRC_UTIL_TIMEUTIL_H_
#define SRC_UTIL_TIMEUTIL_H_

#include <cstdint>

#include <boost/date_time/posix_time/posix_time.hpp>
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

} //namespace util {

#endif /* SRC_UTIL_TIMEUTIL_H_ */
