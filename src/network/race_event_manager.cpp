
#include "network/race_event_manager.hpp"

#include "karts/controller/controller.hpp"
#include "modes/world.hpp"
#include "network/network_config.hpp"
#include "network/protocols/game_events_protocol.hpp"
#include "network/rewind_manager.hpp"
#include "utils/profiler.hpp"
#include "utils/stk_process.hpp"

//=============================================================================
RaceEventManager* g_race_event_manager[PT_COUNT];
// ----------------------------------------------------------------------------
RaceEventManager* RaceEventManager::get()
{
    ProcessType type = STKProcess::getType();
    return g_race_event_manager[type];
}   // get

// ----------------------------------------------------------------------------
void RaceEventManager::create()
{
    ProcessType type = STKProcess::getType();
    g_race_event_manager[type] = new RaceEventManager();
}   // create

// ----------------------------------------------------------------------------
void RaceEventManager::destroy()
{
    ProcessType type = STKProcess::getType();
    delete g_race_event_manager[type];
    g_race_event_manager[type] = NULL;
}   // destroy

// ----------------------------------------------------------------------------
void RaceEventManager::clear()
{
    memset(g_race_event_manager, 0, sizeof(g_race_event_manager));
}   // clear

// ----------------------------------------------------------------------------
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
 *  \param fast_forward If true, then only rewinders in network will be
 *  updated, but not the physics.
 */
void RaceEventManager::update(int ticks, bool fast_forward)
{
    // Replay all recorded events up to the current time
    // This might adjust dt - if a new state is being played, the dt is
    // determined from the last state till 'now'
    PROFILER_PUSH_CPU_MARKER("RaceEvent:play event", 100, 100, 100);
    RewindManager::get()->playEventsTill(World::getWorld()->getTicksSinceStart(),
                                         fast_forward);
    PROFILER_POP_CPU_MARKER();
    if (!fast_forward)
        World::getWorld()->updateWorld(ticks);
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
