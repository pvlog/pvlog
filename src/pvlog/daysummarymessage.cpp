/*
 * This file is part of Pvlog.
 *
 * Copyright (C) 2017 pvlogdev@gmail.com
 *
 * Pvlog is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Pvlog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Pvlog.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "daysummarymessage.h"

#include <sstream>

#include <boost/date_time/c_local_time_adjustor.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/format.hpp>
#include <odb/database.hxx>
#include <odb/query.hxx>

#include "timeutil.h"

#include "models/daydata_odb.h"
#include "models/daydata.h"
#include "models/event.h"
#include "models/event_odb.h"

using model::DayData;
using model::Event;

namespace bg = boost::gregorian;
namespace pt = boost::posix_time;
namespace dt = boost::date_time;

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
	message << boost::format("|%-30s|%-20s|\n") % "Inverter Name" % "Energy[kWh]";
	message << "|------------------------------|--------------------|\n";

	boost::format dayDataFmter("|%-30s|%-20.2f|\n");
	for (const DayData& d : dayDataRes) {
		message << dayDataFmter % d.inverter->name % (static_cast<float>(d.dayYield) / 1000) ;

	}
	message << "\n\n";


	message << "Events:\n\n";

	using EventResult = odb::result<Event>;
	using EventQuery  = odb::query<Event>;

	bg::date curDate = bg::day_clock::local_day();

	pt::ptime begin = util::local_to_utc(pt::ptime(curDate));
	pt::ptime end   = begin + pt::hours(24);

	EventQuery filterData(EventQuery::time >= EventQuery::_ref(begin) && EventQuery::time < EventQuery::_ref(end));
	EventQuery sortResult("ORDER BY" + EventQuery::inverter + "," + EventQuery::time  + "DESC");

	EventResult eventRes(db->query<Event>(filterData + sortResult));
	if (eventRes.empty()) {
		message << "No events present\n";
	} else {
		message << boost::format("|%-30s|%-25s|%-15s|%-30s|\n") % "Inverter Name" % "Time" % "Event number" % "Event Message";
		message << "|------------------------------|-------------------------|---------------|------------------------------|\n";
		boost::format eventFmter("|%-30s|%-25s|%-15d|%-30s|\n");
		for (const Event& e : eventRes) {
			std::string timeString = pt::to_simple_string(dt::c_local_adjustor<pt::ptime>::utc_to_local(e.time));
			message << eventFmter % e.inverter->name % timeString % e.number % e.message;
		}
	}
	message << "\n\n";

	t.commit();

	newDaySummarySignal(message.str());
}

