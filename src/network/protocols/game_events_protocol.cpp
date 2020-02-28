#include "network/protocols/game_events_protocol.hpp"

#include "karts/abstract_kart.hpp"
#include "karts/controller/player_controller.hpp"
#include "modes/capture_the_flag.hpp"
#include "modes/linear_world.hpp"
#include "modes/soccer_world.hpp"
#include "network/event.hpp"
#include "network/game_setup.hpp"
#include "network/network_config.hpp"
#include "network/protocols/server_lobby.hpp"
#include "network/protocol_manager.hpp"
#include "network/rewind_manager.hpp"
#include "network/socket_address.hpp"
#include "network/stk_host.hpp"
#include "network/stk_peer.hpp"
#include "race/race_manager.hpp"

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
    m_last_finished_position = 1;
}   // GameEventsProtocol

// ----------------------------------------------------------------------------
GameEventsProtocol::~GameEventsProtocol()
{
}   // ~GameEventsProtocol

// ----------------------------------------------------------------------------
void GameEventsProtocol::update(int ticks)
{
    if (!World::getWorld())
        ProtocolManager::lock()->findAndTerminate(PROTOCOL_GAME_EVENTS);
}   // update

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
    LinearWorld* lw = dynamic_cast<LinearWorld*>(World::getWorld());
    switch (type)
    {
    case GE_KART_FINISHED_RACE:
        kartFinishedRace(data);     break;
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
    case GE_CTF_SCORED:
    {
        if (!ctf)
            throw std::invalid_argument("No CTF world");
        uint8_t kart_id = data.getUInt8();
        bool red_team_scored = data.getUInt8() == 1;
        int16_t new_kart_scores = data.getUInt16();
        int new_red_scores = data.getUInt8();
        int new_blue_scores = data.getUInt8();
        ctf->ctfScored(kart_id, red_team_scored, new_kart_scores,
            new_red_scores, new_blue_scores);
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
    case GE_CHECK_LINE:
    {
        if (!lw)
            throw std::invalid_argument("No linear world");
        if (NetworkConfig::get()->isClient())
            lw->updateCheckLinesClient(data);
        break;
    }
    default:
        Log::warn("GameEventsProtocol", "Unknown message type.");
        break;
    }
    return true;
}   // notifyEvent

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
    if (RaceManager::get()->modeHasLaps())
    {
        World::getWorld()->getKart(kart_id)
            ->setPosition(m_last_finished_position++);
    }
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
