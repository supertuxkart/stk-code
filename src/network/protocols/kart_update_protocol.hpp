#ifndef KART_UPDATE_PROTOCOL_HPP
#define KART_UPDATE_PROTOCOL_HPP

#include "network/protocol.hpp"
#include "utils/vec3.hpp"
#include "LinearMath/btQuaternion.h"

#include <list>
#include "pthread.h"

class AbstractKart;

class KartUpdateProtocol : public Protocol
{
    public:
        KartUpdateProtocol();
        virtual ~KartUpdateProtocol();

        virtual bool notifyEventAsynchronous(Event* event);
        virtual void setup();
        virtual void update();
        virtual void asynchronousUpdate() {};

    protected:
        std::vector<AbstractKart*> m_karts;
        uint32_t m_self_kart_index;

        std::list<Vec3> m_next_positions;
        std::list<btQuaternion> m_next_quaternions;
        std::list<uint32_t> m_karts_ids;

        pthread_mutex_t m_positions_updates_mutex;
};

#endif // KART_UPDATE_PROTOCOL_HPP
