#include "Log.h"

#include <cstring>
#include <cstdio>
#include <ctime>

Level Log::messageLevel = Trace;

const static char *levelName[] = { "ERROR", "INFO", "WARNING", "DEBUG", "TRACE" };

static const char *filename(const char *file) {
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


std::ostringstream& Log::get(Level level, const char* file, int line) {
	const char *fileName = filename(file);
	std::time_t curTime = std::time(nullptr);

	os << levelName[level] << '[' << ctime(&curTime) << fileName << " " << line << " " << ']' << " ";
	return os;
}

Log:: ~Log() {
	os << std::endl;
	fprintf(stderr, "%s", os.str().c_str());
	fflush(stderr);
}
