
#include "network/race_event_manager.hpp"

#include "karts/controller/controller.hpp"
#include "modes/world.hpp"
#include "network/network_config.hpp"
#include "network/protocol_manager.hpp"
#include "network/protocols/synchronization_protocol.hpp"
#include "network/protocols/controller_events_protocol.hpp"
#include "network/protocols/game_events_protocol.hpp"


RaceEventManager::RaceEventManager()
{
    m_running = false;
}   // RaceEventManager

// ----------------------------------------------------------------------------
RaceEventManager::~RaceEventManager()
{
}   // ~RaceEventManager

// ----------------------------------------------------------------------------
/** In network games this update function is called instead of
 *  World::updateWorld(). This allow this function to postpone calling
 *  the worl update while the countdown from the SynchronisationProtocol is
 *  running.
 */
void RaceEventManager::update(float dt)
{
    // This can happen in case of disconnects - protocol manager is
    // shut down, but still events to process.
    if(!ProtocolManager::getInstance())
        return;

    SynchronizationProtocol* protocol = static_cast<SynchronizationProtocol*>(
            ProtocolManager::getInstance()->getProtocol(PROTOCOL_SYNCHRONIZATION));
    if (protocol) // The existance of this protocol indicates that we play online
    {
        Log::debug("RaceEventManager", "Countdown value is %f",
                   protocol->getCountdown());
        if (protocol->getCountdown() > 0.0)
        {
            return;
        }
        World::getWorld()->setNetworkWorld(true);
    }
    World::getWorld()->updateWorld(dt);

    // if the race is over
    if (World::getWorld()->getPhase() >= WorldStatus::RESULT_DISPLAY_PHASE)
    {
        // consider the world finished.
        stop();
        Log::info("RaceEventManager", "The game is considered finish.");
    }
}   // update

// ----------------------------------------------------------------------------
void RaceEventManager::start()
{
    m_running = true;
}   // start

// ----------------------------------------------------------------------------
void RaceEventManager::stop()
{
    m_running = false;
}   // stop

// ----------------------------------------------------------------------------
bool RaceEventManager::isRaceOver()
{
    if(!World::getWorld())
        return false;
    return (World::getWorld()->getPhase() >  WorldStatus::RACE_PHASE);
}   // isRaceOver

// ----------------------------------------------------------------------------
void RaceEventManager::kartFinishedRace(AbstractKart *kart, float time)
{
    GameEventsProtocol* protocol = static_cast<GameEventsProtocol*>(
        ProtocolManager::getInstance()->getProtocol(PROTOCOL_GAME_EVENTS));
    protocol->kartFinishedRace(kart, time);
}   // kartFinishedRace

// ----------------------------------------------------------------------------
/** Called from the item manager on a server. It triggers a notification to
 *  all clients in the GameEventsProtocol.
 *  \param item The item that was collected.
 *  \param kart The kart that collected the item.
 */
void RaceEventManager::collectedItem(Item *item, AbstractKart *kart)
{
    // this is only called in the server
    assert(NetworkConfig::get()->isServer());

    GameEventsProtocol* protocol = static_cast<GameEventsProtocol*>(
        ProtocolManager::getInstance()->getProtocol(PROTOCOL_GAME_EVENTS));
    protocol->collectedItem(item,kart);
}   // collectedItem

// ----------------------------------------------------------------------------
/** A message from the server to all clients that it is now starting the
 *  'ready' phase. Clients will wait for this event before they display
 *  RSG. This will make sure that the server time is always ahead of 
 *  the client time.
 */
void RaceEventManager::startReadySetGo()
{
    // this is only called in the server
    assert(NetworkConfig::get()->isServer());

    GameEventsProtocol* protocol = static_cast<GameEventsProtocol*>(
        ProtocolManager::getInstance()->getProtocol(PROTOCOL_GAME_EVENTS));
    protocol->startReadySetGo();
}   // startReadySetGo
// ----------------------------------------------------------------------------
void RaceEventManager::controllerAction(Controller* controller,
                                        PlayerAction action, int value)
{
    ControllerEventsProtocol* protocol = static_cast<ControllerEventsProtocol*>(
        ProtocolManager::getInstance()->getProtocol(PROTOCOL_CONTROLLER_EVENTS));
    if (protocol)
        protocol->controllerAction(controller, action, value);
}   // controllerAction

