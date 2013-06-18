#include "connect_to_server.hpp"

#include "../http_functions.hpp"
#include "../time.hpp"
#include "../client_network_manager.hpp"

#include <stdio.h>
#include <stdlib.h>

ConnectToServer::ConnectToServer(CallbackObject* callbackObject) : Protocol(callbackObject)
{
    m_ownPublicIp = 0;
    m_ownPublicPort = 0;
    connected = false;
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
    if (m_ownPublicIp == 0 || m_ownPublicPort == 0 || m_username == "" || m_password == "" || m_hostName == "")
    {
        printf("You have to set the public ip:port, username:password and the host nickname before starting this protocol.\n");
        m_listener->protocolTerminated(this);
    }
}

void ConnectToServer::pause()
{
    m_listener->pauseProtocol(this); // need to be sure that the protocol manager knows
}

void ConnectToServer::unpause()
{
    m_listener->unpauseProtocol(this); // need to be sure that the protocol manager knows
}

void ConnectToServer::update()
{
    if (m_state == NOTHING)
    {
        char url[512];
        sprintf(url, "http://stkconnect.freeserver.me/log.php?set&nick=%s&ip=%u&port=%u&pwd=%s", m_username.c_str(), m_ownPublicIp, m_ownPublicPort, m_password.c_str());
        std::string result = HTTP::getPage(url);
        if (result[0] == 's' && result[1] == 'u' && result[2] == 'c' && result[3] == 'c' && result[4] == 'e' && result[5] == 's' && result[6] == 's')
        {
            printf("Address set.\n");
            m_state = ADDRESS_KNOWN_ONLINE;
        }
        if (result[0] == 'f' && result[1] == 'a' && result[2] == 'i' && result[3] == 'l')
        {
            printf("Login fail. Please re-set username:password and unpause the protocol.\n");
            m_state = NOTHING;
            pause();
        }
    }
    else if (m_state == ADDRESS_KNOWN_ONLINE)
    {
        static double target = 0;
        double currentTime = Time::getSeconds();
        if (currentTime < target-1800) // sometimes the getSeconds method forgets 3600 seconds.
            currentTime += 3600;
        if (currentTime > target)
        {
            char url[512];
            sprintf(url, "http://stkconnect.freeserver.me/log.php?get&nick=%s", m_hostName.c_str());
            std::string result = HTTP::getPage(url);
            if (result == "")
            {
                printf("The host you try to reach does not exist. Change the host name please.\n");
                m_state = NOTHING;
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
                printf("The host you try to reach is not online. There will be a new try in 10 seconds.\n");
                target = currentTime+10;
            }
            else
            {
                printf("Public ip of target is %i.%i.%i.%i:%i\n", (dstIp>>24)&0xff, (dstIp>>16)&0xff, (dstIp>>8)&0xff, dstIp&0xff, dstPort);
                m_serverIp =  ((dstIp&0x000000ff)<<24) // change the server IP to have a network-byte order
                            + ((dstIp&0x0000ff00)<<8)
                            + ((dstIp&0x00ff0000)>>8)
                            + ((dstIp&0xff000000)>>24); 
                m_serverPort = dstPort;
                m_state = PEER_ADDRESS_RETREIVED;
            }
        }
    }
    else if (m_state == PEER_ADDRESS_RETREIVED)
    {
        // we know the distant address:port, just need to connect.
        ClientNetworkManager* networkManager = static_cast<ClientNetworkManager*>(m_callbackObject);
        networkManager->connect(m_serverIp, m_serverPort);
        m_state = CONNECTED;
        connected = true;
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

void ConnectToServer::setHostName(std::string hostName)
{
    m_hostName = hostName;
}
