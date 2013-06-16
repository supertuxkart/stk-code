#include "client_network_manager.hpp"

#include <stdio.h>

ClientNetworkManager::ClientNetworkManager()
{
}

ClientNetworkManager::~ClientNetworkManager()
{
}

void ClientNetworkManager::run()
{
    if (enet_initialize() != 0) 
    {
        printf("Could not initialize enet.\n");
        return;
    }
}

void ClientNetworkManager::connect(uint32_t ip, uint16_t port)
{
    m_localhost = new STKHost();
    m_localhost->setupClient(1, 2, 0, 0);
    m_localhost->startListening();
    
    STKPeer* peer = new STKPeer();
    peer->connectToServer(m_localhost, ip, port, 2, 0);
    
    m_peers.push_back(peer);
}

void ClientNetworkManager::packetReceived(char* data)
{
    printf("ClientNetworkManager::packetReceived()\n");
    puts(data);
}
void ClientNetworkManager::sendPacket(char* data)
{
    m_peers[0]->sendPacket(data);
}
