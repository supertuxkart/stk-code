#include "network/network_world.hpp"

#include "network/network_config.hpp"
#include "network/protocol_manager.hpp"
#include "network/protocols/synchronization_protocol.hpp"
#include "network/protocols/controller_events_protocol.hpp"
#include "network/protocols/game_events_protocol.hpp"
#include "modes/world.hpp"


NetworkWorld::NetworkWorld()
{
    m_running = false;
}   // NetworkWorld

// ----------------------------------------------------------------------------
#include "karts/controller/controller.hpp"
NetworkWorld::~NetworkWorld()
{
}   // ~NetworkWorld

// ----------------------------------------------------------------------------
void NetworkWorld::update(float dt)
{
    // This can happen in case of disconnects - protocol manager is
    // shut down, but still events to process.
    if(!ProtocolManager::getInstance())
        return;

    SynchronizationProtocol* protocol = static_cast<SynchronizationProtocol*>(
            ProtocolManager::getInstance()->getProtocol(PROTOCOL_SYNCHRONIZATION));
    if (protocol) // if this protocol exists, that's that we play online
    {
        Log::debug("NetworkWorld", "Coutdown value is %f",
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
        Log::info("NetworkWorld", "The game is considered finish.");
    }
}   // update

// ----------------------------------------------------------------------------
void NetworkWorld::start()
{
    m_running = true;
}   // start

// ----------------------------------------------------------------------------
void NetworkWorld::stop()
{
    m_running = false;
}   // stop

// ----------------------------------------------------------------------------
bool NetworkWorld::isRaceOver()
{
    if(!World::getWorld())
        return false;
    return (World::getWorld()->getPhase() >  WorldStatus::RACE_PHASE);
}   // isRaceOver

// ----------------------------------------------------------------------------
/** Called from the item manager on a server. It triggers a notification to
 *  all clients in the GameEventsProtocol.
 *  \param item The item that was collected.
 *  \param kart The kart that collected the item.
 */
void NetworkWorld::collectedItem(Item *item, AbstractKart *kart)
{
    // this is only called in the server
    assert(NetworkConfig::get()->isServer());

    GameEventsProtocol* protocol = static_cast<GameEventsProtocol*>(
        ProtocolManager::getInstance()->getProtocol(PROTOCOL_GAME_EVENTS));
    protocol->collectedItem(item,kart);
}   // collectedItem

// ----------------------------------------------------------------------------
void NetworkWorld::controllerAction(Controller* controller,
                                    PlayerAction action, int value)
{
    ControllerEventsProtocol* protocol = static_cast<ControllerEventsProtocol*>(
        ProtocolManager::getInstance()->getProtocol(PROTOCOL_CONTROLLER_EVENTS));
    if (protocol)
        protocol->controllerAction(controller, action, value);
}   // controllerAction

