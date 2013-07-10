#include "network/protocols/request_connection.hpp"

#include "network/protocol_manager.hpp"
#include "online/http_connector.hpp"
#include "online/current_online_user.hpp"
#include "config/user_config.hpp"

RequestConnection::RequestConnection(uint32_t server_id) : Protocol(NULL, PROTOCOL_SILENT)
{
    m_server_id = server_id;
}

RequestConnection::~RequestConnection()
{
}

void RequestConnection::notifyEvent(Event* event)
{
}

void RequestConnection::setup()
{
    m_state = NONE;
}

void RequestConnection::update()
{
    switch (m_state)
    {
        case NONE:
        {
            HTTPConnector * connector = new HTTPConnector((std::string)UserConfigParams::m_server_multiplayer + "address-management.php");
            connector->setParameter("id",CurrentOnlineUser::get()->getUserID());
            connector->setParameter("token",CurrentOnlineUser::get()->getToken());
            connector->setParameter("server_id",m_server_id);
            connector->setParameter("action","request-connection");

            const XMLNode * result = connector->getXMLFromPage();
            std::string rec_success;

            if(result->get("success", &rec_success))
            {
                if (rec_success == "yes")
                {
                    Log::debug("RequestConnection", "Connection Request made successfully.");
                }
                else
                {
                    Log::error("RequestConnection", "Fail to make a request.");
                }
            }
            else
            {
                Log::error("RequestConnection", "Fail to make a request.");
            }
            m_state = DONE;

            break;
        }
        case DONE:
            m_listener->requestTerminate(this);
            break;
    }
}

