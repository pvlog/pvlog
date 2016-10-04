#ifndef SRC_PVLOG_MODELS_DCINPUT_H_
#define SRC_PVLOG_MODELS_DCINPUT_H_

#include <cstdint>

#include <odb/core.hxx>
#include <odb/nullable.hxx>

namespace model {

#pragma db value
struct DcInput {
	odb::nullable<int32_t> power;
	odb::nullable<int32_t> voltage;
	odb::nullable<int32_t> current;
};

} //namespace model {

#endif /* SRC_PVLOG_MODELS_DCINPUT_H_ */
