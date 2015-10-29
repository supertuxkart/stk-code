#include "network/protocols/kart_update_protocol.hpp"

#include "karts/abstract_kart.hpp"
#include "modes/world.hpp"
#include "network/event.hpp"
#include "network/network_world.hpp"
#include "network/protocol_manager.hpp"
#include "network/stk_host.hpp"
#include "utils/time.hpp"

KartUpdateProtocol::KartUpdateProtocol() : Protocol(PROTOCOL_KART_UPDATE)
{
    m_karts = World::getWorld()->getKarts();
    for (unsigned int i = 0; i < m_karts.size(); i++)
    {
        //if (m_karts[i]->getWorldKartId())
        {
            Log::info("KartUpdateProtocol", "Kart %d has id %d and name %s", i, m_karts[i]->getWorldKartId(), m_karts[i]->getIdent().c_str());
        }
        if (m_karts[i]->getIdent() == NetworkWorld::getInstance()->m_self_kart)
        {
            Log::info("KartUpdateProtocol", "My id is %d", i);
            m_self_kart_index = i;
        }
    }
    pthread_mutex_init(&m_positions_updates_mutex, NULL);
}

KartUpdateProtocol::~KartUpdateProtocol()
{
}

bool KartUpdateProtocol::notifyEventAsynchronous(Event* event)
{
    if (event->getType() != EVENT_TYPE_MESSAGE)
        return true;
    NetworkString &ns = event->data();
    if (ns.size() < 36)
    {
        Log::info("KartUpdateProtocol", "Message too short.");
        return true;
    }
    ns.removeFront(4);
    while(ns.size() >= 16)
    {
        uint32_t kart_id = ns.getUInt32(0);

        float a,b,c;
        a = ns.getFloat(4);
        b = ns.getFloat(8);
        c = ns.getFloat(12);
        float d,e,f,g;
        d = ns.getFloat(16);
        e = ns.getFloat(20);
        f = ns.getFloat(24);
        g = ns.getFloat(28);
        pthread_mutex_trylock(&m_positions_updates_mutex);
        m_next_positions.push_back(Vec3(a,b,c));
        m_next_quaternions.push_back(btQuaternion(d,e,f,g));
        m_karts_ids.push_back(kart_id);
        pthread_mutex_unlock(&m_positions_updates_mutex);
        ns.removeFront(32);
    }
    return true;
}

void KartUpdateProtocol::setup()
{
}

void KartUpdateProtocol::update()
{
    if (!World::getWorld())
        return;
    static double time = 0;
    double current_time = StkTime::getRealTime();
    if (current_time > time + 0.1) // 10 updates per second
    {
        time = current_time;
        if (STKHost::isServer())
        {
            NetworkString ns(4+m_karts.size()*32);
            ns.af( World::getWorld()->getTime());
            for (unsigned int i = 0; i < m_karts.size(); i++)
            {
                AbstractKart* kart = m_karts[i];
                Vec3 v = kart->getXYZ();
                btQuaternion quat = kart->getRotation();
                ns.ai32( kart->getWorldKartId());
                ns.af(v[0]).af(v[1]).af(v[2]); // add position
                ns.af(quat.x()).af(quat.y()).af(quat.z()).af(quat.w()); // add rotation
                Log::verbose("KartUpdateProtocol",
                             "Sending %d's positions %f %f %f",
                             kart->getWorldKartId(), v[0], v[1], v[2]);
            }
            sendMessage(ns, false);
        }
        else
        {
            AbstractKart* kart = m_karts[m_self_kart_index];
            Vec3 v = kart->getXYZ();
            btQuaternion quat = kart->getRotation();
            NetworkString ns(36);
            ns.af( World::getWorld()->getTime());
            ns.ai32( kart->getWorldKartId());
            ns.af(v[0]).af(v[1]).af(v[2]); // add position
            ns.af(quat.x()).af(quat.y()).af(quat.z()).af(quat.w()); // add rotation
            Log::verbose("KartUpdateProtocol",
                         "Sending %d's positions %f %f %f",
                         kart->getWorldKartId(), v[0], v[1], v[2]);
            sendMessage(ns, false);
        }
    }
    switch(pthread_mutex_trylock(&m_positions_updates_mutex))
    {
        case 0: /* if we got the lock */
            while (!m_next_positions.empty())
            {
                uint32_t id = m_karts_ids.back();
                // server takes all updates
                if (id != m_self_kart_index || STKHost::isServer())
                {
                    Vec3 pos = m_next_positions.back();
                    btTransform transform = m_karts[id]->getBody()->getInterpolationWorldTransform();
                    transform.setOrigin(pos);
                    transform.setRotation(m_next_quaternions.back());
                    m_karts[id]->getBody()->setCenterOfMassTransform(transform);
                    //m_karts[id]->getBody()->setLinearVelocity(Vec3(0,0,0));
                    Log::verbose("KartUpdateProtocol", "Update kart %i pos to %f %f %f", id, pos[0], pos[1], pos[2]);
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
}




