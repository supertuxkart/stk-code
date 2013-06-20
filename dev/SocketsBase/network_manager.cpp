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

bool NetworkManager::connect(uint32_t ip, uint16_t port)
{
    if (peerExists(ip, port))
        return isConnectedTo(ip, port);
        
    STKPeer* peer = new STKPeer();
    bool success = peer->connectToHost(m_localhost, ip, port, 2, 0);
    if (success)
        m_peers.push_back(peer);
    return success;
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

bool NetworkManager::peerExists(uint32_t ip, uint16_t port)
{
    for (unsigned int i = 0; i < m_peers.size(); i++)
        if (m_peers[i]->getAddress() == ip && m_peers[i]->getPort() == port)
            return true;
    return m_localhost->peerExists(ip, port);
}

bool NetworkManager::isConnectedTo(uint32_t ip, uint16_t port)
{
    for (unsigned int i = 0; i < m_peers.size(); i++)
    {
        if (m_peers[i]->getAddress() == ip && m_peers[i]->getPort() == port && m_peers[i]->isConnected())
        {
            return true;
        }
    }
    return m_localhost->isConnectedTo(ip, port);
}

STKHost* NetworkManager::getHost()
{
    return m_localhost;
}
