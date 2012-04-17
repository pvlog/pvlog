#ifndef PVLOG_EXCEPTION_H
#define PVLOG_EXCEPTION_H

#include <exception>
#include <string>
#include <sstream>

class PvlogwebException: public std::exception {
public:
	PvlogwebException(const std::string& filename, unsigned int line, const std::string& message) throw ()
	{
		std::stringstream ss;
		ss << filename << ", " << line << ": " << message;
		whatMessage = ss.str();
	}
	virtual ~PvlogwebException() throw ()
	{
	}

	virtual const char *what() const throw ()
	{
		return whatMessage.c_str();;
	}

private:
	std::string whatMessage;
};

#define PVLOGWEB_EXCEPT(MESSAGE) throw PvlogwebException(__FILE__, __LINE__, MESSAGE)
#define PVLOGWEB_NOT_NULL(POINTER) if (POINTER == NULL) { PVLOGWEB_EXCEPT("Null pointer exception!"); }

#endif // PVLOGEXCEPTION_H
