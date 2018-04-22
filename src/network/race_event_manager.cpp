
#include "network/race_event_manager.hpp"

#include "karts/controller/controller.hpp"
#include "modes/world.hpp"
#include "network/network_config.hpp"
#include "network/protocols/game_events_protocol.hpp"
#include "network/rewind_manager.hpp"
#include "utils/profiler.hpp"

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
 *  \param ticks Number of physics time steps - should be 1.
 */
void RaceEventManager::update(int ticks)
{
    // Replay all recorded events up to the current time
    // This might adjust dt - if a new state is being played, the dt is
    // determined from the last state till 'now'
    PROFILER_PUSH_CPU_MARKER("RaceEvent:play event", 100, 100, 100);
    RewindManager::get()->playEventsTill(World::getWorld()->getTimeTicks(),
                                         &ticks);
    PROFILER_POP_CPU_MARKER();
    World::getWorld()->updateWorld(ticks);

    // if the race is over
    if (World::getWorld()->getPhase() >= WorldStatus::RESULT_DISPLAY_PHASE &&
        World::getWorld()->getPhase() != WorldStatus::IN_GAME_MENU_PHASE)
    {
        // consider the world finished.
        stop();
        Log::info("RaceEventManager", "The game is considered finish.");
    }
}   // update

// ----------------------------------------------------------------------------
bool RaceEventManager::isRaceOver()
{
    if(!World::getWorld())
        return false;
    return (World::getWorld()->getPhase() > WorldStatus::RACE_PHASE &&
        World::getWorld()->getPhase() != WorldStatus::IN_GAME_MENU_PHASE);
}   // isRaceOver

// ----------------------------------------------------------------------------
void RaceEventManager::kartFinishedRace(AbstractKart *kart, float time)
{
    if (auto game_events_protocol = m_game_events_protocol.lock())
        game_events_protocol->kartFinishedRace(kart, time);
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
    if (auto game_events_protocol = m_game_events_protocol.lock())
        game_events_protocol->collectedItem(item, kart);
}   // collectedItem

