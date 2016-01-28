#include "network/protocols/kart_update_protocol.hpp"

#include "karts/abstract_kart.hpp"
#include "karts/controller/controller.hpp"
#include "modes/world.hpp"
#include "network/event.hpp"
#include "network/network_config.hpp"
#include "network/network_world.hpp"
#include "network/protocol_manager.hpp"
#include "utils/time.hpp"

KartUpdateProtocol::KartUpdateProtocol() : Protocol(PROTOCOL_KART_UPDATE)
{
    World *world = World::getWorld();
    for (unsigned int i = 0; i < world->getNumKarts(); i++)
    {
        Log::info("KartUpdateProtocol", "Kart %d has id %d and name %s",
                  i, world->getKart(i)->getWorldKartId(),
                  world->getKart(i)->getIdent().c_str());
    }
    pthread_mutex_init(&m_positions_updates_mutex, NULL);
}   // KartUpdateProtocol

// ----------------------------------------------------------------------------
KartUpdateProtocol::~KartUpdateProtocol()
{
}   // ~KartUpdateProtocol

// ----------------------------------------------------------------------------
bool KartUpdateProtocol::notifyEventAsynchronous(Event* event)
{
    if (event->getType() != EVENT_TYPE_MESSAGE)
        return true;
    NetworkString &ns = event->data();
    if (ns.size() < 29)
    {
        Log::info("KartUpdateProtocol", "Message too short.");
        return true;
    }
    ns.removeFront(4);
    while(ns.size() >= 29)
    {
        uint8_t kart_id = ns.getUInt8(0);

        float a,b,c;
        a = ns.getFloat(1);
        b = ns.getFloat(5);
        c = ns.getFloat(9);
        float d,e,f,g;
        d = ns.getFloat(13);
        e = ns.getFloat(17);
        f = ns.getFloat(21);
        g = ns.getFloat(25);
        pthread_mutex_trylock(&m_positions_updates_mutex);
        m_next_positions.push_back(Vec3(a,b,c));
        m_next_quaternions.push_back(btQuaternion(d,e,f,g));
        m_karts_ids.push_back(kart_id);
        pthread_mutex_unlock(&m_positions_updates_mutex);
        ns.removeFront(29);
    }   // while ns.size()>29
    return true;
}   // notifyEventAsynchronous

// ----------------------------------------------------------------------------
void KartUpdateProtocol::setup()
{
}   // setup

// ----------------------------------------------------------------------------
void KartUpdateProtocol::update()
{
    if (!World::getWorld())
        return;
    static double time = 0;
    double current_time = StkTime::getRealTime();
    if (current_time > time + 0.1) // 10 updates per second
    {
        time = current_time;
        if (NetworkConfig::get()->isServer())
        {
            World *world = World::getWorld();
            NetworkString ns(4+world->getNumKarts()*29);
            ns.af( world->getTime() );
            for (unsigned int i = 0; i < world->getNumKarts(); i++)
            {
                AbstractKart* kart = world->getKart(i);
                Vec3 xyz = kart->getXYZ();
                btQuaternion quat = kart->getRotation();
                ns.addUInt8( kart->getWorldKartId());
                ns.af(xyz[0]).af(xyz[1]).af(xyz[2]); // add position
                ns.af(quat.x()).af(quat.y()).af(quat.z()).af(quat.w()); // add rotation
                Log::verbose("KartUpdateProtocol",
                             "Sending %d's positions %f %f %f",
                             kart->getWorldKartId(), xyz[0], xyz[1], xyz[2]);
            }
            sendMessage(ns, false);
        }
        else
        {
            NetworkString ns(4+29*race_manager->getNumLocalPlayers());
            ns.af(World::getWorld()->getTime());
            for(unsigned int i=0; i<race_manager->getNumLocalPlayers(); i++)
            {
                AbstractKart *kart = World::getWorld()->getLocalPlayerKart(i);
                const Vec3 &xyz = kart->getXYZ();
                btQuaternion quat = kart->getRotation();
                ns.addUInt8(kart->getWorldKartId());
                ns.af(xyz[0]).af(xyz[1]).af(xyz[2]); // add position
                ns.af(quat.x()).af(quat.y()).af(quat.z()).af(quat.w()); // add rotation
                Log::verbose("KartUpdateProtocol",
                             "Sending %d's positions %f %f %f",
                              kart->getWorldKartId(), xyz[0], xyz[1], xyz[2]);
            }
            sendMessage(ns, false);
        }   // if server
    }   // if (current_time > time + 0.1)

    switch(pthread_mutex_trylock(&m_positions_updates_mutex))
    {
        case 0: /* if we got the lock */
            while (!m_next_positions.empty())
            {
                uint32_t id = m_karts_ids.back();
                AbstractKart *kart = World::getWorld()->getKart(id);
                if(!kart->getController()->isLocalPlayerController())
                {
                    Vec3 pos = m_next_positions.back();
                    btTransform transform = kart->getBody()
                                          ->getInterpolationWorldTransform();
                    transform.setOrigin(pos);
                    transform.setRotation(m_next_quaternions.back());
                    kart->getBody()->setCenterOfMassTransform(transform);
                    Log::verbose("KartUpdateProtocol",
                                 "Update kart %i pos to %f %f %f",
                                 id, pos[0], pos[1], pos[2]);
                }
                else
                {
                    // We should use smoothing here!
                }
                m_next_positions.pop_back();
                m_next_quaternions.pop_back();
                m_karts_ids.pop_back();
            }
            pthread_mutex_unlock(&m_positions_updates_mutex);
            break;
        default:
            break;
    }
}   // update

