#ifndef CONFIG_READER_H
#define CONFIG_READER_H

#include <string>
#include <map>
#include <fstream>

#include <Utility.h>

class ConfigReader {
	DISABLE_COPY(ConfigReader)
private:
	std::fstream file;

	std::map<std::string, std::string> values;
	typedef std::pair<std::string, std::string> stringPair;

protected:
	void open(const std::string & filename);

public:
	explicit ConfigReader(const std::string & file);

	void parse();

	const std::string& getValue(const std::string & name) const;
};

#endif // #ifndef CONFIG_READER_H
