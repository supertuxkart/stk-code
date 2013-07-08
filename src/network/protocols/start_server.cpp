#include "network/protocols/start_server.hpp"

#include "network/network_manager.hpp"
#include "online/current_online_user.hpp"
#include "online/http_connector.hpp"
#include "config/user_config.hpp"

StartServer::StartServer() : Protocol(NULL, PROTOCOL_SILENT)
{
}

StartServer::~StartServer()
{
}

void StartServer::notifyEvent(Event* event)
{
}

void StartServer::setup()
{
    m_state = NONE;
}

void StartServer::update()
{
    if (m_state == NONE)
    {
        HTTPConnector * connector = new HTTPConnector((std::string)UserConfigParams::m_server_multiplayer + "address-management.php");
        connector->setParameter("id",CurrentOnlineUser::get()->getUserID());
        connector->setParameter("token",CurrentOnlineUser::get()->getToken());
        TransportAddress addr = NetworkManager::getInstance()->getPublicAddress();
        connector->setParameter("address",addr.ip);
        connector->setParameter("port",addr.port);
        connector->setParameter("action","start-server");

        const XMLNode * result = connector->getXMLFromPage();
        std::string rec_success;

        if(result->get("success", &rec_success))
        {
            if(rec_success == "yes")
            {
                Log::info("StartServer", "Server is now online.");
            }
            else
            {
                Log::error("StartServer", "Fail to start server.");
            }
        }
        else
        {
            Log::error("StartServer", "Fail to start server.");
        }
        m_state = DONE;
    }
    else if (m_state == DONE)
    {
        m_listener->requestTerminate(this);
    }
}
