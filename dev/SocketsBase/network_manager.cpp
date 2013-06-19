#include "network_manager.hpp"

#include <stdio.h>

NetworkManager::NetworkManager()
{
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
        m_localhost->stopListening();
    else
        m_localhost->startListening();
}

STKHost* NetworkManager::getHost()
{
    return m_localhost;
}
