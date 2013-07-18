#include "network/protocols/kart_update_protocol.hpp"

#include "karts/abstract_kart.hpp"
#include "modes/world.hpp"
#include "network/protocol_manager.hpp"
#include "network/network_world.hpp"

KartUpdateProtocol::KartUpdateProtocol()
    : Protocol(NULL, PROTOCOL_KART_UPDATE)
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

void KartUpdateProtocol::notifyEvent(Event* event)
{
    if (event->type != EVENT_TYPE_MESSAGE)
        return;
    NetworkString ns = event->data;
    if (ns.size() < 20)
    {
        Log::info("KartUpdateProtocol", "Message too short.");
        return;
    }
    float game_time = ns.getFloat(0);
    ns.removeFront(4);
    int nb_updates = 0;
    while(ns.size() >= 16)
    {
        uint32_t kart_id = ns.getUInt32(0);

        float a,b,c;
        a = ns.getFloat(4);
        b = ns.getFloat(8);
        c = ns.getFloat(12);
        pthread_mutex_trylock(&m_positions_updates_mutex);
        m_next_positions.push_back(Vec3(a,b,c));
        m_karts_ids.push_back(kart_id);
        pthread_mutex_unlock(&m_positions_updates_mutex);
        Log::info("KartUpdateProtocol", "Update#%i kart %i pos to %f %f %f", nb_updates, kart_id, a,b,c);
        nb_updates ++;
        ns.removeFront(16);
    }
}

void KartUpdateProtocol::setup()
{
}

void KartUpdateProtocol::update()
{
    static double time = 0;
    double current_time = Time::getRealTime();
    if (current_time > time + 1.0) // 1 updates per second
    {
        time = current_time;
        if (m_listener->isServer())
        {
            NetworkString ns;
            ns.af( World::getWorld()->getTime());
            for (unsigned int i = 0; i < m_karts.size(); i++)
            {
                AbstractKart* kart = m_karts[i];
                Vec3 v = kart->getXYZ();
                ns.ai32( kart->getWorldKartId());
                ns.af(v[0]).af(v[1]).af(v[2]);
                Log::info("KartUpdateProtocol", "Sending %d's positions %f %f %f", kart->getWorldKartId(), v[0], v[1], v[2]);
            }
            m_listener->sendMessage(this, ns, false);
        }
        else
        {
            AbstractKart* kart = m_karts[m_self_kart_index];
            Vec3 v = kart->getXYZ();
            NetworkString ns;
            ns.af( World::getWorld()->getTime());
            ns.ai32( kart->getWorldKartId());
            ns.af(v[0]).af(v[1]).af(v[2]);
            Log::info("KartUpdateProtocol", "Sending %d's positions %f %f %f", kart->getWorldKartId(), v[0], v[1], v[2]);
            Log::info("KartUpdateProtocol", "Sending %d's positions %f %f %f", ns.gui32(4), v[0], v[1], v[2]);
            m_listener->sendMessage(this, ns, false);
        }
    }
    switch(pthread_mutex_trylock(&m_positions_updates_mutex))
    {
        case 0: /* if we got the lock, unlock and return 1 (true) */
            while (!m_next_positions.empty())
            {
                uint32_t id = m_karts_ids.back();
                Vec3 pos = m_next_positions.back();
                m_karts[id]->setXYZ(pos);
                m_next_positions.pop_back();
                m_karts_ids.pop_back();
            }
            pthread_mutex_unlock(&m_positions_updates_mutex);
            break;
        default:
            break;
    }
}




