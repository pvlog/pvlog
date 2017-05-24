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

#include "daydata.h"

namespace model {

Json::Value toJson(const DayStats& dayStats) {
	Json::Value json;

	json["month"] = dayStats.month;
	json["day"]   = dayStats.day;
	json["min"]   = static_cast<Json::Int64>(dayStats.min);
	json["avg"]   = static_cast<Json::Int64>(dayStats.avg);
	json["max"]   = static_cast<Json::Int64>(dayStats.max);
	json["count"] = dayStats.count;

	return json;
}

Json::Value toJson(const MonthStats& monthStats) {
	Json::Value json;

	json["month"] = monthStats.month;
	json["min"]   = static_cast<Json::Int64>(monthStats.min);
	json["avg"]   = static_cast<Json::Int64>(monthStats.avg);
	json["max"]   = static_cast<Json::Int64>(monthStats.max);
	json["count"] = monthStats.count;

	return json;
}

Json::Value toJson(const DayData& dayData) {
	Json::Value json;

	json["inverter"] =  static_cast<Json::Int64>(dayData.inverter->id);
	json["date"]     = boost::gregorian::to_iso_string(dayData.date);
	json["yield"]    = static_cast<Json::Int64>(dayData.dayYield);

	return json;
}

} //namespace model {
