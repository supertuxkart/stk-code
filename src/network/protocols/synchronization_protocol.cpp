#include "network/protocols/synchronization_protocol.hpp"

#include "network/network_manager.hpp"
#include "utils/time.hpp"

//-----------------------------------------------------------------------------

SynchronizationProtocol::SynchronizationProtocol() : Protocol(NULL, PROTOCOL_SYNCHRONIZATION)
{
    unsigned int size = NetworkManager::getInstance()->getPeerCount();
    m_pings.resize(size, std::map<uint32_t,double>());
    m_pings_count.resize(size);
    for (unsigned int i = 0; i < size; i++)
    {
        m_pings_count[i] = 0;
    }
    m_successed_pings.resize(size);
    m_total_diff.resize(size);
    m_average_ping.resize(size);
    m_countdown_activated = false;
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
        Log::verbose("SynchronizationProtocol", "Answering sequence %u", sequence);
        if (event->data.size() == 12 && !m_listener->isServer()) // countdown time in the message
        {
            uint16_t time_to_start = event->data.gui16(10);
            Log::info("SynchronizationProtocol", "Starting game in %u.", time_to_start);
            startCountdown(time_to_start);
        }
        else
            Log::verbose("SynchronizationProtocol", "No countdown for now.");
    }
    else // response
    {
        if (sequence >= m_pings[peer_id].size())
        {
            Log::warn("SynchronizationProtocol", "The sequence# %u isn't known.", sequence);
            return;
        }
        double current_time = Time::getRealTime();
        m_total_diff[peer_id] += current_time - m_pings[peer_id][sequence];
        Log::verbose("SynchronizationProtocol", "InstantPing is %u",
            (unsigned int)((current_time - m_pings[peer_id][sequence])*1000));
        m_successed_pings[peer_id]++;
        m_average_ping[peer_id] = (int)((m_total_diff[peer_id]/m_successed_pings[peer_id])*1000.0);

        Log::verbose("SynchronizationProtocol", "Ping is %u", m_average_ping[peer_id]);
    }
}

//-----------------------------------------------------------------------------

void SynchronizationProtocol::setup()
{
    Log::info("SynchronizationProtocol", "Ready !");
    m_countdown = 5000; // init the countdown to 5k
}

//-----------------------------------------------------------------------------

void SynchronizationProtocol::update()
{
    static double timer = Time::getRealTime();
    double current_time = Time::getRealTime();
    if (current_time > timer+0.1)
    {
        if (m_countdown_activated)
        {
            m_countdown -= (int)((current_time - m_last_countdown_update)*1000.0);
            m_last_countdown_update = current_time;
        }
        std::vector<STKPeer*> peers = NetworkManager::getInstance()->getPeers();
        for (unsigned int i = 0; i < peers.size(); i++)
        {
            NetworkString ns;
            ns.ai8(i).addUInt32(peers[i]->getClientServerToken()).addUInt8(1).addUInt32(m_pings[i].size());
            // now add the countdown if necessary
            if (m_countdown_activated)
            {
                ns.ai16(m_countdown);
                Log::info("SynchronizationProtocol", "Countdown value : %u", m_countdown);
            }
            Log::verbose("SynchronizationProtocol", "Added sequence number %u for peer %d", m_pings[i].size(), i);
            timer = current_time;
            m_pings[i].insert(std::pair<int,double>(m_pings_count[i], timer));
            m_listener->sendMessage(this, peers[i], ns, false);
            m_pings_count[i]++;
        }
        Log::info("SynchronizationProtocol", "Countdown remaining : %u", m_countdown);
    }
}

//-----------------------------------------------------------------------------

void SynchronizationProtocol::startCountdown(int ms_countdown)
{
    m_countdown_activated = true;
    m_countdown = ms_countdown;
    m_last_countdown_update = Time::getRealTime();
    Log::info("SynchronizationProtocol", "Countdown started.");
}
