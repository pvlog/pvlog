#ifndef SRC_PVLOG_MODELS_CONFIGSERVICE_H_
#define SRC_PVLOG_MODELS_CONFIGSERVICE_H_

#include <memory>

#include <odb/database.hxx>
#include <odb/sqlite/database.hxx>

#include "Config.h"
#include "Config_odb.h"


inline std::string readConfig(odb::core::database* db,  const std::string& key) {
	std::shared_ptr<model::Config> config = db->load<model::Config>(key);
	return config->value;
}


#endif /* SRC_PVLOG_MODELS_CONFIGSERVICE_H_ */
