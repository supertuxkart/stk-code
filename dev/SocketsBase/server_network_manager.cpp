#include "server_network_manager.hpp"

#include <enet/enet.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

ServerNetworkManager::ServerNetworkManager()
{
    m_localhost = NULL;
}

ServerNetworkManager::~ServerNetworkManager()
{
}

void ServerNetworkManager::run()
{
    if (enet_initialize() != 0) 
    {
        printf("Could not initialize enet.\n");
        return;
    }
}

void ServerNetworkManager::start()
{
    m_localhost = new STKHost();
    m_localhost->setupServer(STKHost::HOST_ANY, 7000, 32, 2, 0, 0);
    m_localhost->startListening();
    printf("Server now setup, listening on port 7000.\n");
}

void ServerNetworkManager::packetReceived(char* data)
{
    printf("ServerNetworkManager::packetReceived()\n");
    puts(data);
    sendPacket(data);
}
void ServerNetworkManager::sendPacket(char* data)
{
    m_localhost->broadcastPacket(data);
}
