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

    virtual Json::Value getSpotData(const std::string& date);
    virtual Json::Value getLiveSpotData();
    virtual Json::Value getMonthData(const std::string& month);
    virtual Json::Value getYearData(const std::string& year);
    virtual Json::Value getInverter();
    virtual Json::Value getPlants();
};

#endif /* SRC_JSONRPCSERVER_H_ */
