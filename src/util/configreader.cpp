#include <configreader.h>
#include <log.h>
#include <pvlogexception.h>
#include <fstream>
#include <iostream>


static void trim(std::string& source, const char * delims = "\t\n\r\0x20")
{
	size_t pos = source.find_last_not_of(delims);

	if (pos != std::string::npos) source.erase(pos + 1);
	else source.erase();

	pos = source.find_first_not_of(delims);

	if (pos != std::string::npos) source.erase(0, pos);
	else source.erase();
}

void ConfigReader::open(const std::string& filename)
{
	file.open(filename.c_str(), std::fstream::in);

	if (!file.is_open()) PVLOG_EXCEPT("Could not open file: " + filename);

}

ConfigReader::ConfigReader(const std::string& file)
{
	open(file);
}

void ConfigReader::parse()
{

	if (!file.is_open()) PVLOG_EXCEPT("Config file not open!");

	std::string line;
	int lineNum = 0;
	while (std::getline(file, line)) {
		if (line.length() == 0) continue;
		if (line[0] == '#') continue;
		if (line[0] == ';') continue;

		size_t posSharp = line.find('#');
		size_t posSemicolon = line.find(';');

		size_t comment = (posSharp > posSemicolon) ? posSemicolon : posSharp;
		if (comment != std::string::npos) line.erase(comment);

		size_t posEqual = line.find('=');

		if (posEqual == 0) PVLOG_EXCEPT("Broken config file, line: "/* + lineNum*/);

		std::string name = line.substr(0, posEqual);
		std::string value = line.substr(posEqual + 1);

		trim(name);
		trim(value);

		if (name.empty()) PVLOG_EXCEPT("Broken config file"/* + lineNum*/);

		LOG(Debug) << "Config data, key = " << name << " data = " << value;

		values.insert(stringPair(name, value));

		++lineNum;
	}
}

const std::string & ConfigReader::getValue(const std::string& name) const
{
	std::map<std::string, std::string>::const_iterator it = values.find(name);

	if (it == values.end()) PVLOG_EXCEPT(name + ": not found!");

	return it->second;
}
