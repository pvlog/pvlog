#ifndef SRC_PVLOG_MODELS_DAYDATA_H_
#define SRC_PVLOG_MODELS_DAYDATA_H_

#include <memory>

#include <boost/date_time/gregorian/gregorian_types.hpp>

#include "Inverter.h"

namespace model {

#pragma db object
struct DayData {
	#pragma db id auto
	int id;

	#pragma db not_null
	std::shared_ptr<Inverter> inverter;

	boost::gregorian::date date;

	uint32_t dayYield;
};

} //namespace model {


#endif /* SRC_PVLOG_MODELS_DAYDATA_H_ */
