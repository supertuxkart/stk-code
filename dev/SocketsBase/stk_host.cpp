#include "stk_host.hpp"

#include "network_manager.hpp"

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>

void* STKHost::receive_data(void* self)
{
    ENetEvent event;
    ENetHost* host = (((STKHost*)(self))->m_host);
    while (1) 
    {
        while (enet_host_service(host, &event, 0) != 0) {
            printf("message received\n");
            switch (event.type) {
                case ENET_EVENT_TYPE_RECEIVE:
                    NetworkManager::receptionCallback((char*) event.packet->data);
                    break;
                case ENET_EVENT_TYPE_DISCONNECT:
                    printf("Somebody is now disconnected.\n");
                    printf("Disconnected host: %i.%i.%i.%i:%i\n", event.peer->address.host>>24&0xff, event.peer->address.host>>16&0xff, event.peer->address.host>>8&0xff, event.peer->address.host&0xff,event.peer->address.port);
                    break;
                case ENET_EVENT_TYPE_CONNECT:
                    printf("A client has just connected.\n");
                    break;
                case ENET_EVENT_TYPE_NONE:
                    break;
            }
        }
    }
    return NULL;
}

STKHost::STKHost()
{
    m_host = NULL;
    m_listeningThread = (pthread_t*)(malloc(sizeof(pthread_t)));
}

STKHost::~STKHost()
{
}

void STKHost::setupServer(uint32_t address, uint16_t port, int peerCount, int channelLimit, uint32_t maxIncomingBandwidth, uint32_t maxOutgoingBandwidth)
{
    ENetAddress addr;
    addr.host = address;
    addr.port = port;

    m_host = enet_host_create(&addr, peerCount, channelLimit, maxIncomingBandwidth, maxOutgoingBandwidth);
    if (m_host == NULL)
    {
        fprintf (stderr, "An error occurred while trying to create an ENet server host.\n");
        exit (EXIT_FAILURE);
    }
}

void STKHost::setupClient(int peerCount, int channelLimit, uint32_t maxIncomingBandwidth, uint32_t maxOutgoingBandwidth)
{
    m_host = enet_host_create(NULL, peerCount, channelLimit, maxIncomingBandwidth, maxOutgoingBandwidth);
    if (m_host == NULL)
    {
        fprintf (stderr, "An error occurred while trying to create an ENet client host.\n");
        exit (EXIT_FAILURE);
    }
}

void STKHost::startListening()
{
    pthread_create(m_listeningThread, NULL, &STKHost::receive_data, this);
}
void STKHost::stopListening()
{
    pthread_cancel(*m_listeningThread);
}

void STKHost::sendRawPacket(uint8_t* data, int length, unsigned int dstIp, unsigned short dstPort)
{
    struct sockaddr_in to;
    int toLen = sizeof(to);
    memset(&to,0,toLen);

    to.sin_family = AF_INET;
    to.sin_port = htons(dstPort);
    to.sin_addr.s_addr = htonl(dstIp);
    
    sendto(m_host->socket, data, length, 0,(sockaddr*)&to, toLen);
}

uint8_t* STKHost::receiveRawPacket() 
{
    uint8_t* buffer; // max size needed normally (only used for stun)
    buffer = (uint8_t*)(malloc(sizeof(uint8_t)*2048));
    memset(buffer, 0, 2048);
    
    int len = recv(m_host->socket,buffer,2048, 0);
    int i = 0;
    while(len < 0) // wait to receive the message because enet sockets are non-blocking
    {
        i++;
        len = recv(m_host->socket,buffer,2048, 0);
        usleep(1000); // wait 1 millisecond between two checks
    }
    printf("Packet received after %i milliseconds\n", i);
    return buffer;
}
uint8_t* STKHost::receiveRawPacket(unsigned int dstIp, unsigned short dstPort) 
{
    uint8_t* buffer; // max size needed normally (only used for stun)
    buffer = (uint8_t*)(malloc(sizeof(uint8_t)*2048));
    memset(buffer, 0, 2048);
    
    socklen_t fromlen;
    struct sockaddr addr;
    
    fromlen = sizeof(addr);
    int len = recvfrom(m_host->socket, buffer, 2048, 0, &addr, &fromlen);
    
    int i = 0;
    while(len < 0 || (
               (uint8_t)(addr.sa_data[2]) != (dstIp>>24&0xff) 
            && (uint8_t)(addr.sa_data[3]) != (dstIp>>16&0xff) 
            && (uint8_t)(addr.sa_data[4]) != (dstIp>>8&0xff) 
            && (uint8_t)(addr.sa_data[5]) != (dstIp&0xff))) // wait to receive the message because enet sockets are non-blocking
    {
        i++;
        len = recvfrom(m_host->socket, buffer, 2048, 0, &addr, &fromlen);
        usleep(1000); // wait 1 millisecond between two checks
    }
    if (addr.sa_family == AF_INET)
    {
        char s[20];
        inet_ntop(AF_INET, &(((struct sockaddr_in *)&addr)->sin_addr), s, 20);
        printf("IPv4 Address %s\n", s);
    }
    printf("Packet received after %i milliseconds\n", i);
    return buffer;
}

void STKHost::broadcastPacket(char* data)
{
    ENetPacket* packet = enet_packet_create(data, strlen(data)+1,ENET_PACKET_FLAG_RELIABLE);
    enet_host_broadcast(m_host, 0, packet);
}
