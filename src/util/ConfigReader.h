#ifndef CONFIG_READER_H
#define CONFIG_READER_H

#include <string>
#include <map>
#include <fstream>

class ConfigReader {
private:
	std::string filename;
	std::ifstream file;

	std::map<std::string, std::string> values;
	typedef std::pair<std::string, std::string> stringPair;

public:
	explicit ConfigReader(const std::string & file);

	ConfigReader()
	{
	}

	void open(const std::string & filename);

	void parse();

	const std::string & getValue(const std::string & name) const;
};

#endif // #ifndef CONFIG_READER_H
