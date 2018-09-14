#include "network/protocols/game_events_protocol.hpp"

#include "karts/abstract_kart.hpp"
#include "karts/controller/player_controller.hpp"
#include "modes/capture_the_flag.hpp"
#include "modes/soccer_world.hpp"
#include "network/event.hpp"
#include "network/game_setup.hpp"
#include "network/network_config.hpp"
#include "network/protocols/server_lobby.hpp"
#include "network/rewind_manager.hpp"
#include "network/stk_host.hpp"
#include "network/stk_peer.hpp"

#include <stdint.h>

/** This class handles all 'major' game events. E.g.
 *  finishing a race or goal etc. The game events manager is notified from the
 *  game code, and it calls the corresponding function in this class.
 *  The server then notifies all clients. Clients receive the message
 *  in the synchronous notifyEvent function here, decode the message
 *  and call the original game code. The functions name are identical,
 *  e.g. kartFinishedRace(some parameter) is called from the GameEventManager
 *  on the server, and the received message is then handled by 
 *  kartFinishedRace(const NetworkString &).
 */
GameEventsProtocol::GameEventsProtocol() : Protocol(PROTOCOL_GAME_EVENTS)
{
}   // GameEventsProtocol

// ----------------------------------------------------------------------------
GameEventsProtocol::~GameEventsProtocol()
{
}   // ~GameEventsProtocol

// ----------------------------------------------------------------------------
bool GameEventsProtocol::notifyEvent(Event* event)
{
    // Avoid crash in case that we still receive race events when
    // the race is actually over.
    if (event->getType() != EVENT_TYPE_MESSAGE || !World::getWorld())
        return true;
    NetworkString &data = event->data();
    if (data.size() < 1) // for type
    {
        Log::warn("GameEventsProtocol", "Too short message.");
        return true;
    }
    uint8_t type = data.getUInt8();
    CaptureTheFlag* ctf = dynamic_cast<CaptureTheFlag*>(World::getWorld());
    FreeForAll* ffa = dynamic_cast<FreeForAll*>(World::getWorld());
    SoccerWorld* sw = dynamic_cast<SoccerWorld*>(World::getWorld());
    switch (type)
    {
    case GE_KART_FINISHED_RACE:
        kartFinishedRace(data);     break;
    case GE_PLAYER_DISCONNECT:
        eliminatePlayer(data);      break;
    case GE_RESET_BALL:
    {
        if (!sw)
            throw std::invalid_argument("No soccer world");
        sw->handleResetBallFromServer(data);
        break;
    }
    case GE_PLAYER_GOAL:
    {
        if (!sw)
            throw std::invalid_argument("No soccer world");
        sw->handlePlayerGoalFromServer(data);
        break;
    }
    case GE_BATTLE_KART_SCORE:
    {
        if (!ffa)
            throw std::invalid_argument("No free-for-all world");
        ffa->setKartScoreFromServer(data);
        break;
    }
    case GE_CTF_ATTACH:
    {
        if (!ctf)
            throw std::invalid_argument("No CTF world");
        ctf->attachFlag(data);
        break;
    }
    case GE_CTF_RESET:
    {
        if (!ctf)
            throw std::invalid_argument("No CTF world");
        ctf->resetFlag(data);
        break;
    }
    case GE_STARTUP_BOOST:
    {
        if (NetworkConfig::get()->isServer())
        {
            uint8_t kart_id = data.getUInt8();
            if (!event->getPeer()->availableKartID(kart_id))
            {
                Log::warn("GameProtocol", "Wrong kart id %d from %s.",
                    kart_id, event->getPeer()->getAddress().toString().c_str());
                return true;
            }
            float f = LobbyProtocol::get<ServerLobby>()
                ->getStartupBoostOrPenaltyForKart(
                event->getPeer()->getAveragePing(), kart_id);
            NetworkString *ns = getNetworkString();
            ns->setSynchronous(true);
            ns->addUInt8(GE_STARTUP_BOOST).addUInt8(kart_id).addFloat(f);
            sendMessageToPeers(ns, true);
            delete ns;
        }
        else
        {
            uint8_t kart_id = data.getUInt8();
            float boost = data.getFloat();
            AbstractKart* k = World::getWorld()->getKart(kart_id);
            if (boost < 0.0f)
            {
                PlayerController* pc =
                    dynamic_cast<PlayerController*>(k->getController());
                pc->displayPenaltyWarning();
            }
            else
                k->setStartupBoost(boost);
        }
        break;
    }
    default:
        Log::warn("GameEventsProtocol", "Unkown message type.");
        break;
    }
    return true;
}   // notifyEvent

// ----------------------------------------------------------------------------
void GameEventsProtocol::eliminatePlayer(const NetworkString &data)
{
    assert(NetworkConfig::get()->isClient());
    if (data.size() < 1)
    {
        Log::warn("GameEventsProtocol", "eliminatePlayer: Too short message.");
    }
    int kartid = data.getUInt8();
    World::getWorld()->eliminateKart(kartid, false/*notify_of_elimination*/);
    World::getWorld()->getKart(kartid)->setPosition(
        World::getWorld()->getCurrentNumKarts() + 1);
    World::getWorld()->getKart(kartid)->finishedRace(
        World::getWorld()->getTime());
}   // eliminatePlayer

// ----------------------------------------------------------------------------
/** This function is called from the server when a kart finishes a race. It
 *  sends a notification to all clients about this event.
 *  \param kart The kart that finished the race.
 *  \param time The time at which the kart finished.
 */
void GameEventsProtocol::kartFinishedRace(AbstractKart *kart, float time)
{
    NetworkString *ns = getNetworkString(20);
    ns->setSynchronous(true);
    ns->addUInt8(GE_KART_FINISHED_RACE).addUInt8(kart->getWorldKartId())
       .addFloat(time);
    sendMessageToPeers(ns, /*reliable*/true);
    delete ns;
}   // kartFinishedRace

// ----------------------------------------------------------------------------
/** This function is called on a client when it receives a kartFinishedRace
 *  event from the server. It updates the game with this information.
 *  \param ns The message from the server.
 */
void GameEventsProtocol::kartFinishedRace(const NetworkString &ns)
{
    if (ns.size() < 5)
    {
        Log::warn("GameEventsProtocol", "kartFinisheRace: Too short message.");
        return;
    }

    uint8_t kart_id = ns.getUInt8();
    float time      = ns.getFloat();
    World::getWorld()->getKart(kart_id)->finishedRace(time,
                                                      /*from_server*/true);
}   // kartFinishedRace

// ----------------------------------------------------------------------------
void GameEventsProtocol::sendStartupBoost(uint8_t kart_id)
{
    NetworkString *ns = getNetworkString();
    ns->setSynchronous(true);
    ns->addUInt8(GE_STARTUP_BOOST).addUInt8(kart_id);
    sendToServer(ns, /*reliable*/true);
    delete ns;
}   // sendStartupBoost
