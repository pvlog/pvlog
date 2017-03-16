#include "daysummarymessage.h"

#include <sstream>

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/format.hpp>

#include <odb/database.hxx>
#include <odb/query.hxx>

#include "daydata_odb.h"
#include "models/daydata.h"
#include "models/event.h"
#include "event_odb.h"

using model::DayData;
using model::Event;

namespace bg = boost::gregorian;

DaySummaryMessage::DaySummaryMessage(odb::database* db) : db(db) {
	// nothing to do
}

void DaySummaryMessage::generateDaySummaryMessage() {
	odb::session session;
	odb::transaction t(db->begin());

	using DayDataResult = odb::result<DayData>;
	using DayDataQuery  = odb::query<DayData>;

	DayDataResult dayDataRes(db->query<DayData>(DayDataQuery::date == bg::day_clock::local_day()));

	std::ostringstream message;
	message << "Todays production:\n\n";
	message << boost::format("|%-30s|%-20s\n") % "Inverter Name" % "Energy[kWh]";
	message << "|------------------------------|--------------------|\n";

	boost::format dayDataFmter("|%-30s|%-20.2f|\n");
	for (const DayData& d : dayDataRes) {
		message << dayDataFmter % d.inverter->name % (static_cast<float>(d.dayYield) / 1000) ;

	}
	message << "\n\n";


	message << "Events:\n\n";

	using EventResult = odb::result<Event>;
	using EventQuery  = odb::query<Event>;

	EventResult eventRes(db->query<Event>("ORDER BY" + EventQuery::inverter + "," + EventQuery::time  + "DESC"));
	if (eventRes.empty()) {
		message << "No events present\n";
	} else {
		message << boost::format("|%-30s|%-15s|%-30s\n") % "Inverter Name" % "Event number" % "Event Message";
		message << "|------------------------------|---------------|------------------------------|\n";
		boost::format eventFmter("|%-30s|%-15d|%-30s|\n");
		for (const Event& e : eventRes) {
			message << eventFmter % e.inverter->name % e.number % e.message;
		}
	}
	message << "\n\n";

	t.commit();

	newDaySummarySignal(message.str());
}

