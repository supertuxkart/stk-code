#include "client_network_manager.hpp"

#include "protocols/get_public_address.hpp"
#include "protocols/hide_public_address.hpp"
#include "protocols/show_public_address.hpp"
#include "protocols/get_peer_address.hpp"
#include "protocols/connect_to_server.hpp"

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
    m_localhost = new STKHost();
    m_localhost->setupClient(1, 2, 0, 0);
    m_localhost->startListening();
    
    NetworkManager::run();
}

bool ClientNetworkManager::connectToHost(std::string serverNickname)
{
    printf("_NetworkInterface>Starting the connection to host protocol\n");
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
    
    // step 3 : get the server's addres.
    TransportAddress addr;
    GetPeerAddress* gpa = new GetPeerAddress(&addr);
    gpa->setPeerName(serverNickname);
    id = ProtocolManager::getInstance()->startProtocol(gpa);
    while (ProtocolManager::getInstance()->getProtocolState(id) != PROTOCOL_STATE_TERMINATED )
    {
    }
    printf("_NetworkInterface> The public address of the server is known.\n"); 
    
    // step 4 : connect to the server
    ConnectToServer* cts = new ConnectToServer(NULL);
    cts->setServerAddress(addr.ip, addr.port);
    id = ProtocolManager::getInstance()->startProtocol(cts);
    while (ProtocolManager::getInstance()->getProtocolState(id) != PROTOCOL_STATE_TERMINATED )
    {
    } 
    bool success = false;
    if (m_peers[0]->isConnected())
    {
       success = true;
        printf("_NetworkInterface> CONNECTION SUCCES : YOU ARE NOW CONNECTED TO A SERVER.\n");
    }
    else 
    {
        printf("_NetworkInterface> We are NOT connected to the server.\n");
    }
    
    // step 5 : hide our public address
    HidePublicAddress* hpa = new HidePublicAddress(NULL);
    hpa->setPassword(m_playerLogin.password);
    hpa->setUsername(m_playerLogin.username);
    id = ProtocolManager::getInstance()->startProtocol(hpa);
    while (ProtocolManager::getInstance()->getProtocolState(id) != PROTOCOL_STATE_TERMINATED )
    {
    }
    printf("_NetworkInterface> The public address is now hidden online.\n"); 
    return success;
}

void ClientNetworkManager::packetReceived(char* data)
{
    printf("ClientNetworkManager::packetReceived()\n");
    puts(data);
}
void ClientNetworkManager::sendPacket(char* data)
{
    if (m_peers.size() > 1)
        printf("Ambiguous send of data\n");
    m_peers[0]->sendPacket(data);
}

STKPeer* ClientNetworkManager::getPeer()
{
    return m_peers[0];
}
