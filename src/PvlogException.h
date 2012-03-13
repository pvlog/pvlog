#ifndef PVLOG_EXCEPTION_H
#define PVLOG_EXCEPTION_H

#include <exception>
#include <string>
#include <sstream>

class PvlogException : public std::exception {
    public:
        PvlogException(const std::string& filename, unsigned int line,
					const std::string& message) throw()
            {
				std::stringstream ss;
            	ss <<  filename << ", " << line << ": " << message;
            	whatMessage = ss.str();
			}
        virtual ~PvlogException() throw() {}

        virtual const char *what() const throw() {
			return whatMessage.c_str();;
        }

    private:
    	std::string whatMessage;
};

#define PVLOG_EXCEPT(MESSAGE) throw PvlogException(__FILE__, __LINE__, MESSAGE)
#define PVLOG_NOT_NULL(POINTER) if (POINTER == NULL) { PVLOG_EXCEPT("Nullpointer exception!"); }

#endif // PVLOGEXCEPTION_H
