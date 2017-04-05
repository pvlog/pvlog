#ifndef SRC_PVLOG_MODELS_CONFIGSERVICE_H_
#define SRC_PVLOG_MODELS_CONFIGSERVICE_H_

#include <memory>

#include <odb/database.hxx>
#include <odb/sqlite/database.hxx>

#include "config.h"
#include "models/config_odb.h"


inline std::string readConfig(odb::core::database* db,  const std::string& key) {
	odb::transaction t (db->begin ());
	std::shared_ptr<model::Config> config = db->load<model::Config>(key);
	t.commit();
	return config->value;
}


#endif /* SRC_PVLOG_MODELS_CONFIGSERVICE_H_ */
