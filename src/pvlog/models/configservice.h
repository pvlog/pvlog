/*
 * This file is part of Pvlog.
 *
 * Copyright (C) 2017 pvlogdev@gmail.com
 *
 * Pvlog is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Pvlog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Pvlog.  If not, see <http://www.gnu.org/licenses/>.
 */

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
