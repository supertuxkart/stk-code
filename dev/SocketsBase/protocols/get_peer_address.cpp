#include "get_peer_address.hpp"

#include "../time.hpp"
#include "../http_functions.hpp"

#include <stdio.h>
#include <stdlib.h>

GetPeerAddress::GetPeerAddress(CallbackObject* callbackObject) : Protocol(callbackObject, PROTOCOL_SILENT)
{
}

GetPeerAddress::~GetPeerAddress()
{
}

void GetPeerAddress::notifyEvent(Event* event)
{
}

void GetPeerAddress::setup()
{
    m_state = NONE;
}

void GetPeerAddress::update()
{
    if (m_state == NONE)
    {
        static double target = 0;
        double currentTime = Time::getSeconds();
        while (currentTime < target-1800) // sometimes the getSeconds method forgets 3600 seconds.
            currentTime += 3600;
        if (currentTime > target)
        {
            char url[512];
            sprintf(url, "http://stkconnect.freeserver.me/log.php?get&nick=%s", m_peerName.c_str());
            std::string result = HTTP::getPage(url);
            if (result == "")
            {
                printf("__GetPeerAddress> The host you try to reach does not exist. Change the host name please.\n");
                pause();
                return;
            }
            std::string ipAddr = result;
            ipAddr.erase(ipAddr.find_first_of(':'));
            std::string portNb = result;
            portNb.erase(0, portNb.find_first_of(':')+1);
            uint32_t dstIp = (uint32_t)(atoi(ipAddr.c_str()));
            uint16_t dstPort = (uint32_t)(atoi(portNb.c_str()));
            if (dstIp == 0 || dstPort == 0)
            {
                printf("__GetPeerAddress> The host you try to reach is not online. There will be a new try in 10 seconds.\n");
                target = currentTime+10;
            }
            else
            {
                printf("__GetPeerAddress> Public ip of target is %i.%i.%i.%i:%i\n", (dstIp>>24)&0xff, (dstIp>>16)&0xff, (dstIp>>8)&0xff, dstIp&0xff, dstPort);
                uint32_t serverIp =   ((dstIp&0x000000ff)<<24) // change the server IP to have a network-byte order
                                    + ((dstIp&0x0000ff00)<<8)
                                    + ((dstIp&0x00ff0000)>>8)
                                    + ((dstIp&0xff000000)>>24); 
                uint16_t serverPort = dstPort;
                TransportAddress* addr = static_cast<TransportAddress*>(m_callbackObject);
                addr->ip = serverIp;
                addr->port = serverPort;
                m_state = DONE;
            }
        }
    }
    else if (m_state == DONE)
    {
        m_listener->protocolTerminated(this);
    }
}

void GetPeerAddress::setPeerName(std::string peerName)
{
    m_peerName = peerName;
}
