#ifndef LOG_H
#define LOG_H

#include <stdint.h>
#include <cstdio>
#include <sstream>
#include <cstring>

enum Level {
	Error = 0, Info, Warning, Debug
};

static const char *levelName[] = { "ERROR", "INFO", "WARNING", "DEBUG" };

class Log {
public:
	Log()
	{
		//nothing to do
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

#define LOG(LEVEL) \
if (LEVEL > Log::ReportingLevel()) \
; \
else \
Log().Get(LEVEL, __FILE__, __LINE__)

#endif /* #ifndef LOG_H */
