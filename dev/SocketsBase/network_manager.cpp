#include "network_manager.hpp"

#include "protocols/hide_public_address.hpp"
#include "protocols/show_public_address.hpp"
#include "protocols/get_public_address.hpp"

#include "protocol_manager.hpp"
#include "client_network_manager.hpp"
#include "server_network_manager.hpp"

#include <stdio.h>

void* protocolManagerUpdate(void* data)
{
    ProtocolManager* manager = static_cast<ProtocolManager*>(data);
    while(1)
    {
        manager->update();
    }
    return NULL;
}


NetworkManager::NetworkManager()
{
    m_publicAddress.ip = 0;
    m_publicAddress.port = 0;
    m_networkManager = NULL;
    m_protocolManagerUpdateThread = NULL;
}

NetworkManager::~NetworkManager() 
{
}

void NetworkManager::run()
{
    ProtocolManager::getInstance<ProtocolManager>();
    m_protocolManagerUpdateThread = (pthread_t*)(malloc(sizeof(pthread_t)));
    pthread_create(m_protocolManagerUpdateThread, NULL, protocolManagerUpdate, ProtocolManager::getInstance());
}

void NetworkManager::setManualSocketsMode(bool manual)
{
    if (manual)
        m_localhost->stopListening();
    else
        m_localhost->startListening();
}

void NetworkManager::setLogin(std::string username, std::string password)
{
    m_playerLogin.username = username;
    m_playerLogin.password = password;
}

void NetworkManager::setPublicAddress(uint32_t ip, uint16_t port)
{
    m_publicAddress.ip = ip;
    m_publicAddress.port = port;
}

STKHost* NetworkManager::getHost()
{
    return m_localhost;
}
