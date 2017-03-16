#ifndef DAYSUMMARYMESSAGE_H_
#define DAYSUMMARYMESSAGE_H_

#include <string>

#include <boost/signals2.hpp>

namespace odb {
	class database;
}

class DaySummaryMessage {
public:
	boost::signals2::signal<void (const std::string&)> newDaySummarySignal;

	DaySummaryMessage(odb::database* db);

	void generateDaySummaryMessage();

private:
	odb::database* db;
};

#endif /* DAYSUMMARYMESSAGE_H_ */
