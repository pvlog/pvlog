#ifndef PVLOG_EXCEPTION_H
#define PVLOG_EXCEPTION_H

#include <exception>
#include <string>
#include <sstream>

class PvlogException : public std::exception {
    public:
        PvlogException(const std::string & filename, unsigned int line, const std::string & message) throw() :
            _filename(filename),
            _line(line),
            _message(message)
            {
            }
        virtual ~PvlogException() throw() {}

        virtual const char *what() {
            std::stringstream ss;
            ss <<  _filename << ", " << _line << ": " << _message;
            return ss.str().c_str();
            return (char*)NULL;
        }

    private:
        std::string  _filename;
        unsigned int _line;
        std::string  _message;
};

#define PVLOG_EXCEPT(MESSAGE) throw PvlogException(__FILE__, __LINE__, MESSAGE)

#endif // PVLOGEXCEPTION_H
