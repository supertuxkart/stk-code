#ifndef KART_UPDATE_PROTOCOL_HPP
#define KART_UPDATE_PROTOCOL_HPP

#include "network/protocol.hpp"

class AbstractKart;

class KartUpdateProtocol : public Protocol
{
    public:
        KartUpdateProtocol();
        virtual ~KartUpdateProtocol();

        virtual void notifyEvent(Event* event);
        virtual void setup();
        virtual void update();
        virtual void asynchronousUpdate() {};

    protected:
        std::vector<AbstractKart*> m_karts;
        uint32_t m_self_kart_index;

};

#endif // KART_UPDATE_PROTOCOL_HPP
