
#include "network/race_event_manager.hpp"

#include "karts/controller/controller.hpp"
#include "modes/world.hpp"
#include "network/network_config.hpp"
#include "network/protocol_manager.hpp"
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
 *  World::updateWorld(). 
 */
void RaceEventManager::update(float dt)
{
    // This can happen in case of disconnects - protocol manager is
    // shut down, but still events to process.
    if(!ProtocolManager::getInstance())
        return;

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
void RaceEventManager::controllerAction(Controller* controller,
                                        PlayerAction action, int value)
{
    ControllerEventsProtocol* protocol = static_cast<ControllerEventsProtocol*>(
        ProtocolManager::getInstance()->getProtocol(PROTOCOL_CONTROLLER_EVENTS));
    if (protocol)
        protocol->controllerAction(controller, action, value);
}   // controllerAction

