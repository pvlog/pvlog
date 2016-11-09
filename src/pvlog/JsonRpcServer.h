#ifndef SRC_JSONRPCSERVER_H_
#define SRC_JSONRPCSERVER_H_

#include <unordered_map>

#include <abstractpvlogserver.h>

#include "models/Inverter.h"
#include "models/SpotData.h"

namespace odb {
	class database;
}

class JsonRpcServer: public AbstractPvlogServer {
	odb::database *db;
	using InverterSpotData = std::unordered_map<model::InverterPtr, std::vector<model::SpotDataPtr>>;

	InverterSpotData readSpotData(const boost::gregorian::date& date);
public:
	JsonRpcServer(jsonrpc::AbstractServerConnector &conn, odb::database* database);
	virtual ~JsonRpcServer();

    virtual Json::Value getSpotData(const std::string& date) override;
    virtual Json::Value getLiveSpotData() override;
    virtual Json::Value getDayData(const std::string& from, const std::string& to) override;
    virtual Json::Value getMonthData(const std::string& year) override;
    virtual Json::Value getYearData() override;
    virtual Json::Value getInverter() override;
    virtual Json::Value getPlants() override;
};

#endif /* SRC_JSONRPCSERVER_H_ */
