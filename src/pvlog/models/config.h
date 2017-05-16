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

#ifndef SRC_PVLOG_MODELS_CONFIG_H_
#define SRC_PVLOG_MODELS_CONFIG_H_

#include <string>
#include <utility>
#include <memory>

#include <odb/core.hxx>
#include <jsoncpp/json/value.h>

#include "version.h"

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

using ConfigPtr = std::shared_ptr<Config>;

inline Json::Value toJson(const Config& config) {
	Json::Value json;

	json["key"]   = config.key;
	json["value"] = config.value;

	return json;
}

inline Config configFromJson(const Json::Value& configJson) {
	std::string key   = configJson["key"].asString();
	std::string value = configJson["value"].asString();

	return Config(key, value);
}

} //namespace model {

#endif /* SRC_PVLOG_MODELS_CONFIG_H_ */
