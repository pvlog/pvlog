#ifndef SRC_PVLOG_MODELS_DCINPUT_H_
#define SRC_PVLOG_MODELS_DCINPUT_H_

#include <cstdint>

#include <jsoncpp/json/value.h>
#include <boost/optional.hpp>
#include <odb/core.hxx>

#include "utility.h"
#include "version.h"

namespace model {

#pragma db value
struct DcInput {
	boost::optional<int32_t> power;
	boost::optional<int32_t> voltage;
	boost::optional<int32_t> current;
};

inline Json::Value toJson(const DcInput& dcInput) {
	Json::Value json;

	if (dcInput.power) {
		json["power"] = dcInput.power.get();
	}
	if (dcInput.voltage) {
		json["voltage"] = dcInput.voltage.get();
	}
	if (dcInput.current) {
		json["current"] = dcInput.current.get();
	}

	return json;
}

} //namespace model {

#endif /* SRC_PVLOG_MODELS_DCINPUT_H_ */
