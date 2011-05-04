#include "ConfigReader.h"

#include <fstream>

#include "PvlogException.h"

static void trim(std::string & source, const char * delims = "\t\n\r")
{
    size_t pos = source.find_last_not_of(delims);

    if (pos != std::string::npos) source.erase(++pos);

    pos = source.find_first_not_of(delims);

    if (pos != std::string::npos) source.erase(0, pos);
    else source.erase();
}

ConfigReader::ConfigReader(const std::string & file) : filename(file)
{
}

void ConfigReader::open(const std::string & file)
{
    filename = file;
}

void ConfigReader::parse()
{
    if (filename.empty())
        PVLOG_EXCEPT("No filename given!");

    std::ifstream file(filename.c_str());

    if (!file.is_open())
        PVLOG_EXCEPT("Could not open config file: " + filename);

    std::string line;
    int lineNum = 0;
    while (std::getline(file, line)) {
        if (line.length() == 0) continue;
        if (line[0] == '#') continue;
        if (line[0] == ';') continue;

        size_t posSharp     = line.find('#');
        size_t posSemicolon = line.find(';');

        size_t comment = (posSharp > posSemicolon) ? posSemicolon : posSharp;
        if (comment != std::string::npos)
            line.erase(comment);

        size_t posEqual = line.find('=');

        if (posEqual == 0) PVLOG_EXCEPT("Broken config file, line: "/* + lineNum*/);

        std::string name = line.substr(0, posEqual);
        std::string value = line.substr(posEqual + 1);

        trim(name);
        trim(value);

        if (name.empty()) PVLOG_EXCEPT("Broken config file"/* + lineNum*/);

        values.insert(stringPair(name, value));;

        ++lineNum;
    }
}

const std::string & ConfigReader::getValue(const std::string & name) const
{
    return std::string();
}
