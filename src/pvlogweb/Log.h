#ifndef LOG_H
#define LOG_H

#include <stdint.h>
#include <cstdio>
#include <sstream>
#include <cstring>

/*
 class LogBackend {
 public:
 void logDebug(const std::ostream& str) = 0;
 void logInfo(const std::ostream& str) = 0;
 void logWarning(const std::ostream& str) = 0;
 void logError(const std::ostream& str) = 0;
 };

 class LogBackendStderr : public LogBackend {
 void logDebug(const std::ostream& str)
 {
 std::cerr << str.rdbuf();
 }

 void logInfo(const std::ostream& str)
 {
 std::cerr << str.rdbuf();
 }

 void logWarning(const std::ostream& str)
 {
 std::cerr << str.rdbuf();
 }

 void logError(const std::ostream& str)
 {
 std::cerr << str.rdbuf();
 }
 };
 *//*
 class Log {
 public:
 enum Level {
 Error,
 Info,
 WARNING,
 ERROR
 };

 Log(std::ostream& backend);

 void operator()(const std::string& message,
 char const* function,
 char const* file,
 int line);

 private:
 Level level;
 std::ostream& backend;
 };


 class Log {
 private:
 LogBackend *backend;
 Log()
 {
 backend = new LogBackendStderr();
 }

 public:
 Log& logger()
 {
 static Log log();
 return log;
 }

 void setBackend(LogBackend* backend)
 {
 delete this->backend;
 backend = backend;
 }

 void logDebug(const std::ostream& steam)
 {
 backend->logDebug(stream);
 }

 void logInfo(const std::ostream& steam)
 {
 backend->logInfo(stream);
 }

 void logWarning(const std::ostream& steam)
 {
 backend->logWarning(stream);
 }

 void logError(const std::ostream& steam)
 {
 backend->logError(stream);
 }
 }
 */
/*
 static std::ostream& Debug()
 {
 return std::cerr;
 }

 static std::ostream& Warning()
 {
 return std::cerr;
 }

 static std::ostream& Info()
 {
 return std::cerr;
 }

 static std::ostream& Error()
 {
 return std::cerr;
 }

 #define LOG(Logger_, Message_)                  \
  Logger_(                                      \
    static_cast<std::ostringstream&>(           \
      std::ostringstream().flush() << Message_  \
    ).str(),                                    \
    __FILE__,                                   \
    __LINE__                                    \
  );
 */

enum Level {
	Error = 0, Info, Warning, Debug
};

static const char *levelName[] = { "ERROR", "INFO", "WARNING", "DEBUG" };

class Log {
public:
	Log()
	{
	}
	virtual ~Log()
	{
		os << std::endl;
		fprintf(stderr, "%s", os.str().c_str());
		fflush(stderr);
	}

	std::ostringstream& Get(Level level, const char* file, int line)
	{
		const char *fileName = filename(file);

		os << levelName[level] << '[' << fileName << " " << line << ']' << " ";
		messageLevel = level;
		return os;
	}
	static Level& ReportingLevel()
	{
		static Level dumpLevels;
		return dumpLevels;
	}

protected:
	std::ostringstream os;

private:
	static const char *filename(const char *file)
	{
		const char *i;
		int len = strlen(file);
		if (len <= 1) return file;

		for (i = &file[len - 1]; i != file; i--) {
			if (*i == '/' || *i == '\\') {
				return &i[1];
			}
		}
		return file;
	}

	Log(const Log&);
	Log& operator =(const Log&);
private:
	Level messageLevel;
};

/*
 std::ostringstream& Log::Get(Level level)
 {
 os << levelName[level] << ": ";
 messageLevel = level;
 return os;
 }
 Log::~Log()
 {
 os << std::endl;
 fprintf(stderr, "%s", os.str().c_str());
 fflush(stderr);
 }
 */
#define LOG(LEVEL) \
if (LEVEL > Log::ReportingLevel()) \
; \
else \
Log().Get(LEVEL, __FILE__, __LINE__)

#endif /* #ifndef LOG_H */
