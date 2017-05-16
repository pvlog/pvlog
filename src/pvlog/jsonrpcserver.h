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

#ifndef SRC_JSONRPCSERVER_H_
#define SRC_JSONRPCSERVER_H_

#include <unordered_map>

#include <abstractpvlogserver.h>
#include <inverter.h>

#include <spotdata.h>

class Datalogger;

namespace odb {
	class database;
}

class JsonRpcServer: public AbstractPvlogServer {
	odb::database *db;
	using InverterSpotData = std::unordered_map<model::InverterPtr, std::vector<model::SpotDataPtr>>;

	Datalogger* datalogger;

	InverterSpotData readSpotData(const boost::gregorian::date& date);
public:
	JsonRpcServer(jsonrpc::AbstractServerConnector &conn, Datalogger* datalogger, odb::database* database);
	virtual ~JsonRpcServer();

	virtual Json::Value getSpotData(const std::string& date) override;
	virtual Json::Value getStatistics() override;
	virtual Json::Value getLiveSpotData() override;
	virtual Json::Value getDataloggerStatus() override;
	virtual Json::Value getDayData(const std::string& from, const std::string& to) override;
	virtual Json::Value getMonthData(const std::string& year) override;
	virtual Json::Value getYearData() override;
	virtual Json::Value getInverters() override;
	virtual Json::Value getPlants() override;
	virtual Json::Value getEvents() override;
};

#endif /* SRC_JSONRPCSERVER_H_ */
