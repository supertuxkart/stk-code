#include "show_public_address.hpp"

#include "../http_functions.hpp"

#include <stdio.h>

ShowPublicAddress::ShowPublicAddress(CallbackObject* callbackObject) : Protocol(callbackObject, PROTOCOL_SILENT)
{
}

ShowPublicAddress::~ShowPublicAddress()
{
}

void ShowPublicAddress::notifyEvent(Event* event)
{
}

void ShowPublicAddress::setup()
{
    m_state = NONE;
    if (m_publicIp == 0 || m_publicPort == 0 || m_username == "" || m_password == "")
    {
        printf("__ShowPublicAddress> You have to set the public ip:port, username:password and the host nickname before starting this protocol.\n");
        m_listener->protocolTerminated(this);
    }
}

void ShowPublicAddress::update()
{
    if (m_state == NONE)
    {
       char url[512];
        sprintf(url, "http://stkconnect.freeserver.me/log.php?set&nick=%s&ip=%u&port=%u&pwd=%s", m_username.c_str(), m_publicIp, m_publicPort, m_password.c_str());
        std::string result = HTTP::getPage(url);
        if (result[0] == 's' && result[1] == 'u' && result[2] == 'c' && result[3] == 'c' && result[4] == 'e' && result[5] == 's' && result[6] == 's')
        {
            printf("__ShowPublicAddress> Address set.\n");
            m_state = DONE;
        }
        if (result[0] == 'f' && result[1] == 'a' && result[2] == 'i' && result[3] == 'l')
        {
            printf("__ShowPublicAddress> Login fail. Please re-set username:password and unpause the protocol.\n");
            m_state = NONE;
            pause();
        }
    }
    else if (m_state == DONE)
    {
        m_listener->protocolTerminated(this);
    }
}

void ShowPublicAddress::setUsername(std::string username)
{
    m_username = username;
}
void ShowPublicAddress::setPassword(std::string password)
{
    m_password = password;
}
void ShowPublicAddress::setPublicAddress(uint32_t ip, uint16_t port)
{
    m_publicIp = ip;
    m_publicPort = port;
}
