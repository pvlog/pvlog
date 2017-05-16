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

#include "plant.h"

namespace model {

Json::Value toJson(const Plant& plant) {
	Json::Value json;

	json["id"]       = static_cast<Json::Int64>(plant.id);
	json["name"]     = plant.name;
	json["connection"] = plant.connection;
	json["protocol"]   = plant.protocol;
	json["connectionParam"] = plant.connectionParam;
	json["protocolParam"]   = plant.protocolParam;

	return json;
}

Plant plantFromJson(const Json::Value& value) {
	Plant plant;

	if (value.isMember("id") && !value["id"].isNull()) {
		plant.id = value["id"].asInt64();
	}
	plant.name       = value["name"].asString();
	plant.connection = value["connection"].asString();
	plant.protocol   = value["protocol"].asString();
	plant.connectionParam = value["connectionParam"].asString();
	plant.protocolParam   = value["protocolParam"].asString();

	return plant;
}


} //namespace model
