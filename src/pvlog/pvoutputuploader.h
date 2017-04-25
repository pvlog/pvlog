#ifndef PVOUTPUT_UPLOADER_H
#define PVOUTPUT_UPLOADER_H

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <Poco/Net/Context.h>

#include "models/spotdata.h"
#include "models/daydata.h"

namespace odb {
	class database;
}

class PvoutputUploader {
public:
	PvoutputUploader(odb::database* db);

	void uploadSpotData(const std::vector<model::SpotData>& spotDatas);

	void uploadDayData(const std::vector<model::DayData>& dayData);

protected:
	void uploadSpotDataSum(boost::posix_time::ptime time, int32_t power, int32_t yield,
			const std::string& systemId, const std::string& apiKey);

	void uploadDayDataSum(boost::gregorian::date date, int32_t yield,
			const std::string& systemId, const std::string& apiKey);
private:
	odb::database* db;
	Poco::Net::Context::Ptr ptrContext;
};

#endif //#ifndef PVOUTPUT_UPLOADER_H
