/**
 * This file is generated by jsonrpcstub, DO NOT CHANGE IT MANUALLY!
 */

#ifndef JSONRPC_CPP_STUB_ABSTRACTADMINSERVER_H_
#define JSONRPC_CPP_STUB_ABSTRACTADMINSERVER_H_

#include <jsonrpccpp/server.h>

class AbstractAdminServer : public jsonrpc::AbstractServer<AbstractAdminServer>
{
    public:
        AbstractAdminServer(jsonrpc::AbstractServerConnector &conn, jsonrpc::serverVersion_t type = jsonrpc::JSONRPC_SERVER_V2) : jsonrpc::AbstractServer<AbstractAdminServer>(conn, type)
        {
            this->bindAndAddNotification(jsonrpc::Procedure("stopDatalogger", jsonrpc::PARAMS_BY_NAME,  NULL), &AbstractAdminServer::stopDataloggerI);
            this->bindAndAddNotification(jsonrpc::Procedure("startDatalogger", jsonrpc::PARAMS_BY_NAME,  NULL), &AbstractAdminServer::startDataloggerI);
            this->bindAndAddMethod(jsonrpc::Procedure("getInverters", jsonrpc::PARAMS_BY_NAME, jsonrpc::JSON_OBJECT,  NULL), &AbstractAdminServer::getInvertersI);
            this->bindAndAddMethod(jsonrpc::Procedure("getPlants", jsonrpc::PARAMS_BY_NAME, jsonrpc::JSON_OBJECT,  NULL), &AbstractAdminServer::getPlantsI);
            this->bindAndAddMethod(jsonrpc::Procedure("scanForInverters", jsonrpc::PARAMS_BY_NAME, jsonrpc::JSON_OBJECT, "plant",jsonrpc::JSON_OBJECT, NULL), &AbstractAdminServer::scanForInvertersI);
            this->bindAndAddMethod(jsonrpc::Procedure("getSupportedConnections", jsonrpc::PARAMS_BY_NAME, jsonrpc::JSON_OBJECT,  NULL), &AbstractAdminServer::getSupportedConnectionsI);
            this->bindAndAddMethod(jsonrpc::Procedure("getSupportedProtocols", jsonrpc::PARAMS_BY_NAME, jsonrpc::JSON_OBJECT,  NULL), &AbstractAdminServer::getSupportedProtocolsI);
            this->bindAndAddMethod(jsonrpc::Procedure("saveInverter", jsonrpc::PARAMS_BY_NAME, jsonrpc::JSON_OBJECT, "inverter",jsonrpc::JSON_OBJECT, NULL), &AbstractAdminServer::saveInverterI);
            this->bindAndAddMethod(jsonrpc::Procedure("savePlant", jsonrpc::PARAMS_BY_NAME, jsonrpc::JSON_OBJECT, "inverter",jsonrpc::JSON_OBJECT, NULL), &AbstractAdminServer::savePlantI);
            this->bindAndAddMethod(jsonrpc::Procedure("getConfigs", jsonrpc::PARAMS_BY_NAME, jsonrpc::JSON_OBJECT,  NULL), &AbstractAdminServer::getConfigsI);
            this->bindAndAddMethod(jsonrpc::Procedure("saveConfig", jsonrpc::PARAMS_BY_NAME, jsonrpc::JSON_OBJECT, "config",jsonrpc::JSON_OBJECT, NULL), &AbstractAdminServer::saveConfigI);
        }

        inline virtual void stopDataloggerI(const Json::Value &request)
        {
            (void)request;
            this->stopDatalogger();
        }
        inline virtual void startDataloggerI(const Json::Value &request)
        {
            (void)request;
            this->startDatalogger();
        }
        inline virtual void getInvertersI(const Json::Value &request, Json::Value &response)
        {
            (void)request;
            response = this->getInverters();
        }
        inline virtual void getPlantsI(const Json::Value &request, Json::Value &response)
        {
            (void)request;
            response = this->getPlants();
        }
        inline virtual void scanForInvertersI(const Json::Value &request, Json::Value &response)
        {
            response = this->scanForInverters(request["plant"]);
        }
        inline virtual void getSupportedConnectionsI(const Json::Value &request, Json::Value &response)
        {
            (void)request;
            response = this->getSupportedConnections();
        }
        inline virtual void getSupportedProtocolsI(const Json::Value &request, Json::Value &response)
        {
            (void)request;
            response = this->getSupportedProtocols();
        }
        inline virtual void saveInverterI(const Json::Value &request, Json::Value &response)
        {
            response = this->saveInverter(request["inverter"]);
        }
        inline virtual void savePlantI(const Json::Value &request, Json::Value &response)
        {
            response = this->savePlant(request["inverter"]);
        }
        inline virtual void getConfigsI(const Json::Value &request, Json::Value &response)
        {
            (void)request;
            response = this->getConfigs();
        }
        inline virtual void saveConfigI(const Json::Value &request, Json::Value &response)
        {
            response = this->saveConfig(request["config"]);
        }
        virtual void stopDatalogger() = 0;
        virtual void startDatalogger() = 0;
        virtual Json::Value getInverters() = 0;
        virtual Json::Value getPlants() = 0;
        virtual Json::Value scanForInverters(const Json::Value& plant) = 0;
        virtual Json::Value getSupportedConnections() = 0;
        virtual Json::Value getSupportedProtocols() = 0;
        virtual Json::Value saveInverter(const Json::Value& inverter) = 0;
        virtual Json::Value savePlant(const Json::Value& inverter) = 0;
        virtual Json::Value getConfigs() = 0;
        virtual Json::Value saveConfig(const Json::Value& config) = 0;
};

#endif //JSONRPC_CPP_STUB_ABSTRACTADMINSERVER_H_
