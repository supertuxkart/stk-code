#ifndef EVENT_HPP
#define EVENT_HPP

#include "stk_peer.hpp"
#include <stdint.h>
#include <string>

enum EVENT_TYPE
{
    EVENT_TYPE_CONNECTED,
    EVENT_TYPE_DISCONNECTED,
    EVENT_TYPE_MESSAGE
};

class Event
{
    public:
        Event(ENetEvent* event);
        ~Event();
    
        EVENT_TYPE type;
        std::string data;
        STKPeer* peer;
    
    private:
        ENetPacket* m_packet;
};

#endif // EVENT_HPP
