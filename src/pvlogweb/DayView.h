#ifndef DAYVIEW_H
#define DAYVIEW_H

#include <map>

#include "UrlParser.h"
#include "Database.h"

class DayView {
private:
	enum Side {
		AC,
		DC
	};

	struct Request {
		int year;
		int month;
		int day;
		Side side;
		Database::Type type;
		std::vector<uint32_t> inverters;
	};

	UrlParser urlParser;

	Request parseRequest();

public:
	DayView(UrlParser& urlParser);

	void handleRequest();
};

#endif // #ifndef DAYVIEW_H
