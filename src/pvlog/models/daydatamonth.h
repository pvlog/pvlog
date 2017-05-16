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

#ifndef SRC_PVLOG_MODELS_DAYDATAMONTH_H_
#define SRC_PVLOG_MODELS_DAYDATAMONTH_H_

#include "daydata.h"
namespace model {

#pragma db view object(DayData)//
//	query((?) + "GROUP BY year, month")
struct DayDataMonth {
	//#pragma db column("sum(" + DayData::dayYield + ")")
	#pragma db column("yield")
	int64_t yield;

//	#pragma db column("strftime(%m," + DayData::date + ") AS month")
//	int month;
//
//	#pragma db column("strftime(%y," + DayData::date + ") AS year")
//	int year;
};


}

#endif /* SRC_PVLOG_MODELS_DAYDATAMONTH_H_ */
