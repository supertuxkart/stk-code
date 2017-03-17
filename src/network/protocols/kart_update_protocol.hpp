#ifndef KART_UPDATE_PROTOCOL_HPP
#define KART_UPDATE_PROTOCOL_HPP

#include "network/protocol.hpp"
#include "utils/cpp2011.hpp"
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

    /** Time the last kart update was sent. Used to send updates with
     * a fixed frequency. */
    double m_previous_time;

public:
             KartUpdateProtocol();
    virtual ~KartUpdateProtocol();

    virtual bool notifyEvent(Event* event) OVERRIDE;
    virtual void setup() OVERRIDE;
    virtual void update(float dt) OVERRIDE;
    virtual void asynchronousUpdate() OVERRIDE {};

};   // KartUpdateProtocol

#endif // KART_UPDATE_PROTOCOL_HPP
