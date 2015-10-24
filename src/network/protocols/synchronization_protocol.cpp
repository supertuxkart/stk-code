#include "network/protocols/synchronization_protocol.hpp"

#include "network/event.hpp"
#include "network/network_manager.hpp"
#include "network/protocols/kart_update_protocol.hpp"
#include "network/protocols/controller_events_protocol.hpp"
#include "network/protocols/game_events_protocol.hpp"
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

bool SynchronizationProtocol::notifyEventAsynchronous(Event* event)
{
    if (event->getType() != EVENT_TYPE_MESSAGE)
        return true;
    const NetworkString &data = event->data();
    if (data.size() < 10)
    {
        Log::warn("SynchronizationProtocol", "Received a message too short.");
        return true;
    }
    uint8_t talk_id = data.gui8();
    uint32_t token = data.gui32(1);
    uint32_t request = data.gui8(5);
    uint32_t sequence = data.gui32(6);

    std::vector<STKPeer*> peers = NetworkManager::getInstance()->getPeers();
    assert(peers.size() > 0);

    if (ProtocolManager::getInstance()->isServer())
    {
        if (talk_id > peers.size())
        {
            Log::warn("SynchronizationProtocol", "The ID isn't known.");
            return true;
        }
    }

    uint8_t peer_id = 0;
    for (unsigned int i = 0; i < peers.size(); i++)
    {
        if (peers[i]->isSamePeer(event->getPeer()))
        {
            peer_id = i;
        }
    }
    if (peers[peer_id]->getClientServerToken() != token)
    {
        Log::warn("SynchronizationProtocol", "Bad token from peer %d", talk_id);
        return true;
    }

    if (request)
    {
        NetworkString response(10);
        response.ai8(data.gui8(talk_id)).ai32(token).ai8(0).ai32(sequence);
        sendMessage(peers[peer_id], response, false);
        Log::verbose("SynchronizationProtocol", "Answering sequence %u", sequence);

        // countdown time in the message
        if (data.size() == 14 && !ProtocolManager::getInstance()->isServer())
        {
            uint32_t time_to_start = data.gui32(10);
            Log::debug("SynchronizationProtocol", "Request to start game in %d.", time_to_start);
            if (!m_countdown_activated)
                startCountdown(time_to_start);
            else
                m_countdown = (double)(time_to_start/1000.0);
        }
        else
            Log::verbose("SynchronizationProtocol", "No countdown for now.");
    }
    else // response
    {
        if (sequence >= m_pings[peer_id].size())
        {
            Log::warn("SynchronizationProtocol", "The sequence# %u isn't known.", sequence);
            return true;
        }
        double current_time = StkTime::getRealTime();
        m_total_diff[peer_id] += current_time - m_pings[peer_id][sequence];
        Log::verbose("SynchronizationProtocol", "InstantPing is %u",
            (unsigned int)((current_time - m_pings[peer_id][sequence])*1000));
        m_successed_pings[peer_id]++;
        m_average_ping[peer_id] = (int)((m_total_diff[peer_id]/m_successed_pings[peer_id])*1000.0);

        Log::debug("SynchronizationProtocol", "Ping is %u", m_average_ping[peer_id]);
    }
    return true;
}

//-----------------------------------------------------------------------------

void SynchronizationProtocol::setup()
{
    Log::info("SynchronizationProtocol", "Ready !");
    m_countdown = 5.0; // init the countdown to 5s
    m_has_quit = false;
}

//-----------------------------------------------------------------------------

void SynchronizationProtocol::asynchronousUpdate()
{
    static double timer = StkTime::getRealTime();
    double current_time = StkTime::getRealTime();
    if (m_countdown_activated)
    {
        m_countdown -= (current_time - m_last_countdown_update);
        m_last_countdown_update = current_time;
        Log::debug("SynchronizationProtocol", "Update! Countdown remaining : %f", m_countdown);
        if (m_countdown < 0.0 && !m_has_quit)
        {
            m_has_quit = true;
            Log::info("SynchronizationProtocol", "Countdown finished. Starting now.");
            (new KartUpdateProtocol())->requestStart();
            (new ControllerEventsProtocol())->requestStart();
            (new GameEventsProtocol())->requestStart();
            requestTerminate();
            return;
        }
        static int seconds = -1;
        if (seconds == -1)
        {
            seconds = (int)(ceil(m_countdown));
        }
        else if (seconds != (int)(ceil(m_countdown)))
        {
            seconds = (int)(ceil(m_countdown));
            Log::info("SynchronizationProtocol", "Starting in %d seconds.", seconds);
        }
    }
    if (current_time > timer+0.1)
    {
        std::vector<STKPeer*> peers = NetworkManager::getInstance()->getPeers();
        for (unsigned int i = 0; i < peers.size(); i++)
        {
            NetworkString ns(10);
            ns.ai8(i).addUInt32(peers[i]->getClientServerToken()).addUInt8(1).addUInt32(m_pings[i].size());
            // now add the countdown if necessary
            if (m_countdown_activated && 
                ProtocolManager::getInstance()->isServer())
            {
                ns.addUInt32((int)(m_countdown*1000.0));
                Log::debug("SynchronizationProtocol", "CNTActivated: Countdown value : %f", m_countdown);
            }
            Log::verbose("SynchronizationProtocol", "Added sequence number %u for peer %d", m_pings[i].size(), i);
            timer = current_time;
            m_pings[i].insert(std::pair<int,double>(m_pings_count[i], timer));
            sendMessage(peers[i], ns, false);
            m_pings_count[i]++;
        }
    }

}

//-----------------------------------------------------------------------------

void SynchronizationProtocol::startCountdown(int ms_countdown)
{
    m_countdown_activated = true;
    m_countdown = (double)(ms_countdown)/1000.0;
    m_last_countdown_update = StkTime::getRealTime();
    Log::info("SynchronizationProtocol", "Countdown started with value %f", m_countdown);
}
