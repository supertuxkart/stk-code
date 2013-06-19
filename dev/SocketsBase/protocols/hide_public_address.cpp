#include "hide_public_address.hpp"

#include "../http_functions.hpp"

#include <stdio.h>

HidePublicAddress::HidePublicAddress(CallbackObject* callbackObject) : Protocol(callbackObject)
{
}

HidePublicAddress::~HidePublicAddress()
{
}

void HidePublicAddress::messageReceived(uint8_t* data)
{
}

void HidePublicAddress::setup()
{
    m_state = NONE;
}

void HidePublicAddress::update()
{
    if (m_state == NONE)
    {
        char url[512];
        sprintf(url, "http://stkconnect.freeserver.me/log.php?logout&nick=%s&pwd=%s", m_username.c_str(), m_password.c_str());
        std::string result = HTTP::getPage(url);
        if (result[0] == 's' && result[1] == 'u' && result[2] == 'c' && result[3] == 'c' && result[4] == 'e' && result[5] == 's' && result[6] == 's')
        {
            printf("__HidePublicAddress> Public address hidden successfully.\n");
            m_state = DONE;
        }
        if (result[0] == 'f' && result[1] == 'a' && result[2] == 'i' && result[3] == 'l')
        {
            printf("__HidePublicAddress> Public address still visible. Re-set nick:password and retry.\n");
            m_state = NONE;
            pause();
        }
    }
    else if (m_state == DONE)
    {
        m_listener->protocolTerminated(this);
    }
}

void HidePublicAddress::setUsername(std::string username)
{
    m_username = username;
}
void HidePublicAddress::setPassword(std::string password)
{
    m_password = password;
}
