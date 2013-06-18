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

void NetworkManager::receptionCallback(char* data)
{
    instance->packetReceived(data);
}

STKHost* NetworkManager::getHost()
{
    return instance->m_localhost;
}
