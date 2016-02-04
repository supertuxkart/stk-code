#ifndef KART_UPDATE_PROTOCOL_HPP
#define KART_UPDATE_PROTOCOL_HPP

#include "network/protocol.hpp"
#include "utils/vec3.hpp"
#include "LinearMath/btQuaternion.h"

#include <vector>
#include "pthread.h"

class AbstractKart;

class KartUpdateProtocol : public Protocol
{
private:

    /** Stores the last updated position for a kart. */
    std::vector<Vec3> m_next_positions;

    /** Stores the last updated rotation for a kart. */
    std::vector<btQuaternion> m_next_quaternions;

    /** True if a new update for the kart positions was received. */
    bool m_was_updated;

public:
             KartUpdateProtocol();
    virtual ~KartUpdateProtocol();

    virtual bool notifyEvent(Event* event);
    virtual void setup();
    virtual void update();
    virtual void asynchronousUpdate() {};

};   // KartUpdateProtocol

#endif // KART_UPDATE_PROTOCOL_HPP
