#ifndef UTILITY_H
#define UTILITY_H

#include <sstream>
#include <typeinfo>
#include <iomanip>

#include <boost/optional.hpp>
#include <jsoncpp/json/value.h>

#include <pvlogexception.h>

namespace util {

template<typename map_type>
class const_key_iterator: public map_type::const_iterator {
public:
	typedef typename map_type::const_iterator map_iterator;
	typedef typename map_iterator::value_type::first_type key_type;

	const_key_iterator(const map_iterator& other) :
		map_type::const_iterator(other)
	{
		//nothing to do
	}

	const key_type& operator *() const
	{
		return map_iterator::operator*().first;
	}
};

template<class map_type>
class key_iterator: public map_type::iterator {
public:
	typedef typename map_type::iterator map_iterator;
	typedef typename map_iterator::value_type::first_type key_type;

	key_iterator(const map_iterator& other) :
		map_type::iterator(other)
	{
	}
	;

	key_type& operator *()
	{
		return map_type::iterator::operator*().first;
	}
};
/*
enum {
	STR2INT_SUCCESS = 0,
	STR2INT_DATA_LEFT = 1, //Not necessarily and error
	STR2INT_OVERFLOW = - 1,
	STR2INT_UNDERFLOW = -2,
	STR2INT_NONUMBER = -3,
	STR2INT_OTHERERROR = -4;
};
*/

template<typename T>
static inline T convertTo(const std::string& str,
                          std::ios_base& (*base)(std::ios_base &) = std::dec,
                          bool failIfLeftoverChars = true)
{
    std::istringstream i(str);
    T ret;
    char c;
    if (!(i >> ret) || (failIfLeftoverChars && i.get(c))) {
        PVLOG_EXCEPT("convert " + str + " to " + std::string(typeid(T).name()) + " failed.");
    }

    return ret;
}

template<typename T>
inline Json::Value toJson(const boost::optional<T>& opt) {
	Json::Value value;

	if (opt) {
		value = opt.get();
	} else {
		value = nullptr;
	}

	return value;
}

template<typename T>
std::string to_string(T t, int width) {
	std::stringstream ss;
	ss << std::setw(width) << std::setfill('0') << t;
	return ss.str();
}


#define DISABLE_COPY(CLASS) \
    CLASS(const CLASS&) = delete; \
    CLASS& operator=(const CLASS&) = delete;
} //namespace util

#endif // #ifndef UTILITY_H
