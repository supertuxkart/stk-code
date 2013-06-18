#include "connect_to_server.hpp"

#include "../http_functions.hpp"

#include <stdio.h>

ConnectToServer::ConnectToServer(CallbackObject* callbackObject) : Protocol(callbackObject)
{
    m_ownPublicIp = 0;
    m_ownPublicPort = 0;
}

ConnectToServer::~ConnectToServer()
{
}

void ConnectToServer::setup()
{
}

void ConnectToServer::messageReceived(uint8_t* data)
{

}

void ConnectToServer::start()
{
    if (m_ownPublicIp == 0 || m_ownPublicPort == 0 || m_username == "" || m_password == "")
    {
        printf("You have to set the public ip:port and username:password before starting this protocol.\n");
        m_listener->protocolTerminated(this);
    }
}

void ConnectToServer::update()
{
    if (m_state == NOTHING)
    {
        char url[512];
        sprintf(url, "http://stkconnect.freeserver.me/log.php?set&nick=%s&ip=%u&port=%u&pwd=%s", m_username.c_str(), m_ownPublicIp, m_ownPublicPort, m_password.c_str());
        std::string result = HTTP::getPage(url);
        //printf("Result from web page is %s\n", result.c_str());
        if (result[0] == 's' && result[1] == 'u' && result[2] == 'c')
        {
            printf("Address set.\n");
            m_state = ADDRESS_KNOWN_ONLINE;
        }
        if (result[0] == 'f' && result[1] == 'a' && result[2] == 'i')
        {
            printf("Login fail.\n");
            m_state = ADDRESS_KNOWN_ONLINE;
        }
    }
    else if (m_state == ADDRESS_KNOWN_ONLINE)
    {
    }
    else if (m_state == PEER_ADDRESS_RETREIVED)
    {
    }
    else if (m_state == CONNECTED)
    {
    }
            
}

void ConnectToServer::setSelfAddress(uint32_t ip, uint16_t port)
{
    m_ownPublicIp = ip;
    m_ownPublicPort = port;
}

void ConnectToServer::setUsername(std::string username)
{
    m_username = username;
}

void ConnectToServer::setPassword(std::string password)
{
    m_password = password;
}
