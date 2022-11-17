//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2015 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006-2015 Eduardo Hernandez Munoz
//  Copyright (C) 2008-2015 Joerg Henrichs
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


#include "karts/controller/skidding_ai.hpp"

#ifdef AI_DEBUG
#  include "graphics/irr_driver.hpp"
#endif
#include "graphics/show_curve.hpp"
#include "graphics/slip_stream.hpp"
#include "items/attachment.hpp"
#include "items/item_manager.hpp"
#include "items/powerup.hpp"
#include "items/projectile_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/controller/kart_control.hpp"
#include "karts/controller/ai_properties.hpp"
#include "karts/kart_properties.hpp"
#include "karts/max_speed.hpp"
#include "karts/rescue_animation.hpp"
#include "karts/skidding.hpp"
#include "modes/linear_world.hpp"
#include "modes/profile_world.hpp"
#include "physics/triangle_mesh.hpp"
#include "race/race_manager.hpp"
#include "tracks/drive_graph.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"
#include "utils/log.hpp"
#include "utils/vs.hpp"

#include <line2d.h>

#ifdef AI_DEBUG
#  include "irrlicht.h"
   using namespace irr;
#endif

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <iostream>

SkiddingAI::SkiddingAI(AbstractKart *kart)
                   : AIBaseLapController(kart)
{
    m_item_manager = Track::getCurrentTrack()->getItemManager();
    reset();
    // Determine if this AI has superpowers, which happens e.g.
    // for the final race challenge against nolok.
    m_superpower = RaceManager::get()->getAISuperPower();

    m_point_selection_algorithm = PSA_DEFAULT;
    setControllerName("Skidding");

    // Use this define in order to compare the different algorithms that
    // select the next point to steer at.
#undef COMPARE_AIS
#ifdef COMPARE_AIS
    std::string name("");
    m_point_selection_algorithm = m_kart->getWorldKartId() % 2
                                ? PSA_DEFAULT : PSA_NEW;
    switch(m_point_selection_algorithm)
    {
    case PSA_NEW     : name = "New";     break;
    case PSA_DEFAULT : name = "Default"; break;
    }
    setControllerName(name);
#endif


    // Draw various spheres on the track for an AI
#ifdef AI_DEBUG
    for(unsigned int i=0; i<4; i++)
    {
        video::SColor col_debug(128, i==0 ? 128 : 0,
                                     i==1 ? 128 : 0,
                                     i==2 ? 128 : 0);
        m_debug_sphere[i] = irr_driver->addSphere(1.0f, col_debug);
        m_debug_sphere[i]->setVisible(false);
    }
    m_debug_sphere[m_point_selection_algorithm]->setVisible(true);
    m_item_sphere  = irr_driver->addSphere(1.0f, video::SColor(255, 0, 255, 0));

#define CURVE_PREDICT1   0
#define CURVE_KART       1
#define CURVE_LEFT       2
#define CURVE_RIGHT      3
#define CURVE_AIM        4
#define CURVE_QG         5
#define NUM_CURVES (CURVE_QG+1)

    m_curve   = new ShowCurve*[NUM_CURVES];
    for(unsigned int i=0; i<NUM_CURVES; i++)
        m_curve[i] = NULL;
#ifdef AI_DEBUG_CIRCLES
    m_curve[CURVE_PREDICT1]  = new ShowCurve(0.05f, 0.5f,
                                   irr::video::SColor(128,   0,   0, 128));
#endif
#ifdef AI_DEBUG_KART_HEADING
    irr::video::SColor c;
    c = irr::video::SColor(128,   0,   0, 128);
    m_curve[CURVE_KART]      = new ShowCurve(0.5f, 0.5f, c);
#endif
#ifdef AI_DEBUG_NEW_FIND_NON_CRASHING
    m_curve[CURVE_LEFT]      = new ShowCurve(0.5f, 0.5f,
                                            video::SColor(128, 128,   0,   0));
    m_curve[CURVE_RIGHT]     = new ShowCurve(0.5f, 0.5f,
                                            video::SColor(128,   0, 128,   0));
#endif
    m_curve[CURVE_QG]        = new ShowCurve(0.5f, 0.5f,
                                            video::SColor(128,   0, 128,   0));
#ifdef AI_DEBUG_KART_AIM
    irr::video::SColor c1;
    c1 = irr::video::SColor(128,   0,   0, 128);

    m_curve[CURVE_AIM]       = new ShowCurve(0.5f, 0.5f, c1);
#endif
#endif

}   // SkiddingAI

//-----------------------------------------------------------------------------
/** Destructor, mostly to clean up debug data structures.
 */
SkiddingAI::~SkiddingAI()
{
#ifdef AI_DEBUG
    for(unsigned int i=0; i<3; i++)
        irr_driver->removeNode(m_debug_sphere[i]);
    irr_driver->removeNode(m_item_sphere);
    for(unsigned int i=0; i<NUM_CURVES; i++)
    {
        delete m_curve[i];
    }
    delete [] m_curve;
#endif
}   // ~SkiddingAI

//-----------------------------------------------------------------------------
/** Resets the AI when a race is restarted.
 */
void SkiddingAI::reset()
{
    m_time_since_last_shot       = 0.0f;
    m_start_kart_crash_direction = 0;
    m_start_delay                = -1;
    m_time_since_stuck           = 0.0f;
    m_kart_ahead                 = NULL;
    m_distance_ahead             = 0.0f;
    m_distance_leader            = 999.9f;
    m_kart_behind                = NULL;
    m_distance_behind            = 0.0f;
    m_current_curve_radius       = 0.0f;
    m_curve_center               = Vec3(0,0,0);
    m_current_track_direction    = DriveNode::DIR_STRAIGHT;
    m_item_to_collect            = NULL;
    m_last_direction_node        = 0;
    m_avoid_item_close           = false;
    m_skid_probability_state     = SKID_PROBAB_NOT_YET;
    m_last_item_random           = NULL;
    m_burster                    = false;

    AIBaseLapController::reset();
    m_track_node               = Graph::UNKNOWN_SECTOR;
    DriveGraph::get()->findRoadSector(m_kart->getXYZ(), &m_track_node);
    if(m_track_node==Graph::UNKNOWN_SECTOR)
    {
        Log::error(getControllerName().c_str(),
                   "Invalid starting position for '%s' - not on track"
                   " - can be ignored.",
                   m_kart->getIdent().c_str());
        m_track_node = DriveGraph::get()->findOutOfRoadSector(m_kart->getXYZ());
    }

    AIBaseLapController::reset();
}   // reset

//-----------------------------------------------------------------------------
/** Returns a name for the AI.
 *  This is used in profile mode when comparing different AI implementations
 *  to be able to distinguish them from each other.
 */
const irr::core::stringw& SkiddingAI::getNamePostfix() const
{
    // Static to avoid returning the address of a temporary string.
    static irr::core::stringw name="(default)";
    return name;
}   // getNamePostfix

//-----------------------------------------------------------------------------
/** Returns the pre-computed successor of a graph node.
 *  \param index The index of the graph node for which the successor
 *               is searched.
 */
unsigned int SkiddingAI::getNextSector(unsigned int index)
{
    return m_successor_index[index];
}   // getNextSector

//-----------------------------------------------------------------------------
/** This is the main entry point for the AI.
 *  It is called once per frame for each AI and determines the behaviour of
 *  the AI, e.g. steering, accelerating/braking, firing.
 */
void SkiddingAI::update(int ticks)
{
    float dt = stk_config->ticks2Time(ticks);

    // Clear stored items if they were deleted (for example a switched nitro)
    if (m_item_to_collect &&
        !m_item_manager->itemExists(m_item_to_collect))
        m_item_to_collect = NULL;
    if (m_last_item_random &&
        !m_item_manager->itemExists(m_last_item_random))
        m_last_item_random = NULL;

    m_controls->setRescue(false);

    // This is used to enable firing an item backwards.
    m_controls->setLookBack(false);
    m_controls->setNitro(false);

    // Don't do anything if there is currently a kart animations shown.
    if(m_kart->getKartAnimation())
        return;

    if (m_superpower == RaceManager::SUPERPOWER_NOLOK_BOSS)
    {
        if (m_kart->getPowerup()->getType()==PowerupManager::POWERUP_NOTHING)
        {
            if (m_kart->getPosition() > 1)
            {
                int r = rand() % 5;
                if (r == 0 || r == 1)
                    m_kart->setPowerup(PowerupManager::POWERUP_ZIPPER, 1);
                else if (r == 2 || r == 3)
                    m_kart->setPowerup(PowerupManager::POWERUP_BUBBLEGUM, 1);
                else
                    m_kart->setPowerup(PowerupManager::POWERUP_SWATTER, 1);
            }
            else if (m_kart->getAttachment()->getType() == Attachment::ATTACH_SWATTER)
            {
                int r = rand() % 4;
                if (r < 3)
                    m_kart->setPowerup(PowerupManager::POWERUP_BUBBLEGUM, 1);
                else
                    m_kart->setPowerup(PowerupManager::POWERUP_BOWLING, 1);
            }
            else
            {
                int r = rand() % 5;
                if (r == 0 || r == 1)
                    m_kart->setPowerup(PowerupManager::POWERUP_BUBBLEGUM, 1);
                else if (r == 2 || r == 3)
                    m_kart->setPowerup(PowerupManager::POWERUP_SWATTER, 1);
                else
                    m_kart->setPowerup(PowerupManager::POWERUP_BOWLING, 1);
            }

            // also give him some free nitro
            if (RaceManager::get()->getDifficulty() == RaceManager::DIFFICULTY_MEDIUM)
            {
                if (m_kart->getPosition() > 1)
                    m_kart->setEnergy(m_kart->getEnergy() + 2);
                else
                    m_kart->setEnergy(m_kart->getEnergy() + 1);
            }
            else if (RaceManager::get()->getDifficulty() == RaceManager::DIFFICULTY_HARD ||
                RaceManager::get()->getDifficulty() == RaceManager::DIFFICULTY_BEST)
            {
                if (m_kart->getPosition() > 1)
                    m_kart->setEnergy(m_kart->getEnergy() + 7);
                else
                    m_kart->setEnergy(m_kart->getEnergy() + 4);
            }
        }
    }

    // Having a non-moving AI can be useful for debugging, e.g. aiming
    // or slipstreaming.
#undef AI_DOES_NOT_MOVE_FOR_DEBUGGING
#ifdef AI_DOES_NOT_MOVE_FOR_DEBUGGING
    m_controls->setAccel(0);
    m_controls->setSteer(0);
    return;
#endif

    // If the kart needs to be rescued, do it now (and nothing else)
    if(isStuck() && !m_kart->getKartAnimation())
    {
        // For network AI controller
        if (m_enabled_network_ai)
            m_controls->setRescue(true);
        else
            RescueAnimation::create(m_kart);
        AIBaseLapController::update(ticks);
        return;
    }

    if( m_world->isStartPhase() )
    {
        handleRaceStart();
        AIBaseLapController::update(ticks);
        return;
    }

    // Get information that is needed by more than 1 of the handling funcs
    computeNearestKarts();

    if (!m_enabled_network_ai)
    {
        int num_ai = m_world->getNumKarts() - RaceManager::get()->getNumPlayers();
        int position_among_ai = m_kart->getPosition() - m_num_players_ahead;
        // Karts with boosted AI get a better speed cap value
        if (m_kart->getBoostAI())
            position_among_ai = 1;
        float speed_cap = m_ai_properties->getSpeedCap(m_distance_to_player,
            position_among_ai, num_ai);
        m_kart->setSlowdown(MaxSpeed::MS_DECREASE_AI,
            speed_cap, /*fade_in_time*/0);
    }

    //Detect if we are going to crash with the track and/or kart
    checkCrashes(m_kart->getXYZ());
    determineTrackDirection();

    /*Response handling functions*/
    handleAccelerationAndBraking(ticks);
    handleSteering(dt);
    handleRescue(dt);

    // Make sure that not all AI karts use the zipper at the same
    // time in time trial at start up, so disable it during the 5 first seconds
    if(RaceManager::get()->isTimeTrialMode() && (m_world->getTime()<5.0f) )
        m_controls->setFire(false);

    /*And obviously general kart stuff*/
    AIBaseLapController::update(ticks);
}   // update

//-----------------------------------------------------------------------------
/** Decides in which direction to steer. If the kart is off track, it will
 *  steer towards the center of the track. Otherwise it will call one of
 *  the findNonCrashingPoint() functions to determine a point to aim for. Then
 *  it will evaluate items to see if it should aim for any items or try to
 *  avoid item, and potentially adjust the aim-at point, before computing the
 *  steer direction to arrive at the currently aim-at point.
 *  \param dt Time step size.
 */
void SkiddingAI::handleSteering(float dt)
{
    // Special behaviour if we have a bomb attached: try to hit the kart ahead
    // of us.
    if(m_ai_properties->m_handle_bomb &&
        m_kart->getAttachment()->getType()==Attachment::ATTACH_BOMB)
    {
        //TODO : add logic to allow an AI kart to pass the bomb to a kart
        //       close behind by slowing/steering slightly
        // If we are close enough and can pass the bomb, try to hit this kart
        if ( m_kart_ahead != m_kart->getAttachment()->getPreviousOwner() &&
            m_distance_ahead<=10)
        {
            Vec3 target = m_kart_ahead->getXYZ();

            // If we are faster, try to predict the point where we will hit
            // the other kart
            if((m_kart_ahead->getSpeed() < m_kart->getSpeed()) &&
                !m_kart_ahead->isGhostKart())
            {
                float time_till_hit = m_distance_ahead
                                    / (m_kart->getSpeed()-m_kart_ahead->getSpeed());
                target += m_kart_ahead->getVelocity()*time_till_hit;
            }
            float steer_angle = steerToPoint(target);
            setSteering(steer_angle, dt);
            return;
        }
    }

    const int next = m_next_node_index[m_track_node];

    float steer_angle = 0.0f;

    /*The AI responds based on the information we just gathered, using a
     *finite state machine.
     */
    //Reaction to being outside of the road
    float side_dist =
        m_world->getDistanceToCenterForKart( m_kart->getWorldKartId() );

    int item_skill = computeSkill(ITEM_SKILL);

    if( fabsf(side_dist)  >
       0.5f* DriveGraph::get()->getNode(m_track_node)->getPathWidth()+0.5f )
    {
        steer_angle = steerToPoint(DriveGraph::get()->getNode(next)
                                                   ->getCenter());

#ifdef AI_DEBUG
        m_debug_sphere[0]->setPosition(DriveGraph::get()->getNode(next)
                         ->getCenter().toIrrVector());
        Log::debug(getControllerName().c_str(),
                   "Outside of road: steer to center point.");
#endif
    }
    //If we are going to crash against a kart, avoid it if it doesn't
    //drives the kart out of the road
    //TODO : adds item handling to use a cake if available to
    //open the road
    else if( m_crashes.m_kart != -1 && !m_crashes.m_road )
    {
        //-1 = left, 1 = right, 0 = no crash.
        if( m_start_kart_crash_direction == 1 )
        {
            steer_angle = steerToAngle(next, M_PI*0.5f );
            m_start_kart_crash_direction = 0;
        }
        else if(m_start_kart_crash_direction == -1)
        {
            steer_angle = steerToAngle(next, -M_PI*0.5f);
            m_start_kart_crash_direction = 0;
        }
        else
        {
            if(m_world->getDistanceToCenterForKart( m_kart->getWorldKartId() ) >
               m_world->getDistanceToCenterForKart( m_crashes.m_kart ))
            {
                steer_angle = steerToAngle(next, M_PI*0.5f );
                m_start_kart_crash_direction = 1;
            }
            else
            {
                steer_angle = steerToAngle(next, -M_PI*0.5f );
                m_start_kart_crash_direction = -1;
            }
        }

#ifdef AI_DEBUG
        Log::debug(getControllerName().c_str(),
                   "Velocity vector crashes with kart "
                   "and doesn't crashes with road : steer 90 "
                   "degrees away from kart.");
#endif

    }
    else
    {
        m_start_kart_crash_direction = 0;
        Vec3 aim_point;
        int last_node = Graph::UNKNOWN_SECTOR;

        switch(m_point_selection_algorithm)
        {
        case PSA_NEW:    findNonCrashingPointNew(&aim_point, &last_node);
                         break;
        case PSA_DEFAULT:findNonCrashingPoint(&aim_point, &last_node);
                         break;
        }
#ifdef AI_DEBUG
        m_debug_sphere[m_point_selection_algorithm]->setPosition(aim_point.toIrrVector());
#endif
#ifdef AI_DEBUG_KART_AIM
        const Vec3 eps(0,0.5f,0);
        m_curve[CURVE_AIM]->clear();
        m_curve[CURVE_AIM]->addPoint(m_kart->getXYZ()+eps);
        m_curve[CURVE_AIM]->addPoint(aim_point);
#endif

        //Manage item utilisation
        handleItems(dt, &aim_point, last_node, item_skill);

        // Potentially adjust the point to aim for in order to either
        // aim to collect item, or steer to avoid a bad item.
        if(m_ai_properties->m_collect_avoid_items && m_kart->getBlockedByPlungerTicks()<=0)
            handleItemCollectionAndAvoidance(&aim_point, last_node);

        steer_angle = steerToPoint(aim_point);
    }  // if m_current_track_direction!=LEFT/RIGHT

    setSteering(steer_angle, dt);
}   // handleSteering

//-----------------------------------------------------------------------------
/** Decides if the currently selected aim at point (as determined by
 *  handleSteering) should be changed in order to collect/avoid an item.
 *  There are 5 potential phases:
 *  1. Collect all items close by and sort them by items-to-avoid and
 *     items-to-collect. 'Close by' are all items between the current
 *     graph node the kart is on and the graph node the aim_point is on.
 *     The function evaluateItems() filters those items: atm all items-to-avoid
 *     are collected, and all items-to-collect that are not too far away from
 *     the intended driving direction (i.e. don't require a too sharp steering
 *     change).
 *  2. If a pre-selected item (see phase 5) exists, and items-to-avoid which
 *     might get hit if the pre-selected item is collected, the pre-selected
 *     item is unselected. This can happens if e.g. items-to-avoid are behind
 *     the pre-selected items on a different graph node and were therefore not
 *     evaluated then the now pre-selected item was selected initially.
 *  3. If a pre-selected item exists, the kart will steer towards it. The AI
 *     does a much better job of collecting items if after selecting an item
 *     it tries to collect this item even if it doesn't fulfill the original
 *     conditions to be selected in the first place anymore. Example: An item
 *     was selected to be collected because the AI was hitting it anyway. Then
 *     the aim_point changes, and the selected item is not that close anymore.
 *     In many cases it is better to keep on aiming for the items (otherwise
 *     the aiming will not have much benefit and mostly only results in
 *     collecting items that are on long straights).
 *     At this stage because of phase 2) it is certain that no item-to-avoid
 *     will be hit. The function handleSelectedItem() evaluates if it is still
 *     feasible to collect them item (if the kart has already passed the item
 *     it won't reverse to collect it). If the item is still to be aimed for,
 *     adjust the aim_point and return.
 *  4. Make sure to avoid any items-to-avoid. The function steerToAvoid
 *     selects a new aim point if otherwise an item-to-avoid would be hit.
 *     If this is the case, aim_point is adjused and control returns to the
 *     caller.
 *  5. Try to collect an item-to-collect. Select the closest item to the
 *     kart (which in case of a row of items will be the item the kart
 *     is roughly driving towards to anyway). It is then tested if the kart
 *     would hit any item-to-avoid when driving towards this item - if so
 *     the driving direction is not changed and the function returns.
 *     Otherwise, i.e. no items-to-avoid will be hit, it is evaluated if
 *     the kart is (atm) going to hit it anyway. In this case, the item is
 *     selected (see phase 2 and 3). If on the other hand the item is not
 *     too far our of the way, aim_point is adjusted to drive towards
 *     this item (but it is not selected, so the item will be discarded if
 *     the kart is getting too far away from it). Ideally later as the
 *     kart is steering towards this item the item will be selected.
 *
 *  \param aim_point Currently selected point to aim at. If the AI should
 *         try to collect an item, this value will be changed.
 *  \param last_node Index of the graph node on which the aim_point is.
 */
void SkiddingAI::handleItemCollectionAndAvoidance(Vec3 *aim_point,
                                                  int last_node)
{
#ifdef AI_DEBUG
    m_item_sphere->setVisible(false);
#endif
    // Angle to aim_point
    Vec3 kart_aim_direction = *aim_point - m_kart->getXYZ();

    // Make sure we have a valid last_node
    if(last_node==Graph::UNKNOWN_SECTOR)
        last_node = m_next_node_index[m_track_node];

    int node = m_track_node;
    float distance = 0;
    std::vector<const ItemState *> items_to_collect;
    std::vector<const ItemState *> items_to_avoid;

    // 1) Filter and sort all items close by
    // -------------------------------------
    const float max_item_lookahead_distance = 30.f;
    while(distance < max_item_lookahead_distance)
    {
        int n_index= DriveGraph::get()->getNode(node)->getIndex();
        const std::vector<ItemState*> &items_ahead =
                                  m_item_manager->getItemsInQuads(n_index);
        for(unsigned int i=0; i<items_ahead.size(); i++)
        {
            evaluateItems(items_ahead[i],  kart_aim_direction,
                          &items_to_avoid, &items_to_collect);
        }   // for i<items_ahead;
        distance += DriveGraph::get()->getDistanceToNext(node,
                                                      m_successor_index[node]);
        node = m_next_node_index[node];
        // Stop when we have reached the last quad
        if(node==last_node) break;
    }   // while (distance < max_item_lookahead_distance)

    m_avoid_item_close = items_to_avoid.size()>0;

    core::line3df line_to_target_3d((*aim_point).toIrrVector(),
                                     m_kart->getXYZ().toIrrVector());

    // 2) If the kart is aiming for an item, but (suddenly) detects
    //    some close-by items to avoid (e.g. behind the item, which was too
    //    far away to be considered earlier, or because the item was switched
    //    to a bad item), the kart cancels collecting the item if this could
    //    cause the item-to-avoid to be collected.
    // --------------------------------------------------------------------
    if(m_item_to_collect && items_to_avoid.size()>0)
    {
        for(unsigned int i=0; i< items_to_avoid.size(); i++)
        {
            Vec3 d = items_to_avoid[i]->getXYZ()-m_item_to_collect->getXYZ();
            if( d.length2()>m_ai_properties->m_bad_item_closeness_2)
                continue;
            // It could make sense to also test if the bad item would
            // actually be hit, not only if it is close (which can result
            // in false positives, i.e. stop collecting an item though
            // it is not actually necessary). But in (at least) one case
            // steering after collecting m_item_to_collect causes it to then
            // collect the bad item (it's too close to avoid it at that
            // time). So for now here is no additional test, if we see
            // false positives we can handle it here.
            m_item_to_collect = NULL;
            break;
        }   // for i<items_to_avoid.size()
    }   // if m_item_to_collect && items_to_avoid.size()>0


    // 3) Steer towards a pre-selected item
    // -------------------------------------
    if(m_item_to_collect)
    {
        if(handleSelectedItem(kart_aim_direction, aim_point))
        {
            // Still aim at the previsouly selected item.
            *aim_point = m_item_to_collect->getXYZ();
#ifdef AI_DEBUG
            m_item_sphere->setVisible(true);
            m_item_sphere->setPosition(aim_point->toIrrVector());
#endif
            return;
        }

        if(m_ai_debug)
            Log::debug(getControllerName().c_str(), "%s unselects item.",
                       m_kart->getIdent().c_str());
        // Otherwise remove the pre-selected item (and start
        // looking for a new item).
        m_item_to_collect = NULL;
    }   // m_item_to_collect

    // 4) Avoid items-to-avoid
    // -----------------------
    if(items_to_avoid.size()>0)
    {
        // If we need to steer to avoid an item, this takes priority,
        // ignore items to collect and return the new aim_point.
        if(steerToAvoid(items_to_avoid, line_to_target_3d, aim_point))
        {
#ifdef AI_DEBUG
            m_item_sphere->setVisible(true);
            m_item_sphere->setPosition(aim_point->toIrrVector());
#endif
            return;
        }
    }

    // 5) We are aiming for a new item. If necessary, determine
    // randomly if this item sshould actually be collected.
    // --------------------------------------------------------
    if(items_to_collect.size()>0)
    {
        if(items_to_collect[0] != m_last_item_random)
        {
            int p = (int)(100.0f*m_ai_properties->
                          getItemCollectProbability(m_distance_to_player));
            m_really_collect_item = m_random_collect_item.get(100)<p;
            m_last_item_random = items_to_collect[0];
        }
        if(!m_really_collect_item)
        {
            // The same item was selected previously, but it was randomly
            // decided not to collect it - so keep on ignoring this item.
            return;
        }
    }

    // Reset the probability if a different (or no) item is selected.
    if(items_to_collect.size()==0 || items_to_collect[0]!=m_last_item_random)
        m_last_item_random = NULL;

    // 6) Try to aim for items-to-collect
    // ----------------------------------
    if(items_to_collect.size()>0)
    {
        const ItemState *item_to_collect = items_to_collect[0];
        // Test if we would hit a bad item when aiming at this good item.
        // If so, don't change the aim. In this case it has already been
        // ensured that we won't hit the bad item (otherwise steerToAvoid
        // would have detected this earlier).
        if(!hitBadItemWhenAimAt(item_to_collect, items_to_avoid))
        {
            // If the item is hit (with the current steering), it means
            // it's on a good enough driveline, so make this item a permanent
            // target. Otherwise only try to get closer (till hopefully this
            // item s on our driveline)
            if(item_to_collect->hitLine(line_to_target_3d, m_kart))
            {
#ifdef AI_DEBUG
                m_item_sphere->setVisible(true);
                m_item_sphere->setPosition(item_to_collect->getXYZ()
                                                           .toIrrVector());
#endif
                if(m_ai_debug)
                    Log::debug(getControllerName().c_str(),
                               "%s selects item type '%d'.",
                               m_kart->getIdent().c_str(),
                               item_to_collect->getType());
                m_item_to_collect = item_to_collect;
            }
            else
            {
                // Kart will not hit item, try to get closer to this item
                // so that it can potentially become a permanent target.
                const Vec3& xyz = item_to_collect->getXYZ();
                float angle_to_item = (xyz - m_kart->getXYZ())
                    .angle(kart_aim_direction);
                float angle = normalizeAngle(angle_to_item);

                if(fabsf(angle) < 0.3)
                {
                    *aim_point = item_to_collect->getXYZ();
#ifdef AI_DEBUG
                    m_item_sphere->setVisible(true);
                    m_item_sphere->setPosition(item_to_collect->getXYZ()
                                                               .toIrrVector());
#endif
                    if(m_ai_debug)
                        Log::debug(getControllerName().c_str(),
                                   "%s adjusts to hit type %d angle %f.",
                                   m_kart->getIdent().c_str(),
                                   item_to_collect->getType(), angle);
                }
                else
                {
                    if(m_ai_debug)
                        Log::debug(getControllerName().c_str(),
                                   "%s won't hit '%d', angle %f.",
                                   m_kart->getIdent().c_str(),
                                   item_to_collect->getType(), angle);
                }
            }   // kart will not hit item
        }   // does hit hit bad item
    }   // if item to consider was found

}   // handleItemCollectionAndAvoidance

//-----------------------------------------------------------------------------
/** Returns true if the AI would hit any of the listed bad items when trying
 *  to drive towards the specified item.
 *  \param item The item the AI is evaluating for collection.
 *  \param items_to_aovid A list of bad items close by. All of these needs
 *        to be avoided.
 *  \return True if it would hit any of the bad items.
*/
bool SkiddingAI::hitBadItemWhenAimAt(const ItemState *item,
                          const std::vector<const ItemState *> &items_to_avoid)
{
    core::line3df to_item(m_kart->getXYZ().toIrrVector(),
                          item->getXYZ().toIrrVector());
    for(unsigned int i=0; i<items_to_avoid.size(); i++)
    {
        if(items_to_avoid[i]->hitLine(to_item, m_kart))
            return true;
    }
    return false;
}   // hitBadItemWhenAimAt

//-----------------------------------------------------------------------------
/** This function is called when the AI is trying to hit an item that is
 *  pre-selected to be collected. The AI only evaluates if it's still
 *  feasible/useful to try to collect this item, or abandon it (and then
 *  look for a new item). At item is unselected if the kart has passed it
 *  (so collecting it would require the kart to reverse).
 *  \pre m_item_to_collect is defined
 *  \param kart_aim_angle The angle towards the current aim_point.
 *  \param aim_point The current aim_point.
 *  \param last_node
 *  \return True if th AI should still aim for the pre-selected item.
 */
bool SkiddingAI::handleSelectedItem(Vec3 kart_aim_direction, Vec3 *aim_point)
{
    // If the item is unavailable keep on testing. It is not necessary
    // to test if an item has turned bad, this was tested before this
    // function is called.
    if(!m_item_to_collect->isAvailable())
        return false;

    const Vec3 &xyz = m_item_to_collect->getXYZ();
    float angle_to_item = (xyz - m_kart->getXYZ()).angle(kart_aim_direction);
    float angle = normalizeAngle(angle_to_item);

    if(fabsf(angle)>1.5)
    {
        // We (likely) have passed the item we were aiming for
        return false;
    }
    else
    {
        // Keep on aiming for last selected item
        *aim_point = m_item_to_collect->getXYZ();
    }
    return true;
}   // handleSelectedItem

//-----------------------------------------------------------------------------
/** Decides if steering is necessary to avoid bad items. If so, it modifies
 *  the aim_point and returns true.
 *  \param items_to_avoid List of items to avoid.
 *  \param line_to_target The 2d line from the current kart position to
 *         the current aim point.
 *  \param aim_point The point the AI is steering towards (not taking items
 *         into account).
 *  \return True if steering is necessary to avoid an item.
 */
bool SkiddingAI::steerToAvoid(const std::vector<const ItemState *> &items_to_avoid,
                              const core::line3df &line_to_target,
                              Vec3 *aim_point)
{
    // First determine the left-most and right-most item.
    float left_most        = items_to_avoid[0]->getDistanceFromCenter();
    float right_most       = items_to_avoid[0]->getDistanceFromCenter();
    int   index_left_most  = 0;
    int   index_right_most = 0;

    for(unsigned int i=1; i<items_to_avoid.size(); i++)
    {
        float dist = items_to_avoid[i]->getDistanceFromCenter();
        if (dist<left_most)
        {
            left_most       = dist;
            index_left_most = i;
        }
        if(dist>right_most)
        {
            right_most       = dist;
            index_right_most = i;
        }
    }

    // Check if we would drive left of the leftmost or right of the
    // rightmost point - if so, nothing to do.
    const Vec3& left = items_to_avoid[index_left_most]->getXYZ();
    int node_index = items_to_avoid[index_left_most]->getGraphNode();
    const Vec3& normal = DriveGraph::get()->getNode(node_index)->getNormal();
    Vec3 hit;
    Vec3 hit_nor(0, 1, 0);
    const Material* m;
    m_track->getPtrTriangleMesh()->castRay(
        Vec3(line_to_target.getMiddle()) + normal,
        Vec3(line_to_target.getMiddle()) + normal * -10000, &hit, &m,
        &hit_nor);
    Vec3 p1 = line_to_target.start,
         p2 = line_to_target.getMiddle() + hit_nor.toIrrVector(),
         p3 = line_to_target.end;

    int item_index = -1;
    bool is_left    = false;

    // >=0 means the point is to the right of the line, or the line is
    // to the left of the point.
    if(left.sideofPlane(p1,p2,p3) <= 0)
    {
        // Left of leftmost point
        item_index = index_left_most;
        is_left       = true;
    }
    else
    {
        const Vec3& right = items_to_avoid[index_right_most]->getXYZ();
        if (right.sideofPlane(p1, p2, p3) >= 0)
        {
            // Right of rightmost point
            item_index = index_right_most;
            is_left    = false;
        }
    }
    if(item_index>-1)
    {
        // Even though we are on the side, we must make sure
        // that we don't hit that item
        // If we don't hit the item on the side, no more tests are necessary
        if(!items_to_avoid[item_index]->hitLine(line_to_target, m_kart))
            return false;

        // See if we can avoid this item by driving further to the side
        const Vec3 *avoid_point = items_to_avoid[item_index]
                                ->getAvoidancePoint(is_left);
        // If avoid_point is NULL, it means steering more to the sides
        // brings us off track. In this case just try to steer to the
        // other side (e.g. when hitting a left-most item and the kart can't
        // steer further left, steer a bit to the right of the left-most item
        // (without further tests if we might hit anything else).
        if(!avoid_point)
            avoid_point = items_to_avoid[item_index]
                        ->getAvoidancePoint(!is_left);
        *aim_point = *avoid_point;
        return true;
    }

    // At this stage there must be at least two items - if there was
    // only a single item, the 'left of left-most' or 'right of right-most'
    // tests above are had been and an appropriate steering point was already
    // determined.

    // Try to identify two items we are driving between (if the kart is not
    // driving between two items, one of the 'left of left-most' etc.
    // tests before applied and this point would not be reached).

    float min_distance[2] = {99999.9f, 99999.9f};
    int   index[2] = {-1, -1};
    core::vector3df closest3d[2];
    for(unsigned int i=0; i<items_to_avoid.size(); i++)
    {
        const Vec3 &xyz         = items_to_avoid[i]->getXYZ();
        core::vector3df point3d = line_to_target.getClosestPoint(xyz.toIrrVector());
        float d = (xyz.toIrrVector() - point3d).getLengthSQ();
        float direction = xyz.sideofPlane(p1,p2,p3);
        int ind = direction<0 ? 1 : 0;
        if(d<min_distance[ind])
        {
            min_distance[ind] = d;
            index[ind]        = i;
            closest3d[ind]    = point3d;
        }
    }

    assert(index[0]!= index[1]);
    assert(index[0]!=-1       );
    assert(index[1]!=-1       );

    // We are driving between item_to_avoid[index[0]] and ...[1].
    // If we don't hit any of them, just keep on driving as normal
    bool hit_left  = items_to_avoid[index[0]]->hitKart(closest3d[0], m_kart);
    bool hit_right = items_to_avoid[index[1]]->hitKart(closest3d[1], m_kart);
    if( !hit_left && !hit_right)
        return false;

    // If we hit the left item, aim at the right avoidance point
    // of the left item. We might still hit the right item ... this might
    // still be better than going too far off track
    if(hit_left)
    {
        *aim_point =
            *(items_to_avoid[index[0]]->getAvoidancePoint(/*left*/false));
        return true;
    }

    // Now we must be hitting the right item, so try aiming at the left
    // avoidance point of the right item.
    *aim_point = *(items_to_avoid[index[1]]->getAvoidancePoint(/*left*/true));
    return true;
}   // steerToAvoid

//-----------------------------------------------------------------------------
/** This subroutine decides if the specified item should be collected,
 *  avoided, or ignored. It can potentially use the state of the
 *  kart to make this decision, e.g. depending on what item the kart currently
 *  has, how much nitro it has etc. Though atm it picks the first good item,
 *  and tries to avoid any bad items on the track.
 *  \param item The item which is considered for picking/avoidance.
 *  \param kart_aim_angle The angle of the line from the kart to the aim point.
 *         If aim_angle==kart_heading then the kart is driving towards the
 *         item.
 *  \param item_to_avoid A pointer to a previously selected item to avoid
 *         (NULL if no item was avoided so far).
 *  \param item_to_collect A pointer to a previously selected item to collect.
 */
void SkiddingAI::evaluateItems(const ItemState *item, Vec3 kart_aim_direction,
                               std::vector<const ItemState *> *items_to_avoid,
                               std::vector<const ItemState *> *items_to_collect)
{
    const KartProperties *kp = m_kart->getKartProperties();

    // Ignore items that are currently disabled
    if(!item->isAvailable()) return;

    // If the item type is not handled here, ignore it
    Item::ItemType type = item->getType();
    if( type!=Item::ITEM_BANANA    && type!=Item::ITEM_BUBBLEGUM &&
        type!=Item::ITEM_BONUS_BOX &&
        type!=Item::ITEM_NITRO_BIG && type!=Item::ITEM_NITRO_SMALL  )
        return;

    bool avoid = false;
    switch(type)
    {
        // Negative items: avoid them
        case Item::ITEM_BUBBLEGUM: // fallthrough
        case Item::ITEM_BANANA: avoid = true;  break;

        // Positive items: try to collect
        case Item::ITEM_NITRO_BIG:
            // Only collect nitro, if it can actually be stored.
            if (m_kart->getEnergy() + kp->getNitroBigContainer()
                > kp->getNitroMax())
                  return;
            break;
        case Item::ITEM_NITRO_SMALL:
            // Only collect nitro, if it can actually be stored.
            if (m_kart->getEnergy() + kp->getNitroSmallContainer()
                > kp->getNitroMax())
                  return;
            break;
        case Item::ITEM_BONUS_BOX:
            break;
        default: assert(false); break;
    }    // switch


    // Ignore items to be collected that are out of our way (though all items
    // to avoid are collected).
    if(!avoid)
    {
        const Vec3 &xyz = item->getXYZ();
        float angle_to_item =
            (xyz - m_kart->getXYZ()).angle(kart_aim_direction);
        float diff = normalizeAngle(angle_to_item);

        // The kart is driving at high speed, when the current max speed
        // is higher than the max speed of the kart (which is caused by
        // any powerups etc)
        // Otherwise check for skidding. If the kart is still collecting
        // skid bonus, currentMaxSpeed is not affected yet, but it might
        // be if the kart would need to turn sharper, therefore stops
        // skidding, and will get the bonus speed.
        bool high_speed = (m_kart->getCurrentMaxSpeed() >
                           kp->getEngineMaxSpeed() ) ||
                          m_kart->getSkidding()->getSkidBonusReady();
        float max_angle = high_speed
                        ? m_ai_properties->m_max_item_angle_high_speed
                        : m_ai_properties->m_max_item_angle;

        if(fabsf(diff) > max_angle)
            return;

    }   // if !avoid

    // Now insert the item into the sorted list of items to avoid
    // (or to collect). The lists are (for now) sorted by distance
    std::vector<const ItemState*> *list;
    if(avoid)
        list = items_to_avoid;
    else
        list = items_to_collect;

    float new_distance = (item->getXYZ() - m_kart->getXYZ()).length2();

    // This list is usually very short, so use a simple bubble sort
    list->push_back(item);
    int i;
    for(i=(int)list->size()-2; i>=0; i--)
    {
        float d = ((*list)[i]->getXYZ() - m_kart->getXYZ()).length2();
        if(d<=new_distance)
        {
            break;
        }
        (*list)[i+1] = (*list)[i];
    }
    (*list)[i+1] = item;

}   // evaluateItems

//-----------------------------------------------------------------------------
/** Handle all items depending on the chosen strategy.
 *  Level 0 "AI" : do nothing (not used by default)
 *  Level 1 "AI" : use items after a random time
 *  Level 2 to 5 AI : strategy detailed before each item
 *  Each successive level is overall stronger (5 the strongest, 2 the weakest of
 *  non-random strategies), but two levels may share a strategy for a given item.
 *  \param dt Time step size.
 *  STATE: shield on -> avoid usage of offensive items (with certain tolerance)
 *  STATE: swatter on -> avoid usage of shield
 */
void SkiddingAI::handleItems(const float dt, const Vec3 *aim_point, int last_node, int item_skill)
{
    m_controls->setFire(false);
    if(m_kart->getKartAnimation() ||
        m_kart->getPowerup()->getType() == PowerupManager::POWERUP_NOTHING )
        return;

    m_time_since_last_shot += dt;
   
    //time since last shot is meant to avoid using the same item
    //several times in rapid succession ; not to wait to use a useful
    //collected item
    if ( m_kart->getPowerup()->getType() != m_kart->getLastUsedPowerup() )
    {
       m_time_since_last_shot = 50.0f; //The AI may wait if the value is low, so set a high value
    }
   
    if (m_superpower == RaceManager::SUPERPOWER_NOLOK_BOSS)
    {
        m_controls->setLookBack(m_kart->getPowerup()->getType() ==
                                   PowerupManager::POWERUP_BOWLING   );

        if( m_time_since_last_shot > 3.0f )
        {
            m_controls->setFire(true);
            if (m_kart->getPowerup()->getType() == PowerupManager::POWERUP_SWATTER)
                m_time_since_last_shot = 3.0f;
            else
            {
                // to make things less predictable :)
                m_time_since_last_shot = (rand() % 1000) / 1000.0f * 3.0f - 2.0f;
            }
        }
        else
        {
            m_controls->setFire(false);
        }
        return;
    }

    // Tactic 0: don't use item
    // -----------------------------------------
    if(item_skill == 0)
    {
        return;
    }

    // Tactic 1: wait between 5 and 10 seconds, then use item
    // ------------------------------------------------------
    if(item_skill == 1)
    {
        int random_t = 0;
        random_t = m_random_skid.get(6); //Reuse the random skid generator
        random_t = random_t + 5;
          
        if( m_time_since_last_shot > random_t )
        {
            m_controls->setFire(true);
            m_time_since_last_shot = 0.0f;
        }
        return;
    }

    // Tactics 2 to 5: calculate
    // -----------------------------------------
    float min_bubble_time = 2.0f;

    // Preparing item list for item aware actions

    // Angle to aim_point
    Vec3 kart_aim_direction = *aim_point - m_kart->getXYZ();

    // Make sure we have a valid last_node
    if(last_node==Graph::UNKNOWN_SECTOR)
        last_node = m_next_node_index[m_track_node];

    int node = m_track_node;
    float distance = 0;
    std::vector<const ItemState *> items_to_collect;
    std::vector<const ItemState *> items_to_avoid;

    // 1) Filter and sort all items close by
    // -------------------------------------
    const float max_item_lookahead_distance = 20.0f;
    while(distance < max_item_lookahead_distance)
    {
        int n_index= DriveGraph::get()->getNode(node)->getIndex();
        const std::vector<ItemState *> &items_ahead =
            m_item_manager->getItemsInQuads(n_index);
        for(unsigned int i=0; i<items_ahead.size(); i++)
        {
            evaluateItems(items_ahead[i],  kart_aim_direction,
                          &items_to_avoid, &items_to_collect);
        }   // for i<items_ahead;
        distance += DriveGraph::get()->getDistanceToNext(node,
                                                      m_successor_index[node]);
        node = m_next_node_index[node];
        // Stop when we have reached the last quad
        if(node==last_node) break;
    }   // while (distance < max_item_lookahead_distance)

    //items_to_avoid and items_to_collect now contain the closest item information needed after
    //What matters is (a) if the lists are void ; (b) if they are not, what kind of item it is
   
    switch( m_kart->getPowerup()->getType() )
    {
    case PowerupManager::POWERUP_BUBBLEGUM:
        {
            handleBubblegum(item_skill, items_to_collect, items_to_avoid);
            break;
        } // POWERUP_BUBBLEGUM
          
    case PowerupManager::POWERUP_CAKE:
        {
            // if the kart has a shield, do not break it by using a cake.
            if((m_kart->getShieldTime() > min_bubble_time) && (stk_config->m_shield_restrict_weapons == true))
                break;

            handleCake(item_skill);
            break;
        }   // POWERUP_CAKE
          
    case PowerupManager::POWERUP_BOWLING:
        {
            // if the kart has a shield, do not break it by using a bowling ball.
            if((m_kart->getShieldTime() > min_bubble_time) && (stk_config->m_shield_restrict_weapons == true))
                break;

            handleBowling(item_skill);
            break;
        }   // POWERUP_BOWLING

    case PowerupManager::POWERUP_ZIPPER:
        // Do nothing. Further up a zipper is used if nitro should be selected,
        // saving the (potential more valuable nitro) for later
        break;   // POWERUP_ZIPPER

    case PowerupManager::POWERUP_PLUNGER:
        {
            // if the kart has a shield, do not break it by using a plunger.
            if((m_kart->getShieldTime() > min_bubble_time) && (stk_config->m_shield_restrict_weapons == true))
                break;

            // Leave more time after a plunger, since it will take some
            // time before a plunger effect becomes obvious.
            if(m_time_since_last_shot < 5.0f) break;

            // Plungers can be fired backwards and are faster,
            // so allow more distance for shooting.
            bool fire_backwards = (m_kart_behind && m_kart_ahead &&
                                   m_distance_behind < m_distance_ahead) ||
                                  !m_kart_ahead;
            float distance      = fire_backwards ? m_distance_behind
                                                 : m_distance_ahead;
            m_controls->setFire(distance < 30.0f                 ||
                                 m_time_since_last_shot > 10.0f    );
            if(m_controls->getFire())
                m_controls->setLookBack(fire_backwards);
            break;
        }   // POWERUP_PLUNGER

    case PowerupManager::POWERUP_SWITCH:
        {
            handleSwitch(item_skill, items_to_collect, items_to_avoid);
            break;
        } // POWERUP_SWITCH

    case PowerupManager::POWERUP_PARACHUTE:
        {
        // Wait one second more than a previous parachute
        if(m_time_since_last_shot > m_kart->getKartProperties()->getParachuteDurationOther() + 1.0f)
            m_controls->setFire(true);
        break;
        }// POWERUP_PARACHUTE

    case PowerupManager::POWERUP_ANVIL:
        break;   // POWERUP_ANVIL

    case PowerupManager::POWERUP_SWATTER:
        {
             // if the kart has a shield, do not break it by using a swatter.
            if(m_kart->getShieldTime() > min_bubble_time)
                break;

            handleSwatter(item_skill);
            break;
        } // POWERUP_SWATTER
    case PowerupManager::POWERUP_RUBBERBALL:
        // if the kart has a shield, do not break it by using a swatter.
        if((m_kart->getShieldTime() > min_bubble_time) && (stk_config->m_shield_restrict_weapons == true))
            break;
        // Perhaps some more sophisticated algorithm might be useful.
        // For now: fire if there is a kart ahead (which means that
        // this kart is certainly not the first kart)
        m_controls->setFire(m_kart_ahead != NULL);
        break;
    default:
        Log::error(getControllerName().c_str(),
                   "Invalid or unhandled powerup '%d' in default AI.",
                   m_kart->getPowerup()->getType());
        assert(false);
    }
    if(m_controls->getFire())  m_time_since_last_shot = 0.0f;
}   // handleItems


//-----------------------------------------------------------------------------
/** Handle bubblegum depending on the chosen strategy
 * Level 2 : Use the shield immediately after a wait time
 * Level 3 : Use the shield against flyables except cakes. Use the shield against bad attachments
 *           and plunger. Use the bubble gum against an enemy close behind, except if holding a swatter.
 * Level 4 : Level 3, and protect against cakes too, and use before hitting gum/banana
 * Level 5 : Level 4, and use before hitting item box, and let plunger hit
 *                   (can use the shield after), and use against bomb only when the timer ends
 *  \param item_skill The skill with which to use the item
 *  \param items_to_collect The list of close good items
 *  \param items_to_avoid The list of close bad items
 */
void SkiddingAI::handleBubblegum(int item_skill,
                                 const std::vector<const ItemState *> &items_to_collect,
                                 const std::vector<const ItemState *> &items_to_avoid)
{
    float shield_radius = m_ai_properties->m_shield_incoming_radius;

    int projectile_types[4]; //[3] basket, [2] cakes, [1] plunger, [0] bowling
    projectile_types[0] = ProjectileManager::get()->getNearbyProjectileCount(m_kart, shield_radius, PowerupManager::POWERUP_BOWLING);
    projectile_types[1] = ProjectileManager::get()->getNearbyProjectileCount(m_kart, shield_radius, PowerupManager::POWERUP_PLUNGER);
    projectile_types[2] = ProjectileManager::get()->getNearbyProjectileCount(m_kart, shield_radius, PowerupManager::POWERUP_CAKE);
    projectile_types[3] = ProjectileManager::get()->getNearbyProjectileCount(m_kart, shield_radius, PowerupManager::POWERUP_RUBBERBALL);
   
    bool projectile_is_close = false;
    projectile_is_close = ProjectileManager::get()->projectileIsClose(m_kart, shield_radius);

    Attachment::AttachmentType type = m_kart->getAttachment()->getType();
    
    if((item_skill == 2) && (m_time_since_last_shot > 2.0f))
    {
        m_controls->setFire(true);
        m_controls->setLookBack(false);
        return;
    }
    
    // Check if a flyable (cake, ...) is close. If so, use bubblegum
    // as shield
    if(item_skill == 3) //don't protect against cakes
    {
       if( !m_kart->isShielded() && projectile_is_close
          && projectile_types[2] == 0)
       {
          //don't discard swatter against plunger
          if( projectile_types[1] == 0
             || (projectile_types[1] >= 1 && type != Attachment::ATTACH_SWATTER))
          {
             m_controls->setFire(true);
             m_controls->setLookBack(false);
             return;
          }
       }
    }
    else if(item_skill == 4)
    {
       if( !m_kart->isShielded() && projectile_is_close)
       {
          //don't discard swatter against plunger
          if( projectile_types[1] == 0
             || (projectile_types[1] >= 1 && type != Attachment::ATTACH_SWATTER))
          {
             m_controls->setFire(true);
             m_controls->setLookBack(false);
             return;
          }
       } 
    }
    else if (item_skill == 5) //don't protect against plungers alone
    {
       if( !m_kart->isShielded() && projectile_is_close)
       {
          if (projectile_types[0] >=1 || projectile_types[2] >=1 || projectile_types[3] >=1 )
          {
             m_controls->setFire(true);
             m_controls->setLookBack(false);
             return;
          }
       }
    }

    // Use shield to remove bad attachments
    if( (type == Attachment::ATTACH_BOMB && item_skill != 5)
      || type == Attachment::ATTACH_PARACHUTE
      || type == Attachment::ATTACH_ANVIL )
    {
        m_controls->setFire(true);
        m_controls->setLookBack(false);
        return;
    }
    //if it is a bomb, wait : we may pass it to another kart before the timer runs out
    if (item_skill == 5 && type == Attachment::ATTACH_BOMB)
    {
        if (m_kart->getAttachment()->getTicksLeft() < stk_config->time2Ticks(2))
        {
            m_controls->setFire(true);
            m_controls->setLookBack(false);
            return;
        }
    }

    //If the kart view is blocked by a plunger, use the shield
    if(m_kart->getBlockedByPlungerTicks()>0)
    {
        m_controls->setFire(true);
        m_controls->setLookBack(false);
        return;
    }
    
    // Use shield if kart is going to hit a bad item (banana or bubblegum)
    if((item_skill == 4) || (item_skill == 5)) 
    {
       if( !m_kart->isShielded() && items_to_avoid.size()>0)
       {
          float d = (items_to_avoid[0]->getXYZ() - m_kart->getXYZ()).length2();
          
          if ((item_skill == 4 && d < 1.5f) || (item_skill == 5 && d < 0.7f))
          {
             m_controls->setFire(true);
             m_controls->setLookBack(false);
             return;
          }
       }
    }
    
    // Use shield if kart is going to hit an item box
    if (item_skill == 5)
    {
       if( !m_kart->isShielded() && items_to_collect.size()>0)
       {
          float d = (items_to_collect[0]->getXYZ() - m_kart->getXYZ()).length2();
          
          if ((items_to_collect[0]->getType() == Item::ITEM_BONUS_BOX) && (d < 0.7f))
          {
             m_controls->setFire(true);
             m_controls->setLookBack(false);
             return;
          }
       }      
    }
      
    // Avoid dropping all bubble gums one after another
    if( m_time_since_last_shot < 2.0f) return;

    // Use bubblegum if the next kart  behind is 'close'
    // and straight behind

    bool straight_behind = false;
    if (m_kart_behind)
    {
        Vec3 behind_lc = m_kart->getTrans().inverse()
            (m_kart_behind->getXYZ());
        const float abs_angle =
            atan2f(fabsf(behind_lc.x()), fabsf(behind_lc.z()));
        if (abs_angle < 0.2f) straight_behind = true;
    }

    if(m_distance_behind < 8.0f && straight_behind &&
       (!m_item_manager->areItemsSwitched() || item_skill < 4))
    {
        m_controls->setFire(true);
        m_controls->setLookBack(true);
        return;
    }
    return;
} //handleBubblegum

//-----------------------------------------------------------------------------
/** Handle cake depending on the chosen strategy
 * Level 2 : Use the cake against any close vulnerable enemy, with priority to those ahead and close,
 *           check if the enemy is roughly ahead.
 * Level 3 : Level 2 and don't fire on slower karts
 * Level 4 : Level 3 and fire if the kart has a swatter which may hit us
 * Level 5 : Level 4 and don't fire on a shielded kart if we're just behind (gum)
 *  \param item_skill The skill with which to use the item
 */
void SkiddingAI::handleCake(int item_skill)
{
    // Leave some time between shots
    if(m_time_since_last_shot<2.0f) return;

    // Do not fire if the target is too far
    bool kart_behind_is_close = m_distance_behind < 25.0f;
    bool kart_ahead_is_close = m_distance_ahead < 20.0f;

    bool kart_ahead_has_gum = false;

    bool straight_ahead = false;
    bool rather_ahead = false;//used to not fire in big curves
    bool rather_behind = false;

    if (m_kart_ahead)
    {
        kart_ahead_has_gum = (m_kart_ahead->getAttachment()->getType()
                                 == Attachment::ATTACH_BUBBLEGUM_SHIELD);

        Vec3 ahead_lc = m_kart->getTrans().inverse()
            (m_kart_ahead->getXYZ());
        const float abs_angle =
            atan2f(fabsf(ahead_lc.x()), fabsf(ahead_lc.z()));
        if (abs_angle < 0.2f) straight_ahead = true;
        if (abs_angle < 0.5f) rather_ahead = true;
    }

    if (m_kart_behind)
    {
        Vec3 behind_lc = m_kart->getTrans().inverse()
            (m_kart_behind->getXYZ());
        const float abs_angle =
            atan2f(fabsf(behind_lc.x()), fabsf(behind_lc.z()));
        if (abs_angle < 0.5f) rather_behind = true;
    }

    // Do not fire if the kart is driving too slow
    bool kart_behind_is_slow =
        (m_kart_behind && m_kart_behind->getSpeed() < m_kart->getSpeed());
    bool kart_ahead_is_slow =
        (m_kart_ahead && m_kart_ahead->getSpeed() < m_kart->getSpeed());

    //Establish which of the target is better if any
    float fire_ahead = 0.0f;
    float fire_behind = 0.0f;

    if (kart_behind_is_close && rather_behind) fire_behind += 25.0f - m_distance_behind;
    else                      fire_behind -= 100.0f;

    if (kart_ahead_is_close && rather_ahead)  fire_ahead += 30.0f - m_distance_ahead; //prefer targetting ahead
    else                      fire_ahead -= 100.0f;

    // Don't fire on an invulnerable kart.
    if (m_kart_behind && m_kart_behind->isInvulnerable())
        fire_behind -= 100.0f;

    if (m_kart_ahead && m_kart_ahead->isInvulnerable())
        fire_ahead -= 100.0f;

    // Don't fire at a kart that is slower than us. Reason is that
    // we can either save the cake for later since we will overtake
    // the kart anyway, or that this might force the kart ahead to
    // use its nitro/zipper (and then we will shoot since then the
    // kart is faster).    
    if(item_skill >= 3)
    {
        if (kart_behind_is_slow) fire_behind -= 50.0f;
        else                     fire_behind += 25.0f;

        if (kart_ahead_is_slow) fire_ahead -= 50.0f;
        else                    fire_ahead += 25.0f;
    }

    //Try to take out a kart which has a swatter in priority
    if (item_skill>=4)
    {
        bool kart_behind_has_swatter = false;
        if (m_kart_behind)
        {
            kart_behind_has_swatter = (m_kart_behind->getAttachment()->getType()
                                       == Attachment::ATTACH_SWATTER);
        }

        bool kart_ahead_has_swatter = false;
        if (m_kart_ahead)
        {
            kart_ahead_has_swatter = (m_kart_ahead->getAttachment()->getType() 
                                      == Attachment::ATTACH_SWATTER);
        }

        //If it is slower, the swatter is more dangerous
        if (kart_ahead_has_swatter)
        {
            if (kart_ahead_is_slow) fire_ahead += 75.0f;
            else                    fire_ahead += 15.0f;
        }
        //If it is slower, we can wait for it to get faster and closer before firing
        if (kart_behind_has_swatter)
        {
            if (!kart_behind_is_slow) fire_behind += 25.0f;
        }
    }

    //Don't fire on a kart straight ahead with a bubblegum shield
    if (item_skill == 5 && kart_ahead_has_gum && straight_ahead)
    {
        fire_ahead -= 100.0f;
    }

    // Compare how interesting each target is to determine if firing backwards or not

    bool fire_backwards = false;

    if (fire_behind > fire_ahead)
    {
        fire_backwards = true;
    }

    bool fire = (fire_backwards && fire_behind > 0) || (!fire_backwards && fire_ahead > 0);

    m_controls->setFire( fire );
    if(m_controls->getFire())
        m_controls->setLookBack(fire_backwards);
    return;

} //handleCake


//-----------------------------------------------------------------------------
/** Handle the bowling ball depending on the chosen strategy
 * Level 2 : Use the bowling ball against enemies straight ahead
             or straight behind, and not invulnerable, with a 5 second delay
 * Level 3 : Only 3 seconds of delay
 * Level 4 : Same as level 3
 * Level 5 : Level 4 and don't fire on a shielded kart if we're just behind (gum) 
 *  \param item_skill The skill with which to use the item
 */
void SkiddingAI::handleBowling(int item_skill)
{
    // Leave more time between bowling balls, since they are
    // slower, so it should take longer to hit something which
    // can result in changing our target.
    if(item_skill == 2 && m_time_since_last_shot < 5.0f) return;
    if(item_skill >= 3 && m_time_since_last_shot < 3.0f) return;

    // Consider angle towards karts
    bool straight_behind = false;
    bool straight_ahead = false;
    if (m_kart_behind)
    {
        Vec3 behind_lc = m_kart->getTrans().inverse()
            (m_kart_behind->getXYZ());
        const float abs_angle =
            atan2f(fabsf(behind_lc.x()), fabsf(behind_lc.z()));
        if (abs_angle < 0.2f) straight_behind = true;
    }
    if (m_kart_ahead)
    {
        Vec3 ahead_lc = m_kart->getTrans().inverse()
            (m_kart_ahead->getXYZ());
        const float abs_angle =
            atan2f(fabsf(ahead_lc.x()), fabsf(ahead_lc.z()));
        if (abs_angle < 0.2f) straight_ahead = true;
    }

    // Don't fire if the kart we are aiming at is invulnerable.
    if (m_kart_behind && m_kart_behind->isInvulnerable())
        straight_behind = false;

    if (m_kart_ahead && m_kart_ahead->isInvulnerable())
        straight_ahead = false;

    //Don't fire on a kart straight ahead with a bubblegum shield
    if (item_skill == 5)
    {
        bool kart_ahead_has_gum = false;

        if (m_kart_ahead)
        {
            kart_ahead_has_gum = (m_kart_ahead->getAttachment()->getType()
                                 == Attachment::ATTACH_BUBBLEGUM_SHIELD);
        }

        if (kart_ahead_has_gum && straight_ahead)
        {
            straight_ahead = false;//disable firing ahead
        }
    }

    //don't fire if there is no vulnerable kart in the line of fire
    if (!straight_behind && !straight_ahead)
    {
        return;
    }

    // Bowling balls are slower, so only fire on closer karts - but when
    // firing backwards, the kart can be further away, since the ball
    // acts a bit like a mine (and the kart is racing towards it, too)
    bool fire_backwards = (straight_behind && !straight_ahead) ||
                          (straight_behind && m_distance_behind < m_distance_ahead);

    float distance = fire_backwards ? m_distance_behind
                                    : m_distance_ahead;
    m_controls->setFire( ( (fire_backwards && distance < 30.0f)  ||
                           (!fire_backwards && distance <10.0f)    ));
    if(m_controls->getFire())
        m_controls->setLookBack(fire_backwards);
    return;
} //handleBowling

//-----------------------------------------------------------------------------
/** Handle the swatter depending on the chosen strategy
 * Level 2 : Use the swatter immediately after a wait time
 * Level 3 : Use the swatter when enemies are close
 * Level 4 : Level 3 and use the swatter to remove bad attachments
 * Level 5 : Level 4 and use against bomb only when the timer ends
 *  \param item_skill The skill with which to use the item
 */
void SkiddingAI::handleSwatter(int item_skill)
{
    Attachment::AttachmentType type = m_kart->getAttachment()->getType();
    
    if((item_skill == 2) && (m_time_since_last_shot > 2.0f))
    {
        m_controls->setFire(true);
        return;
    }
    
    // Use swatter to remove bad attachments
    if((item_skill == 4) || (item_skill == 5))
    {
        if( (type == Attachment::ATTACH_BOMB
             && item_skill == 4)
             || type == Attachment::ATTACH_PARACHUTE
             || type == Attachment::ATTACH_ANVIL )
        {
            m_controls->setFire(true);
            m_controls->setLookBack(false);
            return;
        }
        //if it is a bomb, wait : we may pass it to another kart before the timer runs out
        if (item_skill == 5 && type == Attachment::ATTACH_BOMB)
        {
            if (m_kart->getAttachment()->getTicksLeft() > stk_config->time2Ticks(3))
            {
                m_controls->setFire(true);
                m_controls->setLookBack(false);
                return;
            }
        }
    }
     // Squared distance for which the swatter works
     float d2 = m_kart->getKartProperties()->getSwatterDistance();

     // Fire if the closest kart ahead or to the back is not already
     // squashed and close enough.
     // FIXME: this can be improved on, since more than one kart might
     //        be hit, and a kart ahead might not be at an angle at
     //        which the glove can be used.
     if(  ( m_kart_ahead && !m_kart_ahead->isSquashed()      &&
             (m_kart_ahead->getXYZ()-m_kart->getXYZ()).length2()<d2 &&
             m_kart_ahead->getSpeed() < m_kart->getSpeed()     ) ||
          ( m_kart_behind && !m_kart_behind->isSquashed() &&
             (m_kart_behind->getXYZ()-m_kart->getXYZ()).length2()<d2) )
             m_controls->setFire(true);
     return;
} //handleSwatter

//-----------------------------------------------------------------------------
/** Handle switch depending on the chosen strategy
 * Level 2 : Use the switch after a wait time
 * Level 3 : Same as level 2 but don't fire if close to a good item
 * Level 4 : Same as level 3 and fire if very close to a bad item
 * Level 5 : Use if it makes a better item available, or if very close
 *           to a bad item. Don't use it if too close of a good item.
 *  \param item_skill The skill with which to use the item
 *  \param items_to_collect The list of close good items
 *  \param items_to_avoid The list of close bad items
 */
void SkiddingAI::handleSwitch(int item_skill,
                              const std::vector<const ItemState *> &items_to_collect,
                              const std::vector<const ItemState *> &items_to_avoid)
{
    // It's extremely unlikely two switches are used close one after another
    if(item_skill == 2)
    {
        if (m_time_since_last_shot > 2.0f)
        {
            m_controls->setFire(true);
           return;
        }
    }
             
    else if((item_skill == 3) || (item_skill == 4))
    {
       if( (item_skill == 4) && items_to_avoid.size() > 0)
       {
           float d = (items_to_avoid[0]->getXYZ() - m_kart->getXYZ()).length2();
       
           if (d < 2.0f)
           {
               m_controls->setFire(true);
               return;
           }
        }
        else if (items_to_collect.size() > 0)
        {
            float d = (items_to_collect[0]->getXYZ() - m_kart->getXYZ()).length2();
       
            if (d > 10.0f)
            {
               m_controls->setFire(true);
               return;
            }
        }
        else if (m_time_since_last_shot > 2.0f)
        {
             m_controls->setFire(true);
             return;
        }
    }
       
    //TODO : retrieve ranking powerup class and use it to evaluate the best item
    //       available depending of if the switch is used or not
    //       In the mean time big nitro > item box > small nitro
    //TODO : make steering switch-aware so that the kart goes towards a bad item
    //       and use the switch at the last moment
    //It would also be possible but complicated to check if using the switch will
    //cause another kart not far from taking a bad item instead of a good one
    //It should also be possible but complicated to discard items when a good
    //and a bad one are two close from one another
    else if(item_skill == 5)
    {  
       //First step : identify the best available item
       int bad = 0;
       int good = 0;

       //Good will store 1 for nitro, big or small, 2 for item box
       //Big nitro are usually hard to take for the AI
       for(int i=(int)items_to_collect.size()-1; i>=0; i--)
       {
           if (items_to_collect[i]->getType() == Item::ITEM_BONUS_BOX)
           {
              good = 2;
             i = -1;
           }
          else if ( (items_to_collect[i]->getType() == Item::ITEM_NITRO_BIG) ||
               (items_to_collect[i]->getType() == Item::ITEM_NITRO_SMALL) )
          {
              good = 1;
          }
       }
           
       //Bad will store 2 for bananas, 3 for bubble gum
       for(int i=(int)items_to_avoid.size()-1; i>=0; i--)
       {
           if (items_to_avoid[i]->getType() == Item::ITEM_BUBBLEGUM)
           {
              bad = 3;
              i = -1;
           }
           else if ( items_to_avoid[i]->getType() == Item::ITEM_BANANA )
           {
              bad = 2;
           }
       }
           
       //Second step : make sure a close item don't make the choice pointless
       if( items_to_avoid.size()>0)
       {
           float d = (items_to_avoid[0]->getXYZ() - m_kart->getXYZ()).length2();
       
           //fire if very close to a bad item
           if (d < 2.0f)
           {
              m_controls->setFire(true);
              return;
           }
       }
       if( items_to_collect.size()>0)
       {
          float d = (items_to_collect[0]->getXYZ() - m_kart->getXYZ()).length2();
       
          //don't fire if close to a good item
          if (d < 5.0f)
          {
             return;
          }
       }
           
       //Third step : Use or don't use to get the best available item
       if( bad > good)
       {
          m_controls->setFire(true);
          return;
       }
    } //item_skill == 5

    return;  
} //handleSwitch

//-----------------------------------------------------------------------------
/** Determines the closest karts just behind and in front of this kart. The
 *  'closeness' is for now simply based on the position, i.e. if a kart is
 *  more than one lap behind or ahead, it is not considered to be closest.
 */
void SkiddingAI::computeNearestKarts()
{
    int my_position    = m_kart->getPosition();

    // If we are not the first, there must be another kart ahead of this kart
    if( my_position>1 )
    {
        m_kart_ahead = m_world->getKartAtPosition(my_position-1);
        if(m_kart_ahead &&
              ( m_kart_ahead->isEliminated() || m_kart_ahead->hasFinishedRace()))
              m_kart_ahead = NULL;
    }
    else
        m_kart_ahead = NULL;

    if( my_position<(int)m_world->getCurrentNumKarts())
    {
        m_kart_behind = m_world->getKartAtPosition(my_position+1);
        if(m_kart_behind &&
            (m_kart_behind->isEliminated() || m_kart_behind->hasFinishedRace()))
            m_kart_behind = NULL;
    }
    else
        m_kart_behind = NULL;

    m_distance_leader = m_distance_ahead = m_distance_behind = 9999999.9f;
    float my_dist = m_world->getOverallDistance(m_kart->getWorldKartId());

    if(m_kart_ahead)
        m_distance_ahead = m_world->getOverallDistance(m_kart_ahead->getWorldKartId()) - my_dist;
    if(m_kart_behind)
        m_distance_behind = my_dist - m_world->getOverallDistance(m_kart_behind->getWorldKartId());

    if(   RaceManager::get()->isFollowMode() && m_kart->getWorldKartId() != 0)
        m_distance_leader = m_world->getOverallDistance(0 /*leader kart ID*/) - my_dist;

    // Compute distance to target player kart

    float target_overall_distance = 0.0f;
    float own_overall_distance = m_world->getOverallDistance(m_kart->getWorldKartId());
    m_num_players_ahead = 0;

    if (m_enabled_network_ai)
    {
        // Use maximum distance to player for network ai
        m_distance_to_player = 9999999.9f;
        return;
    }
    unsigned int n = ProfileWorld::isProfileMode() ? 0 : RaceManager::get()->getNumPlayers();

    std::vector<float> overall_distance;
    // Get the players distances
    for(unsigned int i=0; i<n; i++)
    {
        unsigned int kart_id = m_world->getPlayerKart(i)->getWorldKartId();
        overall_distance.push_back(m_world->getOverallDistance(kart_id));
    }

    // Sort the list in descending order
    std::sort(overall_distance.begin(), overall_distance.end(), std::greater<float>());
   
    // Get the AI's position (the position update may not be done, leading to crashes)
    int curr_position = 1;
    for(unsigned int i=0; i<m_world->getNumKarts(); i++)
    {
        if(   m_world->getOverallDistance(i) > own_overall_distance
           && !m_world->getKart(i)->isEliminated())
            curr_position++;
    }

    for(unsigned int i=0; i<n; i++)
    {
        if(overall_distance[i]>own_overall_distance)
            m_num_players_ahead++;
    }

    // Force best driving when profiling and for FTL leaders
    if(   ProfileWorld::isProfileMode()
       || ( RaceManager::get()->isFollowMode() && m_kart->getWorldKartId() == 0))
        target_overall_distance = 999999.9f;

    // In higher difficulties and in follow the leader, rubber band towards the first player,
    // if at all (SuperTux has no rubber banding at all). Boosted AIs also target the 1st player.
    else if (   RaceManager::get()->getDifficulty() == RaceManager::DIFFICULTY_HARD
             || RaceManager::get()->getDifficulty() == RaceManager::DIFFICULTY_BEST
             || RaceManager::get()->isFollowMode()
             || m_kart->getBoostAI())
    {
        target_overall_distance = overall_distance[n-1]; // Highest player distance
    }
    // Distribute the AIs to players
    else
    {
        int num_ai = m_world->getNumKarts() - RaceManager::get()->getNumPlayers();
        int position_among_ai = curr_position - m_num_players_ahead;

        // Converts a position among AI to a position among players
        // The 1st player get an index of 0, the 2nd an index of 2, etc.
        int target_index = 0;
       
        // Avoid a division by 0. If there is only one AI, it will target the first player
        if (num_ai > 1)
        {
            target_index  = (position_among_ai-1) * (RaceManager::get()->getNumPlayers()-1);
            target_index += (num_ai/2) - 1;
            target_index  = target_index / (num_ai - 1);
        }

        assert(target_index >= 0 && (unsigned)target_index <= n-1);
        target_overall_distance = overall_distance[target_index];
    }
    // Now convert 'maximum overall distance' to distance to player.
    m_distance_to_player = own_overall_distance - target_overall_distance;
}   // computeNearestKarts

//-----------------------------------------------------------------------------
/** Determines if the AI should accelerate or not, and if not if it should brake.
 *  \param ticks Time step size.
 * //TODO : make acceleration steering aware
 */
void SkiddingAI::handleAccelerationAndBraking(int ticks)
{
    // Step 0 (start only) : do not accelerate until we have delayed the start enough
    if( m_start_delay > 0 )
    {
        m_start_delay -= ticks;
        m_controls->setAccel(0.0f);
        return;
    }

    // Step 1 : determine the appropriate max speed for the curve we are in
    //         (this is also calculated in straights, as there is always a
    //          curve lurking at its end)

    // FIXME - requires fixing of the turn radius bugs

    float max_turn_speed =
        m_kart->getSpeedForTurnRadius(m_current_curve_radius)*1.5f;

    // A kart will not brake when the speed is already slower than this
    // value. This prevents a kart from going too slow (or even backwards)
    // in tight curves.
    const float MIN_SPEED = 5.0f;

    // Step 2 : handle braking (there are some cases who need braking besides
    //          a too great speed, like overtaking the leader in FTL)
    handleBraking(max_turn_speed, MIN_SPEED);

    if( m_controls->getBrake())
    {
        m_controls->setAccel(0.0f);
        return;
    }

    // Step 3 : handle nitro and zipper

    // If a bomb is attached, nitro might already be set.
    // FIXME : the bomb situation should be merged here
    if(!m_controls->getNitro())
        handleNitroAndZipper(max_turn_speed);

    // Step 4 : handle plunger effect

    if(m_kart->getBlockedByPlungerTicks()>0)
    {
        int item_skill = computeSkill(ITEM_SKILL);
        float accel_threshold = 0.5f;

        if (item_skill == 0)
            accel_threshold = 0.3f;
        else if (item_skill == 1)
            accel_threshold = 0.5f;
        else if (item_skill == 2)
            accel_threshold = 0.6f;
        else if (item_skill == 3)
            accel_threshold = 0.7f;
        else if (item_skill == 4)
            accel_threshold = 0.8f;
        // The best players, knowing the track, don't slow down with a plunger
        else if (item_skill == 5)
            accel_threshold = 1.0f;

        if(m_kart->getSpeed() > m_kart->getCurrentMaxSpeed() * accel_threshold)
            m_controls->setAccel(0.0f);
        return;
    }

    m_controls->setAccel(stk_config->m_ai_acceleration);

}   // handleAccelerationAndBraking


//-----------------------------------------------------------------------------
/** This function decides if the AI should brake.
 *  The decision can be based on race mode (e.g. in follow the leader the AI
 *  will brake if it is ahead of the leader). Otherwise it will depend on
 *  the direction the AI is facing (if it's not facing in the track direction
 *  it will brake in order to make it easier to re-align itself), and
 *  estimated curve radius (brake to avoid being pushed out of a curve).
 */
void SkiddingAI::handleBraking(float max_turn_speed, float min_speed)
{
    m_controls->setBrake(false);
    // In follow the leader mode, the kart should brake if they are ahead of
    // the leader (and not the leader, i.e. don't have initial position 1)
    // TODO : if there is still time in the countdown and the leader is faster,
    //        the AI kart should not slow down too much, to stay closer to the
    //        leader once overtaken.
    if(   RaceManager::get()->isFollowMode() && m_distance_leader < 2
       && m_kart->getInitialPosition()>1
       && m_world->getOverallDistance(m_kart->getWorldKartId()) > 0 )
    {
#ifdef DEBUG
    if(m_ai_debug)
        Log::debug(getControllerName().c_str(), "braking: %s too close of leader.",
                   m_kart->getIdent().c_str());
#endif

        m_controls->setBrake(true);
        return;
    }

    // If the kart is not facing roughly in the direction of the track, brake
    // so that it is easier for the kart to turn in the right direction.
    if(m_current_track_direction==DriveNode::DIR_UNDEFINED &&
        m_kart->getSpeed() > min_speed)
    {
#ifdef DEBUG
        if(m_ai_debug)
            Log::debug(getControllerName().c_str(),
                       "%s not aligned with track.",
                       m_kart->getIdent().c_str());
#endif
        m_controls->setBrake(true);
        return;
    }
    if(m_current_track_direction==DriveNode::DIR_LEFT ||
       m_current_track_direction==DriveNode::DIR_RIGHT   )
    {
        if(m_kart->getSpeed() > max_turn_speed  &&
            m_kart->getSpeed()>min_speed        &&
            fabsf(m_controls->getSteer()) > 0.95f )
        {
            m_controls->setBrake(true);
#ifdef DEBUG
            if(m_ai_debug)
                Log::debug(getControllerName().c_str(),
                           "speed %f too tight curve: radius %f ",
                           m_kart->getSpeed(),
                           m_kart->getIdent().c_str(),
                           m_current_curve_radius);
#endif
        }
        return;
    }

    return;

}   // handleBraking

//-----------------------------------------------------------------------------
void SkiddingAI::handleRaceStart()
{
    if( m_start_delay <  0 )
    {
        if (m_enabled_network_ai)
        {
            m_start_delay = 0;
            return;
        }
        // Each kart starts at a different, random time, and the time is
        // smaller depending on the difficulty.
        m_start_delay = stk_config->time2Ticks(
                        m_ai_properties->m_min_start_delay
                      + (float) rand() / (float) RAND_MAX
                      * (m_ai_properties->m_max_start_delay -
                         m_ai_properties->m_min_start_delay)   );

        float false_start_probability =
               m_superpower == RaceManager::SUPERPOWER_NOLOK_BOSS
               ? 0.0f  : m_ai_properties->m_false_start_probability;

        // Now check for a false start. If so, add 1 second penalty time.
        if (rand() < (float) RAND_MAX * false_start_probability)
        {
            m_start_delay+=stk_config->m_penalty_ticks;
            return;
        }
        m_kart->setStartupBoost(m_kart->getStartupBoostFromStartTicks(
            m_start_delay + stk_config->time2Ticks(1.0f)));
        m_start_delay = 0;
    }
}   // handleRaceStart

//-----------------------------------------------------------------------------
//TODO: if the AI is crashing constantly, make it move backwards in a straight
//line, then move forward while turning.

void SkiddingAI::handleRescue(const float dt)
{
    // check if kart is stuck
    if(m_kart->getSpeed()<2.0f && !m_kart->getKartAnimation() &&
        !m_world->isStartPhase() && m_start_delay == 0)
    {
        m_time_since_stuck += dt;
        if(m_time_since_stuck > 2.0f)
        {
            // For network AI controller
            if (m_enabled_network_ai)
                m_controls->setRescue(true);
            else
                RescueAnimation::create(m_kart);
            m_time_since_stuck=0.0f;
        }   // m_time_since_stuck > 2.0f
    }
    else
    {
        m_time_since_stuck = 0.0f;
    }
}   // handleRescue

//-----------------------------------------------------------------------------
/** Decides wether to use nitro and zipper or not.
 */
void SkiddingAI::handleNitroAndZipper(float max_safe_speed)
{
    int nitro_skill = computeSkill(NITRO_SKILL);
    int item_skill = computeSkill(ITEM_SKILL);
   
    //Nitro continue to be advantageous during the fadeout (nitro ticks continue to tick in the negatives)
    int nitro_ticks = m_kart->getSpeedIncreaseTicksLeft(MaxSpeed::MS_INCREASE_NITRO);
    float time_until_fadeout = ( stk_config->ticks2Time(nitro_ticks)
                       + m_kart->getKartProperties()->getNitroFadeOutTime() );

    // Because it takes some time after nitro activation to accelerate to the increased max speed,
    // the best moment for the next burst of nitro is not when the fade-out has
    // finished, but it is not either before the fade-out starts.
    float nitro_max_active = m_kart->getKartProperties()->getNitroDuration();
    float nitro_max_fadeout = m_kart->getKartProperties()->getNitroFadeOutTime();

    //Nitro skill 0 : don't use
    //Nitro skill 1 : don't use if the kart is braking, is not on the ground, has finished the race, has no nitro,
    //                has a parachute or an anvil attached, or has a plunger in the face.
    //                Otherwise, use it immediately
    //Nitro skill 2 : Don't use nitro if there is more than 35% of main effect left..
    //                Use it when at max speed or under 5 of speed (after rescue, etc.). Use it to pass bombs.
    //                Tries to builds a reserve of 4 energy to use towards the end
    //Nitro skill 3 : Same as level 2, but don't use until 10% of main effect left, and don't use close
    //                to bad items, and has a target reserve of 8 energy
    //Nitro skill 4 : Same as level 3, but don't use until 50% of fadeout left and ignore the plunger
    //                and has a target reserve of 12 energy
   
    m_controls->setNitro(false);

    float energy_reserve = 0;

    if (nitro_skill == 2)
        energy_reserve = 4;  
    else if (nitro_skill == 3)
        energy_reserve = 8;  
    else if (nitro_skill == 4)
        energy_reserve = 12;  

    // No point in building a big nitro reserve in nitro for FTL AIs,
    // just keep enough to help accelerating after an accident
    if(RaceManager::get()->isFollowMode())
        energy_reserve = std::min(2.0f, energy_reserve);
   
    // Don't use nitro or zipper if we are braking
    if(m_controls->getBrake()) return;
   
    // Don't use nitro or zipper if the kart is not on ground or has finished the race
    if(!m_kart->isOnGround() || m_kart->hasFinishedRace()) return;
   
    // Don't use nitro or zipper when the AI has a plunger in the face!
    if(m_kart->getBlockedByPlungerTicks()>0)
    {
        if ((nitro_skill < 4) && (item_skill < 5))
            return;
        // No else-if because the nitro_skill and item_skill conditions can happen together
        if (nitro_skill < 4)
            nitro_skill = 0;
        if (item_skill < 5)
            item_skill = 0;
    }

    // Don't use nitro or zipper if it would make the kart go too fast

    if(m_kart->getSpeed() + m_kart->getKartProperties()->getNitroMaxSpeedIncrease() > max_safe_speed)
        nitro_skill = 0;

    // FIXME : as the zipper can give +15, but only gives +5 instant, this may be too conservative
    if(m_kart->getSpeed() + m_kart->getKartProperties()->getZipperMaxSpeedIncrease() > max_safe_speed)
        item_skill = 0;

    // If a parachute or anvil is attached, the nitro and zipper don't give much
    // benefit. Better wait till later.
    const bool has_slowdown_attachment =
        m_kart->getAttachment()->getType()==Attachment::ATTACH_PARACHUTE ||
        m_kart->getAttachment()->getType()==Attachment::ATTACH_ANVIL;
    if(has_slowdown_attachment) return;
   
    // Don't compute nitro usage if we don't have nitro
    if( m_kart->getEnergy()==0 )
    {
       nitro_skill = 0;
    }

    // If the AI has a lot of nitro, it should consume it without waiting for some fadeout
    bool big_reserve = false;


    // If this kart is the last kart, set nitro reserve to be at most 2
    const unsigned int num_karts = m_world->getCurrentNumKarts();
    if(nitro_skill >= 2 && m_kart->getPosition()== (int)num_karts &&
        num_karts > 1 )
    {
        energy_reserve = 2;
    }

    // Estimate time towards the end of the race.
    // Decreases the reserve size when there is an estimate of time remaining
    // to the end of less than 2,5 times the maximum nitro effect duration.
    // This vary depending on kart characteristic.
    // There is a margin because the kart will go a bit faster than predicted
    // by the estimate, because the kart may collect more nitro and because
    // there may be moments when it's not useful to use nitro (parachutes, etc).
    if(nitro_skill >= 2 && energy_reserve > 0.0f)
    {
        float finish = m_world->getEstimatedFinishTime(m_kart->getWorldKartId()) - m_world->getTime();
        float max_time_effect = (nitro_max_active + nitro_max_fadeout) / m_kart->getKartProperties()->getNitroConsumption()
                                * m_kart->getEnergy()*2; //the minimum burst consumes around 0.5 energy

        // The burster forces the AI to consume its reserve by series of 2 bursts
        // Otherwise the bursting differences of the various nitro skill wouldn't matter here
        // In short races, most AI nitro usage may be at the end with the reserve
        
        if(m_burster && time_until_fadeout >= 0)
            energy_reserve = 0;
        else if (m_burster)
            m_burster = false;

        if( (2.5f*max_time_effect) >= finish )
        {
            // Absolute reduction to avoid small amount of unburned nitro at the end
            energy_reserve = std::min(energy_reserve, finish/(2.5f*max_time_effect/m_kart->getEnergy()) - 0.5f);
        }
        if( (1.8f*max_time_effect) >= finish || m_kart->getEnergy() >= 16)
            big_reserve = true;
    }
   
    // Don't use nitro if there is already a nitro boost active
    // Nitro effect and fadeout may vary between karts type
    // So vary time according to kart properties
    if (   ((nitro_skill == 4) && time_until_fadeout >= (nitro_max_fadeout*0.50f) && !big_reserve)
        || ((nitro_skill == 4) && time_until_fadeout >= (nitro_max_fadeout))
        || ((nitro_skill == 3) && time_until_fadeout >= (nitro_max_fadeout + nitro_max_active*0.10f))
        || ((nitro_skill == 2) && time_until_fadeout >= (nitro_max_fadeout + nitro_max_active*0.35f)))
    {
       nitro_skill = 0;
    }

    // If trying to pass a bomb to a kart faster or far ahead, use nitro reserve
    if(m_kart->getAttachment()->getType() == Attachment::ATTACH_BOMB
       && nitro_skill >= 2 && energy_reserve > 0.0f)
    {
        if (m_distance_ahead>10.0f || m_kart_ahead->getSpeed() > m_kart->getSpeed() )
        {
            energy_reserve = 0;
        }
    }

    // TODO : if a kart behind and reasonably close goes faster
    //        and it has a swatter, use nitro to try and dodge the swatter.
   
    // Don't use nitro if building an energy reserve
    if (m_kart->getEnergy() <= energy_reserve)
    {
        nitro_skill = 0;  
    }

    // If basic AI, or if the kart is very slow (e.g. after rescue) but not too much (accident)
    // or if kart is near the base max speed, use nitro.
    // This last condition is to profit from the max speed increase
    // And because it means there should be no slowing down from, e.g. plungers
    // If the kart has a huge reserve and may not have enough time until the end to ue it,
    // relax the speed condition.
    if (    nitro_skill == 1
        || (nitro_skill > 1 && (   (m_kart->getSpeed()<5 && m_kart->getSpeed()>2)
                                || (m_kart->getSpeed()> 0.96f*m_kart->getCurrentMaxSpeed())
                                || (m_kart->getSpeed()> 0.3f *m_kart->getCurrentMaxSpeed() && big_reserve))) )
    {
        m_controls->setNitro(true);
        m_burster = !m_burster;
    }

    //TODO : for now we don't disable nitro use when close to bananas and gums,
    //       because it hurts more often than not (when the bad item is avoided)
    if(m_avoid_item_close)
        return;
   
    // Use zipper
    if(m_kart->getPowerup()->getType() == PowerupManager::POWERUP_ZIPPER 
        && item_skill >= 2 && m_kart->getSpeed()>1.0f &&
        m_kart->getSpeedIncreaseTicksLeft(MaxSpeed::MS_INCREASE_ZIPPER)<=0)
    {
        DriveNode::DirectionType dir;
        unsigned int last;
        const DriveNode* dn = DriveGraph::get()->getNode(m_track_node);
        dn->getDirectionData(m_successor_index[m_track_node], &dir, &last);
        if(dir==DriveNode::DIR_STRAIGHT)
        {
            float diff = DriveGraph::get()->getDistanceFromStart(last)
                       - DriveGraph::get()->getDistanceFromStart(m_track_node);
            if(diff<0) diff += Track::getCurrentTrack()->getTrackLength();
            if(diff>m_ai_properties->m_straight_length_for_zipper)
                m_controls->setFire(true);
        }

    }
}   // handleNitroAndZipper

//-----------------------------------------------------------------------------
/** Returns the AI skill value used by the kart
 */
int SkiddingAI::computeSkill(SkillType type)
{
    if (type == ITEM_SKILL)
    {
        int item_skill = 0;

        if (m_ai_properties->m_item_usage_skill > 0)
        {
            if (m_ai_properties->m_item_usage_skill > 5)
            {
                item_skill = 5;
            }
            else
            {
                item_skill = m_ai_properties->m_item_usage_skill;
            }
        }

   
        if (m_kart->getBoostAI() == true)
        {
            if (item_skill < 5)
            {
                item_skill = item_skill + 1; //possible improvement : make the boost amplitude pulled from config
            }
        }
        return item_skill;
    }

    else if (type == NITRO_SKILL)
    {
        int nitro_skill = 0;

        if (m_ai_properties->m_nitro_usage > 0)
        {
            if (m_ai_properties->m_nitro_usage > 4)
            {
                nitro_skill = 4;
            }
            else
            {
                nitro_skill = m_ai_properties->m_nitro_usage;
            }
        }
   
        if (m_kart->getBoostAI() == true)
        {
            if (nitro_skill < 4)
            {
                nitro_skill = nitro_skill + 1; //possible improvement : make the boost amplitude pulled from config
            }
        }
        return nitro_skill;
    }
    
    return 0;
} //computeSkill

//-----------------------------------------------------------------------------
void SkiddingAI::checkCrashes(const Vec3& pos )
{
    int steps = int( m_kart->getVelocityLC().getZ() / m_kart_length );
    if( steps < 2 ) steps = 2;

    // The AI drives significantly better with more steps, so for now
    // add 5 additional steps.
    steps+=5;

    //Right now there are 2 kind of 'crashes': with other karts and another
    //with the track. The sight line is used to find if the karts crash with
    //each other, but the first step is twice as big as other steps to avoid
    //having karts too close in any direction. The crash with the track can
    //tell when a kart is going to get out of the track so it steers.
    m_crashes.clear();

    // If slipstream should be handled actively, trigger overtaking the
    // kart which gives us slipstream if slipstream is ready
    const SlipStream *slip=m_kart->getSlipstream();
    // Atm network ai always use slipstream because it's a player controller
    // underlying
    bool use_slipstream =
        m_enabled_network_ai || m_ai_properties->m_make_use_of_slipstream;
    if(use_slipstream &&
        slip->isSlipstreamReady() &&
        slip->getSlipstreamTarget())
    {
        //Log::debug(getControllerName().c_str(), "%s overtaking %s",
        //           m_kart->getIdent().c_str(),
        //           m_kart->getSlipstreamKart()->getIdent().c_str());
        // FIXME: we might define a minimum distance, and if the target kart
        // is too close break first - otherwise the AI hits the kart when
        // trying to overtake it, actually speeding the other kart up.
        m_crashes.m_kart = slip->getSlipstreamTarget()->getWorldKartId();
    }

    const size_t NUM_KARTS = m_world->getNumKarts();

    float speed = m_kart->getVelocity().length();
    // If the velocity is zero, no sense in checking for crashes in time
    if(speed==0) return;

    Vec3 vel_normal = m_kart->getVelocity().normalized();

    // Time it takes to drive for m_kart_length units.
    float dt = m_kart_length / speed;

    int current_node = m_track_node;
    if(steps<1 || steps>1000)
    {
        Log::warn(getControllerName().c_str(),
                  "Incorrect STEPS=%d. kart_length %f velocity %f",
                  steps, m_kart_length, m_kart->getVelocityLC().getZ());
        steps=1000;
    }
    for(int i = 1; steps > i; ++i)
    {
        Vec3 step_coord = pos + vel_normal* m_kart_length * float(i);

        /* Find if we crash with any kart, as long as we haven't found one
         * yet
         */
        if( m_crashes.m_kart == -1 )
        {
            for( unsigned int j = 0; j < NUM_KARTS; ++j )
            {
                const AbstractKart* kart = m_world->getKart(j);
                // Ignore eliminated karts
                if(kart==m_kart||kart->isEliminated()||kart->isGhostKart()) continue;
                const AbstractKart *other_kart = m_world->getKart(j);
                // Ignore karts ahead that are faster than this kart.
                if(m_kart->getVelocityLC().getZ() < other_kart->getVelocityLC().getZ())
                    continue;
                Vec3 other_kart_xyz = other_kart->getXYZ()
                                    + other_kart->getVelocity()*(i*dt);
                float kart_distance = (step_coord - other_kart_xyz).length();

                if( kart_distance < m_kart_length)
                    m_crashes.m_kart = j;
            }
        }

        /*Find if we crash with the drivelines*/
        if(current_node!=Graph::UNKNOWN_SECTOR &&
            m_next_node_index[current_node]!=-1)
            DriveGraph::get()->findRoadSector(step_coord, &current_node,
                        /* sectors to test*/ &m_all_look_aheads[current_node]);

        if( current_node == Graph::UNKNOWN_SECTOR)
        {
            m_crashes.m_road = true;
            return;
        }
    }
}   // checkCrashes

//-----------------------------------------------------------------------------
/** This is a new version of findNonCrashingPoint, which at this stage is
 *  slightly inferior (though faster and more correct) than the original
 *  version - the original code cuts corner more aggressively than this
 *  version (and in most cases cuting the corner does not end in a
 *  collision, so it's actually faster).
 *  This version find the point furthest ahead which can be reached by
 *  travelling in a straight direction from the current location of the
 *  kart. This is done by using two lines: one from the kart to the
 *  lower left side of the next quad, and one from the kart to the
 *  lower right side of the next quad. The area between those two lines
 *  can be reached by the kart in a straight line, and will not go off
 *  track (assuming that the kart is on track). Then the next quads are
 *  tested: New left/right lines are computed. If the new left line is to
 *  the right of the old left line, the new left line becomes the current
 *  left line:
 *
 *            X      The new left line connecting kart to X will be to the right
 *        \        / of the old left line, so the available area for the kart
 *         \      /  will be dictated by the new left line.
 *          \    /
 *           kart
 *
 *  Similarly for the right side. This will narrow down the available area
 *  the kart can aim at, till finally the left and right line overlap.
 *  All points between the connection of the two end points of the left and
 *  right line can be reached without getting off track. Which point the
 *  kart aims at then depends on the direction of the track: if there is
 *  a left turn, the kart will aim to the left point (and vice versa for
 *  right turn) - slightly offset by the width of the kart to avoid that
 *  the kart is getting off track.
 *  \param aim_position The point to aim for, i.e. the point that can be
 *         driven to in a straight line.
 *  \param last_node The graph node index in which the aim_position is.
*/
void SkiddingAI::findNonCrashingPointNew(Vec3 *result, int *last_node)
{
    *last_node = m_next_node_index[m_track_node];
    const core::vector2df xz = m_kart->getXYZ().toIrrVector2d();

    const DriveNode* dn = DriveGraph::get()->getNode(*last_node);

    // Index of the left and right end of a quad.
    const unsigned int LEFT_END_POINT  = 0;
    const unsigned int RIGHT_END_POINT = 1;
    core::line2df left (xz, (*dn)[LEFT_END_POINT ].toIrrVector2d());
    core::line2df right(xz, (*dn)[RIGHT_END_POINT].toIrrVector2d());

#if defined(AI_DEBUG) && defined(AI_DEBUG_NEW_FIND_NON_CRASHING)
    const Vec3 eps1(0,0.5f,0);
    m_curve[CURVE_LEFT]->clear();
    m_curve[CURVE_LEFT]->addPoint(m_kart->getXYZ()+eps1);
    m_curve[CURVE_LEFT]->addPoint((*dn)[LEFT_END_POINT]+eps1);
    m_curve[CURVE_LEFT]->addPoint(m_kart->getXYZ()+eps1);
    m_curve[CURVE_RIGHT]->clear();
    m_curve[CURVE_RIGHT]->addPoint(m_kart->getXYZ()+eps1);
    m_curve[CURVE_RIGHT]->addPoint((*dn)[RIGHT_END_POINT]+eps1);
    m_curve[CURVE_RIGHT]->addPoint(m_kart->getXYZ()+eps1);
#endif
#if defined(AI_DEBUG_KART_HEADING) || defined(AI_DEBUG_NEW_FIND_NON_CRASHING)
    const Vec3 eps(0,0.5f,0);
    m_curve[CURVE_KART]->clear();
    m_curve[CURVE_KART]->addPoint(m_kart->getXYZ()+eps);
    Vec3 forw(0, 0, 50);
    m_curve[CURVE_KART]->addPoint(m_kart->getTrans()(forw)+eps);
#endif
    while(1)
    {
        unsigned int next_sector = m_next_node_index[*last_node];
        const DriveNode* dn_next = DriveGraph::get()->getNode(next_sector);
        // Test if the next left point is to the right of the left
        // line. If so, a new left line is defined.
        if(left.getPointOrientation((*dn_next)[LEFT_END_POINT].toIrrVector2d())
            < 0 )
        {
            core::vector2df p = (*dn_next)[LEFT_END_POINT].toIrrVector2d();
            // Stop if the new point is to the right of the right line
            if(right.getPointOrientation(p)<0)
                break;
            left.end = p;
#if defined(AI_DEBUG) && defined(AI_DEBUG_NEW_FIND_NON_CRASHING)
            Vec3 ppp(p.X, m_kart->getXYZ().getY(), p.Y);
            m_curve[CURVE_LEFT]->addPoint(ppp+eps);
            m_curve[CURVE_LEFT]->addPoint(m_kart->getXYZ()+eps);
#endif
        }
        else
            break;

        // Test if new right point is to the left of the right line. If
        // so, a new right line is defined.
        if(right.getPointOrientation((*dn_next)[RIGHT_END_POINT].toIrrVector2d())
            > 0 )
        {
            core::vector2df p = (*dn_next)[RIGHT_END_POINT].toIrrVector2d();
            // Break if new point is to the left of left line
            if(left.getPointOrientation(p)>0)
                break;
#if defined(AI_DEBUG) && defined(AI_DEBUG_NEW_FIND_NON_CRASHING)

            Vec3 ppp(p.X, m_kart->getXYZ().getY(), p.Y);
            m_curve[CURVE_RIGHT]->addPoint(ppp+eps);
            m_curve[CURVE_RIGHT]->addPoint(m_kart->getXYZ()+eps);
#endif
            right.end = p;
        }
        else
            break;
        *last_node = next_sector;
    }   // while

    //Vec3 ppp(0.5f*(left.end.X+right.end.X),
    //         m_kart->getXYZ().getY(),
    //         0.5f*(left.end.Y+right.end.Y));
    //*result = ppp;

    *result = DriveGraph::get()->getNode(*last_node)->getCenter();
}   // findNonCrashingPointNew

//-----------------------------------------------------------------------------
/** This is basically the original AI algorithm. It is clearly buggy:
 *  1. the test:
 *
 *         distance + m_kart_width * 0.5f
 *                  > DriveGraph::get()->getNode(*last_node)->getPathWidth() )
 *
 *     is incorrect, it should compare with getPathWith*0.5f (since distance
 *     is the distance from the center, i.e. it is half the path width if
 *     the point is at the edge).
 *  2. the test:
 *
 *         DriveGraph::get()->spatialToTrack(&step_track_coord, step_coord,
 *                                          *last_node );
 *     in the for loop tests always against distance from the same
 *     graph node (*last_node), while de-fact the loop will test points
 *     on various graph nodes.
 *
 *  This results in this algorithm often picking points to aim at that
 *  would actually force the kart off track. But in reality the kart has
 *  to turn (and does not immediate in one frame change its direction)
 *  which takes some time - so it is actually mostly on track.
 *  Since this algoritm (so far) ends up with by far the best AI behaviour,
 *  it is for now the default).
 *  \param aim_position On exit contains the point the AI should aim at.
 *  \param last_node On exit contais the graph node the AI is aiming at.
*/
 void SkiddingAI::findNonCrashingPoint(Vec3 *aim_position, int *last_node)
{
#ifdef AI_DEBUG_KART_HEADING
    const Vec3 eps(0,0.5f,0);
    m_curve[CURVE_KART]->clear();
    m_curve[CURVE_KART]->addPoint(m_kart->getXYZ()+eps);
    Vec3 forw(0, 0, 50);
    m_curve[CURVE_KART]->addPoint(m_kart->getTrans()(forw)+eps);
#endif
    *last_node = m_next_node_index[m_track_node];
    float angle = DriveGraph::get()->getAngleToNext(m_track_node,
                                              m_successor_index[m_track_node]);

    Vec3 direction;
    Vec3 step_track_coord;

    // The original while(1) loop is replaced with a for loop to avoid
    // infinite loops (which we had once or twice). Usually the number
    // of iterations in the while loop is less than 7.
    for(unsigned int j=0; j<100; j++)
    {
        // target_sector is the sector at the longest distance that we can
        // drive to without crashing with the track.
        int target_sector = m_next_node_index[*last_node];
        float angle1 = DriveGraph::get()->getAngleToNext(target_sector,
                                                m_successor_index[target_sector]);
        // In very sharp turns this algorithm tends to aim at off track points,
        // resulting in hitting a corner. So test for this special case and
        // prevent a too-far look-ahead in this case
        float diff = normalizeAngle(angle1-angle);
        if(fabsf(diff)>1.5f)
        {
            *aim_position = DriveGraph::get()->getNode(target_sector)
                                            ->getCenter();
            return;
        }

        //direction is a vector from our kart to the sectors we are testing
        direction = DriveGraph::get()->getNode(target_sector)->getCenter()
                  - m_kart->getXYZ();

        float len=direction.length();
        unsigned int steps = (unsigned int)( len / m_kart_length );
        if( steps < 3 ) steps = 3;

        // That shouldn't happen, but since we had one instance of
        // STK hanging, add an upper limit here (usually it's at most
        // 20 steps)
        if( steps>1000) steps = 1000;

        // Protection against having vel_normal with nan values
        if(len>0.0f) {
            direction*= 1.0f/len;
        }

        Vec3 step_coord;
        //Test if we crash if we drive towards the target sector
        for(unsigned int i = 2; i < steps; ++i )
        {
            step_coord = m_kart->getXYZ()+direction*m_kart_length * float(i);

            DriveGraph::get()->spatialToTrack(&step_track_coord, step_coord,
                                             *last_node );

            float distance = fabsf(step_track_coord[0]);

            //If we are outside, the previous node is what we are looking for
            if ( distance + m_kart_width * 0.5f
                 > DriveGraph::get()->getNode(*last_node)->getPathWidth() )
            {
                *aim_position = DriveGraph::get()->getNode(*last_node)
                                                ->getCenter();
                return;
            }
        }
        angle = angle1;
        *last_node = target_sector;
    }   // for i<100
    *aim_position = DriveGraph::get()->getNode(*last_node)->getCenter();
}   // findNonCrashingPoint

//-----------------------------------------------------------------------------
/** Determines the direction of the track ahead of the kart: 0 indicates
 *  straight, +1 right turn, -1 left turn.
 */
void SkiddingAI::determineTrackDirection()
{
    const DriveGraph *dg = DriveGraph::get();
    unsigned int succ    = m_successor_index[m_track_node];
    unsigned int next    = dg->getNode(m_track_node)->getSuccessor(succ);
    float angle_to_track = 0.0f;
    if (m_kart->getVelocity().length() > 0.0f)
    {
        Vec3 track_direction = -dg->getNode(m_track_node)->getCenter()
            + dg->getNode(next)->getCenter();
        angle_to_track =
            track_direction.angle(m_kart->getVelocity().normalized());
    }
    angle_to_track = normalizeAngle(angle_to_track);

    // In certain circumstances (esp. S curves) it is possible that the
    // kart is not facing in the direction of the track. In this case
    // determining the curve radius based on the direction the kart is
    // facing results in very incorrect results (example: if the kart is
    // in a tight curve, but already facing towards the last point of the
    // curve - in this case a huge curve radius will be computes (since
    // the kart is nearly going straight), while in fact the kart would
    // go across the circle and not along, bumping into the track).
    // To avoid this we set the direction to undefined in this case,
    // which causes the kart to brake (which will allow the kart to
    // quicker be aligned with the track again).
    if(fabsf(angle_to_track) > 0.22222f * M_PI)
    {
        m_current_track_direction = DriveNode::DIR_UNDEFINED;
        return;
    }

    dg->getNode(next)->getDirectionData(m_successor_index[next],
                                        &m_current_track_direction,
                                        &m_last_direction_node);

#ifdef AI_DEBUG
    m_curve[CURVE_QG]->clear();
    for(unsigned int i=m_track_node; i<=m_last_direction_node; i++)
    {
        m_curve[CURVE_QG]->addPoint(dg->getNode(i)->getCenter());
    }
#endif

    if(m_current_track_direction==DriveNode::DIR_LEFT  ||
       m_current_track_direction==DriveNode::DIR_RIGHT   )
    {
        handleCurve();
    }   // if(m_current_track_direction == DIR_LEFT || DIR_RIGHT   )


    return;
}   // determineTrackDirection

// ----------------------------------------------------------------------------
/** If the kart is at/in a curve, determine the turn radius.
 */
void SkiddingAI::handleCurve()
{
    // Ideally we would like to have a circle that:
    // 1) goes through the kart position
    // 2) has the current heading of the kart as tangent in that point
    // 3) goes through the last point
    // 4) has a tangent at the last point that faces towards the next node
    // Unfortunately conditions 1 to 3 already fully determine the circle,
    // i.e. it is not always possible to find an appropriate circle.
    // Using the first three conditions is mostly a good choice (since the
    // kart will already point towards the direction of the circle), and
    // the case that the kart is facing wrong was already tested for before

    const DriveGraph *dg = DriveGraph::get();
    const Vec3& last_xyz = dg->getNode(m_last_direction_node)->getCenter();

    determineTurnRadius(last_xyz, &m_curve_center, &m_current_curve_radius);
    assert(!std::isnan(m_curve_center.getX()));
    assert(!std::isnan(m_curve_center.getY()));
    assert(!std::isnan(m_curve_center.getZ()));

#undef ADJUST_TURN_RADIUS_TO_AVOID_CRASH_INTO_TRACK
#ifdef ADJUST_TURN_RADIUS_TO_AVOID_CRASH_INTO_TRACK
    // NOTE: this can deadlock if the AI is going on a shortcut, since
    // m_last_direction_node is based on going on the main driveline :(
    unsigned int i= m_track_node;
    while(1)
    {
        i = m_next_node_index[i];
        // Pick either the lower left or right point:
        int index = m_current_track_direction==DriveNode::DIR_LEFT
                  ? 0 : 1;
        Vec3 curve_center_wc = m_kart->getTrans()(m_curve_center);
        float r = (curve_center_wc - *(dg->getNode(i))[index]).length();
        if(m_current_curve_radius < r)
        {
            last_xyz = *(dg->getNode(i)[index]);
            determineTurnRadius(last_xyz,
                                &m_curve_center, &m_current_curve_radius);
            m_last_direction_node = i;
            break;
        }
        if(i==m_last_direction_node)
            break;
    }
#endif
#if defined(AI_DEBUG) && defined(AI_DEBUG_CIRCLES)
    m_curve[CURVE_PREDICT1]->makeCircle(m_kart->getTrans()(m_curve_center),
                                        m_current_curve_radius);
    m_curve[CURVE_PREDICT1]->addPoint(last_xyz);
    m_curve[CURVE_PREDICT1]->addPoint(m_kart->getTrans()(m_curve_center));
    m_curve[CURVE_PREDICT1]->addPoint(m_kart->getXYZ());
#endif

}   // handleCurve
// ----------------------------------------------------------------------------
/** Determines if the kart should skid. The base implementation enables
 *  skidding
 *  \param steer_fraction The steering fraction as computed by the
 *          AIBaseLapController.
 *  \return True if the kart should skid.
 */
bool SkiddingAI::canSkid(float steer_fraction)
{
    if(fabsf(steer_fraction)>1.5f)
    {
        // If the kart has to do a sharp turn, but is already skidding, find
        // a good time to release the skid button, since this will turn the
        // kart more sharply:
        if(m_controls->getSkidControl())
        {
#ifdef DEBUG
            if(m_ai_debug)
            {
                if(fabsf(steer_fraction)>=2.5f)
                    Log::debug(getControllerName().c_str(),
                               "%s stops skidding (%f).",
                               m_kart->getIdent().c_str(), steer_fraction);
            }
#endif
            // If the current turn is not sharp enough, delay releasing
            // the skid button.
            return fabsf(steer_fraction)<2.5f;
        }

        // If the kart is not skidding, now is not a good time to start
        return false;
    }

    // No skidding on straights
    if(m_current_track_direction==DriveNode::DIR_STRAIGHT ||
       m_current_track_direction==DriveNode::DIR_UNDEFINED  )
    {
#ifdef DEBUG
        if(m_controls->getSkidControl() && m_ai_debug)
        {
            Log::debug(getControllerName().c_str(),
                       "%s stops skidding on straight.",
                       m_kart->getIdent().c_str());
        }
#endif
        return false;
    }

    const float MIN_SKID_SPEED = 5.0f;
    const DriveGraph *dg = DriveGraph::get();
    Vec3 last_xyz        = m_kart->getTrans().inverse()
                           (dg->getNode(m_last_direction_node)->getCenter());

    // Only try skidding when a certain minimum speed is reached.
    if(m_kart->getSpeed()<MIN_SKID_SPEED) return false;

    // Estimate how long it takes to finish the curve
    Vec3 diff_kart = -m_curve_center;
    Vec3 diff_last = last_xyz - m_curve_center;
    float angle_kart = atan2(diff_kart.getX(), diff_kart.getZ());
    float angle_last = atan2(diff_last.getX(), diff_last.getZ());
    float angle = m_current_track_direction == DriveNode::DIR_RIGHT
                ? angle_last - angle_kart
                : angle_kart - angle_last;
    angle = normalizeAngle(angle);
    float length = m_current_curve_radius*fabsf(angle);
    float duration = length / m_kart->getSpeed();
    // The estimated skdding time is usually too short - partly because
    // he speed of the kart decreases during the turn, partly because
    // the actual path is adjusted during the turn. So apply an
    // experimentally found factor in to get better estimates.
    duration *= 1.5f;

    // If the remaining estimated time for skidding is too short, stop
    // it. This code will mostly trigger the bonus at the end of a skid.
    if(m_controls->getSkidControl() && duration < 1.0f)
    {
        if(m_ai_debug)
            Log::debug(getControllerName().c_str(),
                       "'%s' too short, stop skid.",
                       m_kart->getIdent().c_str());
        return false;
    }
    // Test if the AI is trying to skid against track direction. This
    // can happen if the AI is adjusting steering somewhat (e.g. in a
    // left turn steer right to avoid getting too close to the left
    // vorder). In this case skidding will be useless.
    else if( (steer_fraction > 0 &&
              m_current_track_direction==DriveNode::DIR_LEFT) ||
             (steer_fraction < 0 &&
              m_current_track_direction==DriveNode::DIR_RIGHT)  )
        {
#ifdef DEBUG
            if(m_controls->getSkidControl() && m_ai_debug)
                Log::debug(getControllerName().c_str(),
                           "%s skidding against track direction.",
                           m_kart->getIdent().c_str());
#endif
            return false;
        }
    // If there is a skidding bonus, try to get it.
    else if (m_kart->getKartProperties()->getSkidBonusSpeed().size() > 0 &&
             m_kart->getKartProperties()->getSkidTimeTillBonus()[0] < duration)
    {
#ifdef DEBUG
        if(!m_controls->getSkidControl() && m_ai_debug)
            Log::debug(getControllerName().c_str(),
                       "%s start skid, duration %f.",
                       m_kart->getIdent().c_str(), duration);
#endif
        return true;

    }  // if curve long enough for skidding

#ifdef DEBUG
        if(m_controls->getSkidControl() && m_ai_debug)
            Log::debug(getControllerName().c_str(),
                       "%s has no reasons to skid anymore.",
                       m_kart->getIdent().c_str());
#endif
    return false;
}   // canSkid

//-----------------------------------------------------------------------------
/** Converts the steering angle to a lr steering in the range of -1 to 1.
 *  If the steering angle is too great, it will also trigger skidding. This
 *  function uses a 'time till full steer' value specifying the time it takes
 *  for the wheel to reach full left/right steering similar to player karts
 *  when using a digital input device. The parameter is defined in the kart
 *  properties and helps somewhat to make AI karts more 'pushable' (since
 *  otherwise the karts counter-steer to fast).
 *  It also takes the effect of a plunger into account by restricting the
 *  actual steer angle to 50% of the maximum.
 *  \param angle Steering angle.
 *  \param dt Time step.
 */
void SkiddingAI::setSteering(float angle, float dt)
{
    float steer_fraction = angle / m_kart->getMaxSteerAngle();

    // Use a simple finite state machine to make sure to randomly decide
    // whether to skid or not only once per skid section. See docs for
    // m_skid_probability_state for more details.
    if(!canSkid(steer_fraction))
    {
        m_skid_probability_state = SKID_PROBAB_NOT_YET;
        m_controls->setSkidControl(KartControl::SC_NONE);
    }
    else
    {
        KartControl::SkidControl sc = steer_fraction > 0
                                    ? KartControl::SC_RIGHT
                                    : KartControl::SC_LEFT;
        if(m_skid_probability_state==SKID_PROBAB_NOT_YET)
        {
            int prob = (int)(100.0f*m_ai_properties
                               ->getSkiddingProbability(m_distance_to_player));
            int r = m_random_skid.get(100);
            m_skid_probability_state = (r<prob)
                                     ? SKID_PROBAB_SKID
                                     : SKID_PROBAB_NO_SKID;
#undef PRINT_SKID_STATS
#ifdef PRINT_SKID_STATS
            Log::info(getControllerName().c_str(),
                      "%s distance %f prob %d skidding %s",
                      m_kart->getIdent().c_str(), distance, prob,
                      sc= ? "no" : sc==KartControl::SC_LEFT ? "left" : "right");
#endif
        }
        m_controls->setSkidControl(m_skid_probability_state == SKID_PROBAB_SKID
                                   ? sc : KartControl::SC_NONE );
    }

    // Adjust steer fraction in case to be in [-1,1]
    if     (steer_fraction >  1.0f) steer_fraction =  1.0f;
    else if(steer_fraction < -1.0f) steer_fraction = -1.0f;

    // Restrict steering when a plunger is in the face
    // The degree of restriction depends on item_skill

    //FIXME : the AI speed estimate in curves don't account for this restriction
    if(m_kart->getBlockedByPlungerTicks()>0)
    {
        int item_skill = computeSkill(ITEM_SKILL);
        float steering_limit = 0.5f;
        if (item_skill == 0)
            steering_limit = 0.35f;
        else if (item_skill == 1)
            steering_limit = 0.45f;
        else if (item_skill == 2)
            steering_limit = 0.55f;
        else if (item_skill == 3)
            steering_limit = 0.65f;
        else if (item_skill == 4)
            steering_limit = 0.75f;
        else if (item_skill == 5)
            steering_limit = 0.9f;

        if     (steer_fraction >  steering_limit) steer_fraction =  steering_limit;
        else if(steer_fraction < -steering_limit) steer_fraction = -steering_limit;
    }

    const Skidding *skidding = m_kart->getSkidding();

    // If we are supposed to skid, but the current steering is still
    // in the wrong direction, don't start to skid just now, since then
    // we can't turn into the direction we want to anymore (see
    // Skidding class)
    Skidding::SkidState ss = skidding->getSkidState();
    if((ss==Skidding::SKID_ACCUMULATE_LEFT  && steer_fraction>0.2f ) ||
       (ss==Skidding::SKID_ACCUMULATE_RIGHT && steer_fraction<-0.2f)    )
    {
        m_controls->setSkidControl(KartControl::SC_NONE);
#ifdef DEBUG
        if(m_ai_debug)
            Log::info(getControllerName().c_str(),
                      "'%s' wrong steering, stop skid.",
                      m_kart->getIdent().c_str());
#endif
    }

    if(m_controls->getSkidControl()!=KartControl::SC_NONE &&
            ( ss==Skidding::SKID_ACCUMULATE_LEFT ||
              ss==Skidding::SKID_ACCUMULATE_RIGHT  )  )
    {
        steer_fraction =
            skidding->getSteeringWhenSkidding(steer_fraction);
        if(fabsf(steer_fraction)>1.8)
        {
#ifdef DEBUG
            if(m_ai_debug)
                Log::info(getControllerName().c_str(),
                          "%s steering too much (%f).",
                          m_kart->getIdent().c_str(), steer_fraction);
#endif
            m_controls->setSkidControl(KartControl::SC_NONE);
        }
        if(steer_fraction<-1.0f)
            steer_fraction = -1.0f;
        else if(steer_fraction>1.0f)
            steer_fraction = 1.0f;
    }

    float old_steer      = m_controls->getSteer();

    // The AI has its own 'time full steer' value (which is the time
    float max_steer_change = dt/m_ai_properties->m_time_full_steer;
    if(old_steer < steer_fraction)
    {
        m_controls->setSteer( (old_steer+max_steer_change > steer_fraction)
                              ? steer_fraction : old_steer+max_steer_change );
    }
    else
    {
        m_controls->setSteer( (old_steer-max_steer_change < steer_fraction)
                               ? steer_fraction : old_steer-max_steer_change );
    }


}   // setSteering
