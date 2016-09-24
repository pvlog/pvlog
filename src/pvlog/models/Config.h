#ifndef SRC_PVLOG_MODELS_CONFIG_H_
#define SRC_PVLOG_MODELS_CONFIG_H_

#include <string>
#include <utility>

#include <odb/core.hxx>

namespace model {

#pragma db object
struct Config {
	#pragma db id
	std::string key;

	std::string value;

	Config(std::string key, std::string value) : key(std::move(key)), value(std::move(value)) {
		//nothing to do
	}

	Config() {}
};

} //namespace model {

#endif /* SRC_PVLOG_MODELS_CONFIG_H_ */
