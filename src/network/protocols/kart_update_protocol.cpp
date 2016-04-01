#include "network/protocols/kart_update_protocol.hpp"

#include "karts/abstract_kart.hpp"
#include "karts/controller/controller.hpp"
#include "modes/world.hpp"
#include "network/event.hpp"
#include "network/network_config.hpp"
#include "network/protocol_manager.hpp"
#include "utils/time.hpp"

KartUpdateProtocol::KartUpdateProtocol() : Protocol(PROTOCOL_KART_UPDATE)
{
}   // KartUpdateProtocol

// ----------------------------------------------------------------------------
KartUpdateProtocol::~KartUpdateProtocol()
{
}   // ~KartUpdateProtocol

// ----------------------------------------------------------------------------
void KartUpdateProtocol::setup()
{
    // Allocate arrays to store one position and rotation for each kart
    // (which is the update information from the server to the client).
    m_next_positions.resize(World::getWorld()->getNumKarts());
    m_next_quaternions.resize(World::getWorld()->getNumKarts());

    // This flag keeps track if valid data for an update is in
    // the arrays
    m_was_updated = false;

    m_previous_time = 0;
}   // setup

// ----------------------------------------------------------------------------
/** Store the update events in the queue. Since the events are handled in the
 *  synchronous notify function, there is no lock necessary to 
 */
bool KartUpdateProtocol::notifyEvent(Event* event)
{
    // It might be possible that we still receive messages after
    // the game was exited, so make sure we still have a world.
    if (event->getType() != EVENT_TYPE_MESSAGE || !World::getWorld())
        return true;
    NetworkString &ns = event->data();
    if (ns.size() < 33)
    {
        Log::info("KartUpdateProtocol", "Message too short.");
        return true;
    }
    float time = ns.getFloat();
    while(ns.size() >= 29)
    {
        uint8_t kart_id             = ns.getUInt8();
        Vec3 xyz                    = ns.getVec3();
        btQuaternion quat           = ns.getQuat();
        m_next_positions  [kart_id] = xyz;
        m_next_quaternions[kart_id] = quat;
    }   // while ns.size()>29

    // Set the flag that a new update was received
    m_was_updated = true;
    return true;
}   // notifyEvent

// ----------------------------------------------------------------------------
/** Sends regular update events from the server to all clients and from the
 *  clients to the server (FIXME - is that actually necessary??)
 *  Then it applies all update events that have been received in notifyEvent.
 *  This two-part implementation means that if the server should send two
 *  or more updates before this client handles them, only the last one will
 *  actually be handled (i.e. outdated kart position updates are discarded).
 */
void KartUpdateProtocol::update(float dt)
{
    if (!World::getWorld())
        return;

    double current_time = StkTime::getRealTime();
    if (current_time > m_previous_time + 0.1) // 10 updates per second
    {
        m_previous_time = current_time;
        if (NetworkConfig::get()->isServer())
        {
            World *world = World::getWorld();
            NetworkString *ns = getNetworkString(4+world->getNumKarts()*29);
            ns->setSynchronous(true);
            ns->addFloat( world->getTime() );
            for (unsigned int i = 0; i < world->getNumKarts(); i++)
            {
                AbstractKart* kart = world->getKart(i);
                Vec3 xyz = kart->getXYZ();
                ns->addUInt8( kart->getWorldKartId());
                ns->add(xyz).add(kart->getRotation());
                Log::verbose("KartUpdateProtocol",
                             "Sending %d's positions %f %f %f",
                             kart->getWorldKartId(), xyz[0], xyz[1], xyz[2]);
            }
            sendMessageToPeersChangingToken(ns, /*reliable*/false);
            delete ns;
        }
        else
        {
            NetworkString *ns =
                     getNetworkString(4+29*race_manager->getNumLocalPlayers());
            ns->setSynchronous(true);
            ns->addFloat(World::getWorld()->getTime());
            for(unsigned int i=0; i<race_manager->getNumLocalPlayers(); i++)
            {
                AbstractKart *kart = World::getWorld()->getLocalPlayerKart(i);
                const Vec3 &xyz = kart->getXYZ();
                ns->addUInt8(kart->getWorldKartId());
                ns->add(xyz).add(kart->getRotation());
                Log::verbose("KartUpdateProtocol",
                             "Sending %d's positions %f %f %f",
                              kart->getWorldKartId(), xyz[0], xyz[1], xyz[2]);
            }
            sendToServer(ns, /*reliable*/false);
            delete ns;
        }   // if server
    }   // if (current_time > time + 0.1)


    // Now handle all update events that have been received.
    // There is no lock necessary, since receiving new positions is done in
    // notifyEvent, which is called from the same thread that calls this
    // function.
    if(m_was_updated)
    {
        for (unsigned id = 0; id < m_next_positions.size(); id++)
        {
            AbstractKart *kart = World::getWorld()->getKart(id);
            if (!kart->getController()->isLocalPlayerController())
            {
                btTransform transform = kart->getBody()
                                      ->getInterpolationWorldTransform();
                transform.setOrigin(m_next_positions[id]);
                transform.setRotation(m_next_quaternions[id]);
                kart->getBody()->setCenterOfMassTransform(transform);
                Log::verbose("KartUpdateProtocol", "Update kart %i pos",
                             id);
            }   // if not local player
        }   // for id < num_karts
        m_was_updated = false;  // mark that all updates were applied
    }   // if m_was_updated
}   // update

