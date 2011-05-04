#ifndef PVLOG_EXCEPTION_H
#define PVLOG_EXCEPTION_H

#include <exception>

class PvlogException : public std::exception {
    public:
        PvlogException(const std::string &filename, unsigned int line, const std::string &message) :
            m_filename(filename),
            m_line(line),
            m_message(message)
            {
            }

        virtual const char *what() {
            return m_filename + ", " + m_line + ": " + m_message;
        }

    private:
        std::string  m_filename;
        unsigned int m_line;
        std::string  m_message;
};

#define PVLOG_EXCEPT(MESSAGE) throw PvlogException(__FILE__, __LINE__, MESSAGE)

#endif // PVLOGEXCEPTION_H
