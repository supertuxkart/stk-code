#include "network_manager.hpp"

#include <stdio.h>

NetworkManager* NetworkManager::instance = NULL;

NetworkManager::NetworkManager()
{
    if (NetworkManager::instance)
    {
        printf("Warning : a Newtork Manager is being deleted.\n");
        delete NetworkManager::instance;
    }
    NetworkManager::instance = this;
    printf("New Network Manager created.\n");
}

NetworkManager::~NetworkManager() 
{
}

void NetworkManager::run()
{
}

void NetworkManager::setManualSocketsMode(bool manual)
{
    if (manual)
        instance->getHost()->stopListening();
    else
        instance->getHost()->startListening();
}

void NetworkManager::sendRawPacket(uint8_t* data, int length, unsigned int dstIp, unsigned short dstPort)
{
    instance->getHost()->sendRawPacket(data, length, dstIp, dstPort);
}
uint8_t* NetworkManager::receiveRawPacket()
{
    return instance->getHost()->receiveRawPacket();
}
void NetworkManager::receptionCallback(char* data)
{
    instance->packetReceived(data);
}

STKHost* NetworkManager::getHost()
{
    return m_localhost;
}
