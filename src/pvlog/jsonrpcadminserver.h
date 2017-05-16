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

#ifndef SRC_PVLOG_JSONRPCADMINSERVER_H_
#define SRC_PVLOG_JSONRPCADMINSERVER_H_

#include "abstractadminserver.h"

class Datalogger;
namespace odb {
	class database;
}

class JsonRpcAdminServer : public AbstractAdminServer {
private:
	Datalogger* datalogger;
	odb::database* db;
public:
	JsonRpcAdminServer(jsonrpc::AbstractServerConnector& conn, Datalogger* datalogger, odb::database* db);

	virtual ~JsonRpcAdminServer();

	virtual void stopDatalogger() override;

	virtual void startDatalogger() override;

	virtual bool isDataloggerRunning() override;

	virtual Json::Value getInverters() override;

	virtual Json::Value getPlants() override;

	virtual Json::Value scanForInverters(const Json::Value& plant) override;

	virtual Json::Value getSupportedConnections() override;

	virtual Json::Value getSupportedProtocols() override;

	virtual Json::Value saveInverter(const Json::Value& inverter) override;

	virtual Json::Value deleteInverter(const std::string& inverterId) override;

	virtual Json::Value savePlant(const Json::Value& inverter) override;

	virtual Json::Value deletePlant(const std::string& plantId) override;

	virtual Json::Value getConfigs() override;

	virtual Json::Value saveConfig(const Json::Value& config) override;

	virtual Json::Value saveEmailServer(const std::string& server, int port, const std::string& username, const std::string& password) override;

	virtual Json::Value getEmailServer() override;

	virtual Json::Value saveEmail(const std::string& email) override;

	virtual Json::Value getEmail() override;

	virtual Json::Value sendTestEmail() override;
};



#endif /* SRC_PVLOG_JSONRPCADMINSERVER_H_ */
