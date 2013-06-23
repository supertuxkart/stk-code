#include "event.hpp"

#include "network_manager.hpp"

#include <vector>
#include <stdio.h>

Event::Event(ENetEvent* event)
{
    switch (event->type)
    {
    case ENET_EVENT_TYPE_CONNECT:
        type = EVENT_TYPE_CONNECTED;
        break;
    case ENET_EVENT_TYPE_DISCONNECT:
        type = EVENT_TYPE_DISCONNECTED;
        break;
    case ENET_EVENT_TYPE_RECEIVE:
        type = EVENT_TYPE_MESSAGE;
        break;
    default:
        break;
    }
    if (type == EVENT_TYPE_MESSAGE)
        data = std::string((char*)(event->packet->data));
    else if (event->data)
        data = std::string((char*)(event->data));
    
    if (event->packet)
        m_packet = event->packet;

    std::vector<STKPeer*> peers = NetworkManager::getInstance()->getPeers();
    peer = NULL;
    for (unsigned int i = 0; i < peers.size(); i++)
    {
        if (*peers[i] == event->peer)
        {
            peer = peers[i];
            return;
        }
    }
    if (peer == NULL)
    {
        printf("The peer still does not exist in %u peers\n", peers.size());
        STKPeer* newPeer = new STKPeer();
        newPeer->m_peer = event->peer;
        peer = newPeer;
    }
}
    
Event::~Event()
{
    if (m_packet)
        enet_packet_destroy(m_packet);
}
