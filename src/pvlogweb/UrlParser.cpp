#include "UrlParser.h"
#include "PvlogException.h"
#include "Utility.h"

using std::string;
using std::vector;
using std::set;

static char toNum(char ascii)
{
	if (ascii >= 0x30 && ascii <= 0x39) {
		return ascii - 0x30;
	} else if (ascii >= 0x41 && ascii <= 0x46) {
		return ascii - 0x41;
	} else {
		PVLOG_EXCEPT(string("Invalid url: "));
	}
}

std::string UrlParser::unascape(const std::string& str)
{
	string unascaped;
	bool escape = false;

	for (string::const_iterator it = str.begin(); it != str.end(); ++it) {
		if (escape) {
			char first = toNum(*it);
			++it;
			if (it == str.end()) PVLOG_EXCEPT(string("Invalid url: ") + url);
			char second = toNum(*it);
			unascaped.push_back(first << 8 | second);
			escape = false;
		} else if (*it == '%') {
			escape = true;
		} else {
			unascaped.push_back(*it);
		}
	}
	//check if last character is "%"
	if (escape) PVLOG_EXCEPT(string("Invalid url: ") + url);

	return unascaped;
}

void UrlParser::add(const string& str)
{
	string key;
	string value;
	bool valueExists;

	for (string::const_iterator it = str.begin(); it != str.end(); ++it) {
		if (*it == '=') {
			key.assign(str.begin(), it);
			value.assign(it, str.end());
			valueExists = true;
		}
	}

	if (valueExists) {
		key = unascape(key);
		value = unascape(value);
	} else {
		key = unascape(str);
		value = "";
	}

	values.insert(std::make_pair(key, value));
}

void UrlParser::parse()
{
	std::string str;
	for (string::const_iterator it = url.begin(); it != url.end(); ++it) {
		if (*it == seperator) {
			add(str);
		} else {
			str.push_back(*it);
		}
	}
	parsed = true;
}

void UrlParser::remove(const string& key)
{
	values.erase(key);
}

string UrlParser::get(const string& key)
{
	if (!parsed) parse();

	Values::const_iterator it = values.find(key);
	if (it == values.end()) {
		PVLOG_EXCEPT("Key does not exist!");
	}

	return it->first;
}

vector<string> UrlParser::getRange(const string& key)
{
	if (!parsed) parse();

	std::pair<Values::const_iterator, Values::const_iterator> equalRange = values.equal_range(key);
	return vector<string>(util::const_key_iterator<Values>(equalRange.first),
			util::const_key_iterator<Values>(equalRange.second));

}

set<string> UrlParser::getOptions()
{
	if (!parsed) parse();

	return set<string>(util::const_key_iterator<Values>(values.begin()),
			util::const_key_iterator<Values>(values.end()));
}



