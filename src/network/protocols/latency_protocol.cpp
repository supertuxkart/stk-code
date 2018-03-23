#include "network/protocols/latency_protocol.hpp"

#include "network/event.hpp"
#include "network/network_config.hpp"
#include "network/stk_host.hpp"
#include "network/stk_peer.hpp"
#include "utils/time.hpp"

//-----------------------------------------------------------------------------
/** This protocol tries to determine the average latency between client and
 *  server. While this information is not used atm, it might be useful for
 *  the server to determine how much behind the clients it should start.
 *  FIXME: ATM the main thread will load the world as part of an update
 *  of the ProtocolManager (which updates the protocols). Since all protocols
 *  are locked dusing this update, the synchronisation protocol is actually
 *  delayed from starting while world is loading (since finding the protocol
 *  for a message requires the protocol lock) - causing at least two frames
 *  of significanlty delayed pings :(
 */
LatencyProtocol::LatencyProtocol() 
               : Protocol(PROTOCOL_SYNCHRONIZATION)
{
    for (std::shared_ptr<STKPeer> peer : STKHost::get()->getPeers())
    {
        m_pings[peer] = std::map<uint32_t,double>();
        m_average_ping[peer] = 0;
        m_successed_pings[peer] = 0;
        m_total_diff[peer] = 0.0;
    }
    m_pings_count = 0;
    m_last_time = 0.0;
}   // LatencyProtocol

//-----------------------------------------------------------------------------
LatencyProtocol::~LatencyProtocol()
{
    for (auto p : m_average_ping)
    {
        std::string peer_name = "disconnected";
        if (auto peer = p.first.lock())
            peer_name = peer->getAddress().toString();
        Log::info("LatencyProtocol", "Peer %s: Average ping %u.",
            peer_name.c_str(), p.second);
    }
}   // ~LatencyProtocol

//-----------------------------------------------------------------------------
void LatencyProtocol::setup()
{
    Log::info("LatencyProtocol", "Ready !");
}   // setup

 //-----------------------------------------------------------------------------
/** Called when receiving a message. On the client side the message is a ping
 *  from the server, which is answered back. On the server the received message
 *  is a reply to a previous ping request. The server will keep track of
 *  average latency.
 */
bool LatencyProtocol::notifyEventAsynchronous(Event* event)
{
    if (event->getType() != EVENT_TYPE_MESSAGE)
        return true;
    if(!checkDataSize(event, 5)) return true;

    const NetworkString &data = event->data();
    uint32_t request  = data.getUInt8();
    uint32_t sequence = data.getUInt32();

    if (request)
    {
        // Only a client should receive a request for a ping response
        assert(NetworkConfig::get()->isClient());
        NetworkString *response = getNetworkString(5);
        // The '0' indicates a response to a ping request
        response->addUInt8(0).addUInt32(sequence);
        event->getPeer()->sendPacket(response, false);
        delete response;
    }
    else // receive response to a ping request
    {
        // Only a server should receive this kind of message
        assert(NetworkConfig::get()->isServer());
        if (sequence >= m_pings[event->getPeerSP()].size())
        {
            Log::warn("LatencyProtocol",
                      "The sequence# %u isn't known.", sequence);
            return true;
        }
        double current_time = StkTime::getRealTime();
        m_total_diff[event->getPeerSP()] +=
            current_time - m_pings[event->getPeerSP()][sequence];
        m_successed_pings[event->getPeerSP()]++;
        m_average_ping[event->getPeerSP()] =
            (int)((m_total_diff[event->getPeerSP()] /
            m_successed_pings[event->getPeerSP()]) * 1000.0);

        Log::debug("LatencyProtocol",
            "Peer %s: sequence %d ping %u average %u at %lf",
            event->getPeerSP()->getAddress().toString().c_str(), sequence,
            (unsigned int)(
            (current_time - m_pings[event->getPeerSP()][sequence]) * 1000),
            m_average_ping[event->getPeerSP()],
            StkTime::getRealTime());
    }
    return true;
}   // notifyEventAsynchronous

//-----------------------------------------------------------------------------
/** Waits for the countdown to be started. On the server the start of the
 *  countdown is triggered by ServerLobby::finishedLoadingWorld(),
 *  which is called once all clients have confirmed that they are ready to
 *  start. The server will send a ping request to each client once a second,
 *  and include the information if the countdown has started (and its current
 *  value). On the client the countdown is started in notifyEvenAsynchronous()
 *  when a server ping is received that indicates that the countdown has
 *  started. The measured times can be used later to estimate the latency
 *  between server and client.
 */
void LatencyProtocol::asynchronousUpdate()
{
    double current_time = StkTime::getRealTime();
    if (NetworkConfig::get()->isServer() &&  current_time > m_last_time+1)
    {
        for (auto& p : m_pings)
        {
            if (auto peer = p.first.lock())
            {
                NetworkString *ping_request = getNetworkString(5);
                ping_request->addUInt8(1).addUInt32((unsigned)p.second.size());
                peer->sendPacket(ping_request, false);
                p.second[m_pings_count] = current_time;
            }
        }   // for i < all peers
        m_last_time = current_time;
        m_pings_count++;
    }   // if current_time > m_last_time + 0.1
}   // asynchronousUpdate

