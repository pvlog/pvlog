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

	virtual Json::Value getInverters() override;

	virtual Json::Value getPlants() override;

	virtual Json::Value scanForInverters(const Json::Value& plant) override;

	virtual Json::Value getSupportedConnections() override;

	virtual Json::Value getSupportedProtocols() override;

	virtual Json::Value saveInverter(const Json::Value& inverter) override;

	virtual Json::Value savePlant(const Json::Value& inverter) override;

	virtual Json::Value getConfigs() override;

	virtual Json::Value saveConfig(const Json::Value& config) override;
};



#endif /* SRC_PVLOG_JSONRPCADMINSERVER_H_ */
