#ifndef SRC_PVLOG_MODELS_PHASE_H_
#define SRC_PVLOG_MODELS_PHASE_H_

#include <cstdint>

#include <jsoncpp/json/value.h>
#include <boost/optional.hpp>
#include <odb/core.hxx>

#include <utility.h>

namespace model {

#pragma db value
struct Phase {
	int32_t power;
	boost::optional<int32_t> voltage;
	boost::optional<int32_t> current;
};

inline Json::Value toJson(const Phase& phase) {
	using util::toJson;
	Json::Value json;

	json["power"]   = phase.power;
	json["voltage"] = toJson(phase.voltage);
	json["current"] = toJson(phase.current);

	return json;
}

} //namespace model {

#endif /* SRC_PVLOG_MODELS_PHASE_H_ */
