#include "network/protocols/stop_server.hpp"

#include "network/network_manager.hpp"
#include "online/current_online_user.hpp"
#include "online/http_connector.hpp"
#include "config/user_config.hpp"

StopServer::StopServer() : Protocol(NULL, PROTOCOL_SILENT)
{
}

StopServer::~StopServer()
{
}

void StopServer::notifyEvent(Event* event)
{
}

void StopServer::setup()
{
    m_state = NONE;
}

void StopServer::update()
{
    if (m_state == NONE)
    {
        TransportAddress addr = NetworkManager::getInstance()->getPublicAddress();
        HTTPConnector * connector = new HTTPConnector((std::string)UserConfigParams::m_server_multiplayer + "address-management.php");
        connector->setParameter("id",CurrentOnlineUser::get()->getUserID());
        connector->setParameter("token",CurrentOnlineUser::get()->getToken());
        connector->setParameter("address",addr.ip);
        connector->setParameter("port",addr.port);
        connector->setParameter("action","stop-server");

        const XMLNode * result = connector->getXMLFromPage();
        std::string rec_success;

        if(result->get("success", &rec_success))
        {
            if(rec_success == "yes")
            {
                Log::info("StopServer", "Server is now offline.");
            }
            else
            {
                Log::error("StopServer", "Fail to stop server.");
            }
        }
        else
        {
            Log::error("StopServer", "Fail to stop server.");
        }
        m_state = DONE;
    }
    else if (m_state == DONE)
    {
        m_listener->requestTerminate(this);
    }
}
