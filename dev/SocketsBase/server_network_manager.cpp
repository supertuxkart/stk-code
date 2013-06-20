#include "server_network_manager.hpp"

#include "protocols/get_public_address.hpp"
#include "protocols/hide_public_address.hpp"
#include "protocols/show_public_address.hpp"
#include "protocols/get_peer_address.hpp"
#include "protocols/connect_to_server.hpp"

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
    NetworkManager::run();
}

void ServerNetworkManager::start()
{
    m_localhost = new STKHost();
    m_localhost->setupServer(STKHost::HOST_ANY, 7321, 32, 2, 0, 0);
    m_localhost->startListening();
    printf("Server now setup, listening on port 7321.\n");
    
    printf("_NetworkInterface>Starting the global protocol\n");
    // step 1 : retreive public address
    int id = ProtocolManager::getInstance()->startProtocol(new GetPublicAddress(&m_publicAddress));
    while (ProtocolManager::getInstance()->getProtocolState(id) != PROTOCOL_STATE_TERMINATED )
    {
    }
    printf("_NetworkInterface> The public address is known.\n"); 
    
    // step 2 : show the public address for others (here, the server)
    ShowPublicAddress* spa = new ShowPublicAddress(NULL);
    spa->setPassword(m_playerLogin.password);
    spa->setUsername(m_playerLogin.username);
    spa->setPublicAddress(m_publicAddress.ip, m_publicAddress.port);
    id = ProtocolManager::getInstance()->startProtocol(spa);
    while (ProtocolManager::getInstance()->getProtocolState(id) != PROTOCOL_STATE_TERMINATED )
    {
    }
    printf("_NetworkInterface> The public address is being shown online.\n"); 
}

bool ServerNetworkManager::connectToPeer(std::string peerUsername)
{
    printf("_NetworkInterface>Starting the connection to host protocol\n");
    
    // step 3 : get the peer's addres.
    TransportAddress addr;
    GetPeerAddress* gpa = new GetPeerAddress(&addr);
    gpa->setPeerName(peerUsername);
    uint32_t id = ProtocolManager::getInstance()->startProtocol(gpa);
    while (ProtocolManager::getInstance()->getProtocolState(id) != PROTOCOL_STATE_TERMINATED )
    {
    }
    printf("_NetworkInterface> The public address of the peer is known.\n"); 
    
    // step 2 : connect to the peer
    ConnectToServer* cts = new ConnectToServer(NULL);
    cts->setServerAddress(addr.ip, addr.port);
    id = ProtocolManager::getInstance()->startProtocol(cts);
    while (ProtocolManager::getInstance()->getProtocolState(id) != PROTOCOL_STATE_TERMINATED )
    {
    } 
    bool success = false;
    if (isConnectedTo(addr.ip, addr.port))
    {
        success = true;
        printf("_NetworkInterface> CONNECTION SUCCES : YOU ARE NOW CONNECTED TO A PEER.\n");
    }
    else 
    {
        printf("_NetworkInterface> We are NOT connected to the server.\n");
    }
    
    return success;
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
