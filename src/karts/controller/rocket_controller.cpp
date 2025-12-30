//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2024 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "karts/controller/rocket_controller.hpp"

#include "config/stk_config.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/controller/kart_control.hpp"
#include "karts/kart_gfx.hpp"
#include "karts/kart_properties.hpp"
#include "karts/max_speed.hpp"
#include "modes/linear_world.hpp"
#include "modes/world.hpp"
#include "network/network_string.hpp"
#include "tracks/drive_graph.hpp"
#include "tracks/drive_node.hpp"

// Duration constants - computed at first use since stk_config isn't available at static init
int RocketController::MIN_DURATION_TICKS = 0;
int RocketController::MAX_DURATION_TICKS = 0;

//-----------------------------------------------------------------------------
RocketController::RocketController(AbstractKart *kart,
                                   Controller* original_controller)
                : AIBaseLapController(kart)
{
    // Initialize duration constants on first use
    if (MIN_DURATION_TICKS == 0)
    {
        MIN_DURATION_TICKS = stk_config->time2Ticks(8.0f);
        MAX_DURATION_TICKS = stk_config->time2Ticks(12.0f);
    }

    m_original_controller = original_controller;
    m_start_ticks = World::getWorld()->getTicksSinceStart();
    m_is_active = true;

    setControllerName("Rocket");
}   // RocketController

//-----------------------------------------------------------------------------
RocketController::~RocketController()
{
}   // ~RocketController

//-----------------------------------------------------------------------------
void RocketController::reset()
{
    AIBaseLapController::reset();
    m_start_ticks = World::getWorld()->getTicksSinceStart();
    m_is_active = true;
}   // reset

//-----------------------------------------------------------------------------
bool RocketController::shouldTerminate() const
{
    World* world = World::getWorld();
    if (!world)
        return true;  // Terminate if no world (safety)

    int current_ticks = world->getTicksSinceStart();
    int elapsed_ticks = current_ticks - m_start_ticks;

    // Always run for at least MIN_DURATION
    if (elapsed_ticks < MIN_DURATION_TICKS)
        return false;

    // Check position - if in top half, can terminate after min duration
    int position = m_kart->getPosition();
    int num_karts = world->getNumKarts();
    bool in_top_half = position <= (num_karts / 2);

    if (in_top_half)
        return true;

    // If still in bottom half but max duration reached, terminate
    if (elapsed_ticks >= MAX_DURATION_TICKS)
        return true;

    return false;
}   // shouldTerminate

//-----------------------------------------------------------------------------
void RocketController::terminate()
{
    m_is_active = false;
    // The kart will handle restoring the original controller
}   // terminate

//-----------------------------------------------------------------------------
/** Find a point to steer toward that won't crash into walls.
 *  Based on EndController's simpler approach.
 */
void RocketController::findNonCrashingPoint(Vec3 *result)
{
    unsigned int sector = m_next_node_index[m_track_node];

    Vec3 direction;
    Vec3 step_track_coord;

    // Find the farthest sector we can drive to without crashing
    // Limit iterations to prevent infinite loop on unusual tracks
    int max_iterations = 100;
    while (max_iterations-- > 0)
    {
        int target_sector = m_next_node_index[sector];

        // Direction from kart to target sector
        direction = DriveGraph::get()->getNode(target_sector)->getCenter()
                  - m_kart->getXYZ();

        float len = direction.length();
        // Guard against division by zero
        if (m_kart_length <= 0.0f)
            break;
        int steps = int(len / m_kart_length);
        if (steps < 3) steps = 3;

        // Normalize direction
        if (len > 0.0f)
        {
            direction *= 1.0f / len;
        }

        Vec3 step_coord;
        // Test if we crash if we drive towards the target sector
        for (int i = 2; i < steps; ++i)
        {
            step_coord = m_kart->getXYZ() + direction * m_kart_length * float(i);

            DriveGraph::get()->spatialToTrack(&step_track_coord, step_coord, sector);

            float distance = fabsf(step_track_coord[0]);

            // If we are outside the track, the previous sector is our target
            if (distance + m_kart_width * 0.5f
                > DriveGraph::get()->getNode(sector)->getPathWidth() * 0.5f)
            {
                *result = DriveGraph::get()->getNode(sector)->getCenter();
                return;
            }
        }
        sector = target_sector;
    }
    // Fallback: if loop exhausted without finding crash point, use current sector
    *result = DriveGraph::get()->getNode(sector)->getCenter();
}   // findNonCrashingPoint

//-----------------------------------------------------------------------------
/** Handle steering using simple pathfinding - no drifting.
 *  Based on EndController's approach.
 */
void RocketController::handleSteering(float dt)
{
    Vec3 target_point;

    // If we're off the road, steer back to center
    float dist_to_center = fabsf(m_world->getDistanceToCenterForKart(m_kart->getWorldKartId()));
    float path_width = DriveGraph::get()->getNode(m_track_node)->getPathWidth();

    if (dist_to_center > 0.5f * path_width + 0.5f)
    {
        // Outside of road: steer to next node's center
        const int next = m_next_node_index[m_track_node];
        target_point = DriveGraph::get()->getNode(next)->getCenter();
    }
    else
    {
        // On the road: find a safe point ahead
        findNonCrashingPoint(&target_point);
    }

    // Use the inherited steerToPoint and setSteering methods
    setSteering(steerToPoint(target_point), dt);
}   // handleSteering

//-----------------------------------------------------------------------------
void RocketController::update(int ticks)
{
    // Check if we should terminate
    if (shouldTerminate())
    {
        terminate();
        return;
    }

    // Update base class (handles track node updates)
    AIBaseLapController::update(ticks);

    // Disable skidding, looking back, nitro, brake
    m_controls->setSkidControl(KartControl::SC_NONE);
    m_controls->setLookBack(false);
    m_controls->setNitro(false);
    m_controls->setBrake(false);

    // Handle steering with simple non-drifting AI
    float dt = stk_config->ticks2Time(ticks);
    handleSteering(dt);

    // Force full acceleration
    m_controls->setAccel(1.0f);

    // Apply speed boost - 300% max speed
    float base_max_speed = m_kart->getKartProperties()->getEngineMaxSpeed();
    float speed_increase = base_max_speed * (SPEED_MULTIPLIER - 1.0f);

    // Use moderate engine force to prevent physics issues
    float engine_boost = 500.0f;

    // Apply continuous speed boost (refresh every frame)
    m_kart->increaseMaxSpeed(MaxSpeed::MS_INCREASE_ZIPPER,
                             speed_increase,      // add_speed
                             engine_boost,        // engine_force
                             ticks + 5,           // duration (slightly longer than frame)
                             0);                  // fade_out_time

    // Keep the zipper flame effect going continuously
    m_kart->getKartGFX()->setCreationRateAbsolute(KartGFX::KGFX_ZIPPER, 800.0f);

    // Make kart invulnerable during rocket boost (immune to items)
    // Use a fixed duration that covers multiple frames to ensure continuous protection
    m_kart->setInvulnerableTicks(stk_config->time2Ticks(0.5f));
}   // update

//-----------------------------------------------------------------------------
bool RocketController::isLocalPlayerController() const
{
    // Return the original controller's value so we keep camera/effects
    if (m_original_controller)
        return m_original_controller->isLocalPlayerController();
    return false;
}   // isLocalPlayerController

//-----------------------------------------------------------------------------
bool RocketController::isPlayerController() const
{
    if (m_original_controller)
        return m_original_controller->isPlayerController();
    return false;
}   // isPlayerController

//-----------------------------------------------------------------------------
bool RocketController::saveState(BareNetworkString *buffer) const
{
    buffer->addUInt32(m_start_ticks);
    buffer->addUInt8(m_is_active ? 1 : 0);
    return true;
}   // saveState

//-----------------------------------------------------------------------------
void RocketController::rewindTo(BareNetworkString *buffer)
{
    m_start_ticks = buffer->getUInt32();
    m_is_active = buffer->getUInt8() != 0;
}   // rewindTo

//-----------------------------------------------------------------------------
bool RocketController::action(PlayerAction action, int value, bool dry_run)
{
    // Ignore player input during rocket boost
    return true;
}   // action

//-----------------------------------------------------------------------------
void RocketController::newLap(int lap)
{
    // Call base class implementation
    AIBaseLapController::newLap(lap);
}   // newLap

//-----------------------------------------------------------------------------
void RocketController::skidBonusTriggered()
{
    // No skid bonus during rocket
}   // skidBonusTriggered

//-----------------------------------------------------------------------------
void RocketController::finishedRace(float time)
{
    // Terminate rocket if race finishes
    terminate();
}   // finishedRace

//-----------------------------------------------------------------------------
void RocketController::crashed(const AbstractKart *k)
{
    // Continue rocket even on crash
}   // crashed

//-----------------------------------------------------------------------------
void RocketController::crashed(const Material *m)
{
    // Continue rocket even on crash
}   // crashed

//-----------------------------------------------------------------------------
void RocketController::handleZipper(bool play_sound)
{
    // Zippers still work during rocket
    m_kart->handleZipper(NULL, play_sound);
}   // handleZipper

//-----------------------------------------------------------------------------
void RocketController::collectedItem(const ItemState &item, float previous_energy)
{
    // Items still collected during rocket
}   // collectedItem

//-----------------------------------------------------------------------------
void RocketController::setPosition(int p)
{
    // Position changes handled by world
}   // setPosition
