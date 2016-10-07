#ifndef SRC_PVLOG_MODELS_DCINPUT_H_
#define SRC_PVLOG_MODELS_DCINPUT_H_

#include <cstdint>

#include <jsoncpp/json/value.h>
#include <boost/optional.hpp>
#include <odb/core.hxx>

#include "Utility.h"

namespace model {

#pragma db value
struct DcInput {
	boost::optional<int32_t> power;
	boost::optional<int32_t> voltage;
	boost::optional<int32_t> current;
};

Json::Value toJson(const DcInput& dcInput) {
	using util::toJson;
	Json::Value json;

	json["power"]   = toJson(dcInput.power);
	json["voltage"] = toJson(dcInput.voltage);
	json["current"] = toJson(dcInput.current);

	return json;
}

} //namespace model {

#endif /* SRC_PVLOG_MODELS_DCINPUT_H_ */
