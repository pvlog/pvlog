#ifndef UTILITY_H
#define UTILITY_H

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


#define DISABLE_COPY(CLASS) \
	private : \
    	CLASS(const CLASS&); \
    	CLASS& operator=(const CLASS&);
} //namespace util

#endif // #ifndef UTILITY_H
