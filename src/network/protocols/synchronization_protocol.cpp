#include "network/protocols/synchronization_protocol.hpp"

#include "network/network_manager.hpp"
#include "utils/time.hpp"

//-----------------------------------------------------------------------------

SynchronizationProtocol::SynchronizationProtocol(uint32_t* ping, bool* successed) : Protocol(NULL, PROTOCOL_SYNCHRONIZATION)
{
    m_average_ping = ping;
    m_successed = successed;
    m_pings.resize(NetworkManager::getInstance()->getPeerCount(), std::vector<std::pair<double,double> >(0));
    m_pings_count = 0;
    m_successed_pings = 0;
    m_total_diff = 0;
}

//-----------------------------------------------------------------------------

SynchronizationProtocol::~SynchronizationProtocol()
{
}

//-----------------------------------------------------------------------------

void SynchronizationProtocol::notifyEvent(Event* event)
{
    if (event->type != EVENT_TYPE_MESSAGE)
        return;
    if (event->data.size() < 10)
    {
        Log::warn("SynchronizationProtocol", "Received a message too short.");
        return;
    }
    uint8_t talk_id = event->data.gui8(0);
    uint32_t token = event->data.gui32(1);
    uint32_t request = event->data.gui8(5);
    uint32_t sequence = event->data.gui32(6);

    std::vector<STKPeer*> peers = NetworkManager::getInstance()->getPeers();

    if (m_listener->isServer())
    {
        if (talk_id > peers.size())
        {
            Log::warn("SynchronizationProtocol", "The ID isn't known.");
            return;
        }
    }

    uint8_t peer_id = (m_listener->isServer() ? talk_id : 0); // on clients, peer index is 0
    if (peers[peer_id]->getClientServerToken() != token)
    {
        Log::warn("SynchronizationProtocol", "Bad token from peer %d", talk_id);
        return;
    }

    if (request)
    {
        NetworkString response;
        response.ai8(event->data.gui8(talk_id)).ai32(token).ai8(0).ai32(sequence);
        m_listener->sendMessage(this, peers[talk_id], response, false);
        Log::info("SynchronizationProtocol", "Answering sequence %u", sequence);
    }
    else // response
    {
        if (sequence >= m_pings[peer_id].size())
        {
            Log::warn("SynchronizationProtocol", "The sequence# %u isn't known.", sequence);
            return;
        }
        m_pings[peer_id][sequence].second = Time::getRealTime();
        m_total_diff += (m_pings[peer_id][sequence].second - m_pings[peer_id][sequence].first);
        Log::verbose("SynchronizationProtocol", "InstantPing is %u", (unsigned int)((m_pings[peer_id][sequence].second - m_pings[peer_id][sequence].first)*1000));
        m_successed_pings++;
        *m_average_ping = (int)((m_total_diff/m_successed_pings)*1000.0);
        if ( *m_successed == false && m_successed_pings > 5)
        {
            *m_successed = true; // success after 5 pings (we have good idea of ping)
        }
        Log::verbose("SynchronizationProtocol", "Ping is %u", *m_average_ping);
    }
}

//-----------------------------------------------------------------------------

void SynchronizationProtocol::setup()
{
}

//-----------------------------------------------------------------------------

void SynchronizationProtocol::update()
{
    static double timer = Time::getRealTime();
    if (Time::getRealTime() > timer+0.1 && m_pings_count < 100) // max 100 pings (10 seconds)
    {
        std::vector<STKPeer*> peers = NetworkManager::getInstance()->getPeers();
        for (unsigned int i = 0; i < peers.size(); i++)
        {
            NetworkString ns;
            ns.ai8(i).addUInt32(peers[i]->getClientServerToken()).addUInt8(1).addUInt32(m_pings[i].size());
            Log::verbose("SynchronizationProtocol", "Added sequence number %u", m_pings[i].size());
            timer = Time::getRealTime();
            m_pings[i].push_back(std::pair<double, double>(timer, 0.0));
            m_listener->sendMessage(this, peers[i], ns, false);
        }
        m_pings_count++;
    }
}

//-----------------------------------------------------------------------------
