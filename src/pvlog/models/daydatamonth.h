/*
 * DayDataMonth.h
 *
 *  Created on: Nov 8, 2016
 *      Author: benjamin
 */

#ifndef SRC_PVLOG_MODELS_DAYDATAMONTH_H_
#define SRC_PVLOG_MODELS_DAYDATAMONTH_H_

#include <daydata.h>
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
