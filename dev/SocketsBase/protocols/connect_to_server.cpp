#include "connect_to_server.hpp"

#include "../client_network_manager.hpp"
#include "../time.hpp"

#include <stdio.h>
#include <stdlib.h>

ConnectToServer::ConnectToServer(CallbackObject* callbackObject) : Protocol(callbackObject, PROTOCOL_CONNECTION)
{
    m_serverIp = 0;
    m_serverPort = 0;
    m_state = NONE;
}

ConnectToServer::~ConnectToServer()
{
}

void ConnectToServer::notifyEvent(Event* event)
{
    if (event->type == EVENT_TYPE_CONNECTED && event->peer->getAddress() == m_serverIp && event->peer->getPort() == m_serverPort)
    {
        printf("The Connect To Server protocol has received an event notifying that he's connected to the peer. The peer sent \"%s\"\n", event->data.c_str());
        m_state = DONE; // we received a message, we are connected
    }
}

void ConnectToServer::setup()
{
    m_state = NONE;
    if (m_serverIp == 0 || m_serverPort == 0 )
    {
        printf("You have to set the server's public ip:port of the server.\n");
        m_listener->protocolTerminated(this);
    }
}

void ConnectToServer::update()
{
    if (m_state == NONE)
    {
        static double target = 0;
        double currentTime = Time::getSeconds();
        while (currentTime < target-1800) // sometimes the getSeconds method forgets 3600 seconds.
            currentTime += 3600;
        if (currentTime > target)
        {
            NetworkManager::getInstance()->connect(m_serverIp, m_serverPort);
            if (NetworkManager::getInstance()->isConnectedTo(m_serverIp, m_serverPort))
            {   
                m_state = DONE;
                return;
            }
            target = currentTime+5;
            printf("Retrying to connect in 5 seconds.\n");
        }
    }
    else if (m_state == DONE)
    {
        m_listener->protocolTerminated(this);
    }
}

void ConnectToServer::setServerAddress(uint32_t ip, uint16_t port)
{
    m_serverIp = ip;
    m_serverPort = port;
}

