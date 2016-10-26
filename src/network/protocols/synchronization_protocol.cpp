#include "network/protocols/synchronization_protocol.hpp"

#include "network/event.hpp"
#include "network/network_config.hpp"
#include "network/protocols/kart_update_protocol.hpp"
#include "network/protocols/controller_events_protocol.hpp"
#include "network/protocols/game_events_protocol.hpp"
#include "network/stk_host.hpp"
#include "network/stk_peer.hpp"
#include "utils/time.hpp"

//-----------------------------------------------------------------------------

SynchronizationProtocol::SynchronizationProtocol() 
                       : Protocol(PROTOCOL_SYNCHRONIZATION)
{
    unsigned int size = STKHost::get()->getPeerCount();
    m_pings.resize(size, std::map<uint32_t,double>());
    m_successed_pings.resize(size, 0);
    m_total_diff.resize(size, 0);
    m_average_ping.resize(size, 0);
    m_pings_count = 0;
    m_countdown_activated = false;
    m_last_time = -1;
}   // SynchronizationProtocol

//-----------------------------------------------------------------------------
SynchronizationProtocol::~SynchronizationProtocol()
{
}   // ~SynchronizationProtocol

//-----------------------------------------------------------------------------
void SynchronizationProtocol::setup()
{
    Log::info("SynchronizationProtocol", "Ready !");
    m_countdown = 5.0; // init the countdown to 5s
    m_has_quit = false;
}   // setup
 //-----------------------------------------------------------------------------
/** Called when receiving a message. On the client side the message is a ping
 *  from the server, which is answered back. The client will also check if the
 *  server has started the countdown (which is indicated in the ping message).
 *  On the server the received message is a reply to a previous ping request.
 *  The server will keep track of average latency.
 */
bool SynchronizationProtocol::notifyEventAsynchronous(Event* event)
{
    if (event->getType() != EVENT_TYPE_MESSAGE)
        return true;
    if(!checkDataSize(event, 5)) return true;

    const NetworkString &data = event->data();
    uint32_t request  = data.getUInt8();
    uint32_t sequence = data.getUInt32();

    const std::vector<STKPeer*> &peers = STKHost::get()->getPeers();
    assert(peers.size() > 0);

    // Find the right peer id. The host id (i.e. each host sendings its
    // host id) can not be used here, since host ids can have gaps (if a
    // host should disconnect)
    uint8_t peer_id = -1;
    for (unsigned int i = 0; i < peers.size(); i++)
    {
        if (peers[i]->isSamePeer(event->getPeer()))
        {
            peer_id = i;
            break;
        }
    }

    if (request)
    {
        // Only a client should receive a request for a ping response
        assert(NetworkConfig::get()->isClient());
        NetworkString *response = getNetworkString(5);
        // The '0' indicates a response to a ping request
        response->addUInt8(0).addUInt32(sequence);
        event->getPeer()->sendPacket(response, false);
        delete response;
        Log::verbose("SynchronizationProtocol", "Answering sequence %u at %lf",
                     sequence, StkTime::getRealTime());

        // countdown time in the message
        if (data.size() == 4)
        {
            float time_to_start = data.getFloat();
            Log::debug("SynchronizationProtocol",
                       "Request to start game in %f.", time_to_start);
            if (!m_countdown_activated)
                startCountdown(time_to_start);
            else
            {
                // Adjust the time based on the value sent from the server.
                m_countdown = time_to_start;
            }
        }
        else
            Log::verbose("SynchronizationProtocol", "No countdown for now.");
    }
    else // receive response to a ping request
    {
        // Only a server should receive this kind of message
        assert(NetworkConfig::get()->isServer());
        if (sequence >= m_pings[peer_id].size())
        {
            Log::warn("SynchronizationProtocol",
                      "The sequence# %u isn't known.", sequence);
            return true;
        }
        double current_time = StkTime::getRealTime();
        m_total_diff[peer_id] += current_time - m_pings[peer_id][sequence];
        m_successed_pings[peer_id]++;
        m_average_ping[peer_id] =
            (int)((m_total_diff[peer_id]/m_successed_pings[peer_id])*1000.0);

        Log::debug("SynchronizationProtocol",
            "Peer %d sequence %d ping %u average %u at %lf",
            peer_id, sequence,
            (unsigned int)((current_time - m_pings[peer_id][sequence])*1000),
            m_average_ping[peer_id],
            StkTime::getRealTime());
    }
    return true;
}   // notifyEventAsynchronous

//-----------------------------------------------------------------------------
/** Waits for the countdown to be started. On the server the start of the
 *  countdown is triggered by the StartGameProtocol::startRace(), which is
 *  called once all clients have confirmed that they are ready to start.
 *  The server will send a ping request to each client once a second, and
 *  include the information if the countdown has started (and its current
 *  value). On the client the countdown is started in notifyEvenAsynchronous()
 *  when a server ping is received that indicates that the countdown has
 *  started. The measured times can be used later to estimate the latency
 *  between server and client.
 */
void SynchronizationProtocol::asynchronousUpdate()
{
    float current_time = float(StkTime::getRealTime());
    if (m_countdown_activated)
    {
        m_countdown -= (current_time - m_last_countdown_update);
        m_last_countdown_update = current_time;
        Log::debug("SynchronizationProtocol",
                   "Update! Countdown remaining : %f", m_countdown);
        if (m_countdown < 0.0 && !m_has_quit)
        {
            m_has_quit = true;
            Log::info("SynchronizationProtocol",
                      "Countdown finished. Starting now.");
            (new KartUpdateProtocol())->requestStart();
            (new ControllerEventsProtocol())->requestStart();
            (new GameEventsProtocol())->requestStart();
            requestTerminate();
            return;
        }
    }   // if m_countdown_activated

    if (NetworkConfig::get()->isServer() &&  current_time > m_last_time+1)
    {
        const std::vector<STKPeer*> &peers = STKHost::get()->getPeers();
        for (unsigned int i = 0; i < peers.size(); i++)
        {
            NetworkString *ping_request = 
                            getNetworkString(m_countdown_activated ? 9 : 5);
            ping_request->addUInt8(1).addUInt32(m_pings[i].size());
            // Server adds the countdown if it has started. This will indicate
            // to the client to start the countdown as well (first time the 
            // message is received), or to update the countdown time.
            if (m_countdown_activated)
            {
                ping_request->addFloat(m_countdown);
                Log::debug("SynchronizationProtocol",
                           "CNTActivated: Countdown value : %f", m_countdown);
            }
            Log::verbose("SynchronizationProtocol",
                         "Added sequence number %u for peer %d at %lf",
                         m_pings[i].size(), i, StkTime::getRealTime());
            m_pings[i] [ m_pings_count ] = current_time;
            peers[i]->sendPacket(ping_request, false);
            delete ping_request;
        }   // for i M peers
        m_last_time = current_time;
        m_pings_count++;
    }   // if current_time > m_last_time + 0.1
}   // asynchronousUpdate

//-----------------------------------------------------------------------------
/** Starts the countdown on this machine. On the server side this function
 *  is called from the StartGameProtocol (when all players have confirmed that
 *  they are ready to play). On the client side this function is called from
 *  this protocol when a message from the server is received indicating that
 *  the countdown has to be started.
 *  \param ms_countdown Countdown to use in ms.
 */
void SynchronizationProtocol::startCountdown(float ms_countdown)
{
    m_countdown_activated   = true;
    m_countdown             = ms_countdown;
    m_last_countdown_update = float(StkTime::getRealTime());
    Log::info("SynchronizationProtocol", "Countdown started with value %f",
              m_countdown);
}   // startCountdown
