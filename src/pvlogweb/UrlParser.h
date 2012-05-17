#include <string>
#include <map>
#include <vector>

class UrlParser {
private:
	typedef std::multimap<std::string, std::string> Values;
	Values values;
	std::string url;
	char seperator;
	bool parsed;

	std::string unascape(const std::string& str);
	void add(const std::string& str);

public:
	UrlParser(const std::string& url, char seperator = '=') :
		url(url),
		seperator(seperator),
		parsed(false)
	{
		//nothing to do
	}

	~UrlParser()
	{
		//nothing to do
	}

	void parse();

	std::string get(const std::string& key);

	void remove(const std::string& key);

	std::vector<std::string> getRange(const std::string& key);

	std::vector<std::string> getOptions();
};
