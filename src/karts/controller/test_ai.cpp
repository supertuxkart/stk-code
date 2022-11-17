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


#include "karts/controller/test_ai.hpp"

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

// This define is only used to make the TestAI as similar to the
// SkiddingAI as possible. This way the two AIs can be compared 
// without too many false and distracing positives.

#define SkiddingAI TestAI

SkiddingAI::SkiddingAI(AbstractKart *kart)
                   : AIBaseLapController(kart)
{
    reset();
    // Determine if this AI has superpowers, which happens e.g.
    // for the final race challenge against nolok.
    m_superpower = RaceManager::get()->getAISuperPower();

    m_point_selection_algorithm = PSA_DEFAULT;
    setControllerName("TestAI");

    // Use this define in order to compare the different algorithms that
    // select the next point to steer at.
#undef COMPARE_AIS
#ifdef COMPARE_AIS
    std::string name("");
    m_point_selection_algorithm = m_kart->getWorldKartId() % 2
                                ? PSA_DEFAULT : PSA_FIXED;
    switch(m_point_selection_algorithm)
    {
    case PSA_FIXED   : name = "Fixed";   break;
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
   
    //Detect if we are going to crash with the track and/or kart
    checkCrashes(m_kart->getXYZ());
    determineTrackDirection();

    // Special behaviour if we have a bomb attach: try to hit the kart ahead
    // of us.
    bool commands_set = false;
    if(m_ai_properties->m_handle_bomb &&
        m_kart->getAttachment()->getType()==Attachment::ATTACH_BOMB &&
        m_kart_ahead )
    {
        // Use nitro if the kart is far ahead, or faster than this kart
        m_controls->setNitro(m_distance_ahead>10.0f ||
                             m_kart_ahead->getSpeed() > m_kart->getSpeed());
        // If we are close enough, try to hit this kart
        if(m_distance_ahead<=10)
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
            commands_set = true;
        }
        handleRescue(dt);
    }
    if(!commands_set)
    {
        /*Response handling functions*/
        handleAcceleration(ticks);
        handleSteering(dt);
        handleItems(dt);
        handleRescue(dt);
        handleBraking();
        // If a bomb is attached, nitro might already be set.
        if(!m_controls->getNitro())
            handleNitroAndZipper();
    }
    // If we are supposed to use nitro, but have a zipper,
    // use the zipper instead (unless there are items to avoid cloe by)
    if(m_controls->getNitro() &&
        m_kart->getPowerup()->getType()==PowerupManager::POWERUP_ZIPPER &&
        m_kart->getSpeed()>1.0f &&
        m_kart->getSpeedIncreaseTicksLeft(MaxSpeed::MS_INCREASE_ZIPPER)<=0 &&
        !m_avoid_item_close)
    {
        // Make sure that not all AI karts use the zipper at the same
        // time in time trial at start up, so during the first 5 seconds
        // this is done at random only.
        if(RaceManager::get()->getMinorMode()!=RaceManager::MINOR_MODE_TIME_TRIAL ||
            (m_world->getTime()<3.0f && rand()%50==1) )
        {
            m_controls->setNitro(false);
            m_controls->setFire(true);
        }
    }

    /*And obviously general kart stuff*/
    AIBaseLapController::update(ticks);
}   // update

//-----------------------------------------------------------------------------
/** This function decides if the AI should brake.
 *  The decision can be based on race mode (e.g. in follow the leader the AI
 *  will brake if it is ahead of the leader). Otherwise it will depend on
 *  the direction the AI is facing (if it's not facing in the track direction
 *  it will brake in order to make it easier to re-align itself), and
 *  estimated curve radius (brake to avoid being pushed out of a curve).
 */
void SkiddingAI::handleBraking()
{
    m_controls->setBrake(false);
    // In follow the leader mode, the kart should brake if they are ahead of
    // the leader (and not the leader, i.e. don't have initial position 1)
    if(RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_FOLLOW_LEADER &&
        m_kart->getPosition() < m_world->getKart(0)->getPosition()           &&
        m_kart->getInitialPosition()>1                                         )
    {
#ifdef DEBUG
    if(m_ai_debug)
        Log::debug(getControllerName().c_str(), "braking: %s ahead of leader.",
                   m_kart->getIdent().c_str());
#endif

        m_controls->setBrake(true);
        return;
    }

    // A kart will not brake when the speed is already slower than this
    // value. This prevents a kart from going too slow (or even backwards)
    // in tight curves.
    const float MIN_SPEED = 5.0f;

    // If the kart is not facing roughly in the direction of the track, brake
    // so that it is easier for the kart to turn in the right direction.
    if(m_current_track_direction==DriveNode::DIR_UNDEFINED &&
        m_kart->getSpeed() > MIN_SPEED)
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
        float max_turn_speed =
            m_kart->getSpeedForTurnRadius(m_current_curve_radius);

        if(m_kart->getSpeed() > 1.5f*max_turn_speed  &&
            m_kart->getSpeed()>MIN_SPEED             &&
            fabsf(m_controls->getSteer()) > 0.95f          )
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
    const int next = m_next_node_index[m_track_node];

    float steer_angle = 0.0f;

    /*The AI responds based on the information we just gathered, using a
     *finite state machine.
     */
    //Reaction to being outside of the road
    float side_dist =
        m_world->getDistanceToCenterForKart( m_kart->getWorldKartId() );

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
        case PSA_FIXED : findNonCrashingPointFixed(&aim_point, &last_node);
                         break;
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

        // Potentially adjust the point to aim for in order to either
        // aim to collect item, or steer to avoid a bad item.
        if(m_ai_properties->m_collect_avoid_items)
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
    ItemManager* im = Track::getCurrentTrack()->getItemManager();
    while(distance < max_item_lookahead_distance)
    {
        int n_index= DriveGraph::get()->getNode(node)->getIndex();
        const std::vector<ItemState *> &items_ahead =
            im->getItemsInQuads(n_index);
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
 *  Either (low level AI) just use an item after 10 seconds, or do a much
 *  better job on higher level AI - e.g. aiming at karts ahead/behind, wait an
 *  appropriate time before using multiple items etc.
 *  \param dt Time step size.
 *  TODO: Implications of Bubble-Shield for AI's powerup-handling

 *  STATE: shield on -> avoid usage of offensive items (with certain tolerance)
 *  STATE: swatter on -> avoid usage of shield
 */
void SkiddingAI::handleItems(const float dt)
{
    m_controls->setFire(false);
    if(m_kart->getKartAnimation() ||
        m_kart->getPowerup()->getType() == PowerupManager::POWERUP_NOTHING )
        return;

    m_time_since_last_shot += dt;

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

    // Tactic 1: wait ten seconds, then use item
    // -----------------------------------------
    if(m_ai_properties->m_item_usage_skill <= 1)
    {
        if( m_time_since_last_shot > 10.0f )
        {
            m_controls->setFire(true);
            m_time_since_last_shot = 0.0f;
        }
        return;
    }

    // Tactic 2: calculate
    // -------------------
    float min_bubble_time = 2.0f;

    switch( m_kart->getPowerup()->getType() )
    {
    case PowerupManager::POWERUP_BUBBLEGUM:
        {
            Attachment::AttachmentType type = m_kart->getAttachment()->getType();
            // Don't use shield when we have a swatter.
            if( type == Attachment::ATTACH_SWATTER)
                break;

            // Check if a flyable (cake, ...) is close. If so, use bubblegum
            // as shield
            if( !m_kart->isShielded() &&
                ProjectileManager::get()->projectileIsClose(m_kart,
                                    m_ai_properties->m_shield_incoming_radius) )
            {
                m_controls->setFire(true);
                m_controls->setLookBack(false);
                break;
            }

            // Avoid dropping all bubble gums one after another
            if( m_time_since_last_shot < 3.0f) break;

            // Use bubblegum if the next kart  behind is 'close' but not too close
            // (too close likely means that the kart is not behind but more to the
            // side of this kart and so won't be hit by the bubble gum anyway).
            // Should we check the speed of the kart as well? I.e. only drop if
            // the kart behind is faster? Otoh this approach helps preventing an
            // overtaken kart to overtake us again.
            if(m_distance_behind < 15.0f && m_distance_behind > 3.0f    )
            {
                m_controls->setFire(true);
                m_controls->setLookBack(true);
                break;
            }

            // If this kart is in its last lap, drop bubble gums at every
            // opportunity, since this kart won't envounter them anymore.
            LinearWorld *lin_world = dynamic_cast<LinearWorld*>(World::getWorld());
            if(m_time_since_last_shot > 3.0f &&
                lin_world &&
                lin_world->getFinishedLapsOfKart(m_kart->getWorldKartId())
                                   == RaceManager::get()->getNumLaps()-1)
            {
                m_controls->setFire(true);
                m_controls->setLookBack(true);
                break;
            }
            break;   // POWERUP_BUBBLEGUM
        }
    case PowerupManager::POWERUP_CAKE:
        {
            // if the kart has a shield, do not break it by using a cake.
            if(m_kart->getShieldTime() > min_bubble_time)
                break;
            // Leave some time between shots
            if(m_time_since_last_shot<3.0f) break;

            // Do not fire if the kart is driving too slow
            bool kart_behind_is_slow =
                (m_kart_behind && m_kart_behind->getSpeed() < m_kart->getSpeed());
            bool kart_ahead_is_slow =
                (m_kart_ahead && m_kart_ahead->getSpeed() < m_kart->getSpeed());
            // Consider firing backwards if there is no kart ahead or it is
            // slower. Also, if the kart behind is closer and not slower than
            // this kart.
            bool fire_backwards = !m_kart_ahead ||
                                  (m_kart_behind                             &&
                                    (m_distance_behind < m_distance_ahead ||
                                     kart_ahead_is_slow                    ) &&
                                    !kart_behind_is_slow
                                  );

            // Don't fire at a kart that is slower than us. Reason is that
            // we can either save the cake for later since we will overtake
            // the kart anyway, or that this might force the kart ahead to
            // use its nitro/zipper (and then we will shoot since then the
            // kart is faster).
            if ((fire_backwards && kart_behind_is_slow) ||
                (!fire_backwards && kart_ahead_is_slow)    )
                break;

            // Don't fire if the kart we are aiming at is invulnerable.
            if ((fire_backwards  && m_kart_behind && m_kart_behind->isInvulnerable()) ||
                (!fire_backwards && m_kart_ahead && m_kart_ahead->isInvulnerable())    )
                return;

            float distance = fire_backwards ? m_distance_behind
                                            : m_distance_ahead;
            // Since cakes can be fired all around, just use a sane distance
            // with a bit of extra for backwards, as enemy will go towards cake
            m_controls->setFire( (fire_backwards && distance < 25.0f) ||
                                 (!fire_backwards && distance < 20.0f)  );
            if(m_controls->getFire())
                m_controls->setLookBack(fire_backwards);
            break;
        }   // POWERUP_CAKE

    case PowerupManager::POWERUP_BOWLING:
        {
            // if the kart has a shield, do not break it by using a bowling ball.
            if(m_kart->getShieldTime() > min_bubble_time)
                break;
            // Leave more time between bowling balls, since they are
            // slower, so it should take longer to hit something which
            // can result in changing our target.
            if(m_time_since_last_shot < 5.0f) break;
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

            // Bowling balls are slower, so only fire on closer karts - but when
            // firing backwards, the kart can be further away, since the ball
            // acts a bit like a mine (and the kart is racing towards it, too)
            bool fire_backwards = (m_kart_behind && m_kart_ahead &&
                                   m_distance_behind < m_distance_ahead) ||
                                  !m_kart_ahead;

            // Don't fire if the kart we are aiming at is invulnerable.
            if ((fire_backwards  && m_kart_behind && m_kart_behind->isInvulnerable()) ||
                (!fire_backwards && m_kart_ahead && m_kart_ahead->isInvulnerable())    )
                return;

            float distance = fire_backwards ? m_distance_behind
                                            : m_distance_ahead;
            m_controls->setFire( ( (fire_backwards && distance < 30.0f)  ||
                                   (!fire_backwards && distance <10.0f)    ) &&
                                m_time_since_last_shot > 3.0f &&
                                (straight_behind || straight_ahead)             );
            if(m_controls->getFire())
                m_controls->setLookBack(fire_backwards);
            break;
        }   // POWERUP_BOWLING

    case PowerupManager::POWERUP_ZIPPER:
        // Do nothing. Further up a zipper is used if nitro should be selected,
        // saving the (potential more valuable nitro) for later
        break;   // POWERUP_ZIPPER

    case PowerupManager::POWERUP_PLUNGER:
        {
            // if the kart has a shield, do not break it by using a plunger.
            if(m_kart->getShieldTime() > min_bubble_time)
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
        // For now don't use a switch if this kart is first (since it's more
        // likely that this kart then gets a good iteam), otherwise use it
        // after a waiting an appropriate time
        if(m_kart->getPosition()>1 &&
            m_time_since_last_shot 
            > stk_config->ticks2Time(stk_config->m_item_switch_ticks)+2.0f)
            m_controls->setFire(true);
        break;   // POWERUP_SWITCH

    case PowerupManager::POWERUP_PARACHUTE:
        // Wait one second more than a previous parachute
        if(m_time_since_last_shot > m_kart->getKartProperties()->getParachuteDurationOther() + 1.0f)
            m_controls->setFire(true);
        break;   // POWERUP_PARACHUTE

    case PowerupManager::POWERUP_ANVIL:
        // Wait one second more than a previous anvil
        if(m_time_since_last_shot < m_kart->getKartProperties()->getAnvilDuration() + 1.0f) break;

        if(RaceManager::get()->getMinorMode()==RaceManager::MINOR_MODE_FOLLOW_LEADER)
        {
            m_controls->setFire(m_world->getTime()<1.0f &&
                                m_kart->getPosition()>2    );
        }
        else
        {
            m_controls->setFire(m_time_since_last_shot > 3.0f &&
                                m_kart->getPosition()>1          );
        }
        break;   // POWERUP_ANVIL

    case PowerupManager::POWERUP_SWATTER:
        {
            // Squared distance for which the swatter works
            float d2 = m_kart->getKartProperties()->getSwatterDistance();
            // if the kart has a shield, do not break it by using a swatter.
            if(m_kart->getShieldTime() > min_bubble_time)
                break;
            // Fire if the closest kart ahead or to the back is not already
            // squashed and close enough.
            // FIXME: this can be improved on, since more than one kart might
            //        be hit, and a kart ahead might not be at an angle at
            //        which the glove can be used.
            if(  ( m_kart_ahead && !m_kart_ahead->isSquashed()             &&
                    (m_kart_ahead->getXYZ()-m_kart->getXYZ()).length2()<d2 &&
                    m_kart_ahead->getSpeed() < m_kart->getSpeed()            ) ||
                 ( m_kart_behind && !m_kart_behind->isSquashed() &&
                    (m_kart_behind->getXYZ()-m_kart->getXYZ()).length2()<d2) )
                    m_controls->setFire(true);
            break;
        }
    case PowerupManager::POWERUP_RUBBERBALL:
        // if the kart has a shield, do not break it by using a swatter.
        if(m_kart->getShieldTime() > min_bubble_time)
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

    m_distance_ahead = m_distance_behind = 9999999.9f;
    float my_dist = m_world->getOverallDistance(m_kart->getWorldKartId());
    if(m_kart_ahead)
    {
        m_distance_ahead =
            m_world->getOverallDistance(m_kart_ahead->getWorldKartId())
            -my_dist;
    }
    if(m_kart_behind)
    {
        m_distance_behind = my_dist
            -m_world->getOverallDistance(m_kart_behind->getWorldKartId());
    }

    // Compute distance to nearest player kart
    float max_overall_distance = 0.0f;
    unsigned int n = ProfileWorld::isProfileMode()
                   ? 0 : RaceManager::get()->getNumPlayers();
    for(unsigned int i=0; i<n; i++)
    {
        unsigned int kart_id =
            m_world->getPlayerKart(i)->getWorldKartId();
        if(m_world->getOverallDistance(kart_id)>max_overall_distance)
            max_overall_distance = m_world->getOverallDistance(kart_id);
    }
    if(max_overall_distance==0.0f)
        max_overall_distance = 999999.9f;   // force best driving
    // Now convert 'maximum overall distance' to distance to player.
    m_distance_to_player =
                m_world->getOverallDistance(m_kart->getWorldKartId())
                - max_overall_distance;
}   // computeNearestKarts

//-----------------------------------------------------------------------------
/** Determines if the AI should accelerate or not.
 *  \param dt Time step size.
 */
void SkiddingAI::handleAcceleration(int ticks)
{
    //Do not accelerate until we have delayed the start enough
    if( m_start_delay > 0 )
    {
        m_start_delay -= ticks;
        m_controls->setAccel(0.0f);
        return;
    }

    // FIXME: This test appears to be incorrect, since at this stage
    // m_brake has not been reset from the previous frame, which can
    // cause too long slow downs. On the other hand removing it appears
    // to decrease performance in some narrower tracks
    if( m_controls->getBrake())
    {
        m_controls->setAccel(0.0f);
        return;
    }

    if(m_kart->getBlockedByPlungerTicks()>0)
    {
        if(m_kart->getSpeed() < m_kart->getCurrentMaxSpeed() / 2)
            m_controls->setAccel(0.05f);
        else
            m_controls->setAccel(0.0f);
        return;
    }

    m_controls->setAccel(stk_config->m_ai_acceleration);

}   // handleAcceleration

//-----------------------------------------------------------------------------
void SkiddingAI::handleRaceStart()
{
    if( m_start_delay <  0 )
    {
        // Each kart starts at a different, random time, and the time is
        // smaller depending on the difficulty.
        m_start_delay = stk_config->time2Ticks(
                        m_ai_properties->m_min_start_delay
                      + (float) rand() / RAND_MAX
                      * (m_ai_properties->m_max_start_delay -
                         m_ai_properties->m_min_start_delay)   );

        float false_start_probability =
               m_superpower == RaceManager::SUPERPOWER_NOLOK_BOSS
               ? 0.0f  : m_ai_properties->m_false_start_probability;

        // Now check for a false start. If so, add 1 second penalty time.
        if (rand() < RAND_MAX * false_start_probability)
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
/** Decides wether to use nitro or not.
 */
void SkiddingAI::handleNitroAndZipper()
{
    m_controls->setNitro(false);

    // The next line prevents usage of nitro on long straights, where the kart
    // is already fast.

    // If we are already very fast, save nitro.
    if(m_kart->getSpeed() > 0.95f*m_kart->getCurrentMaxSpeed())
        return;
    // About the above: removing this line might enable the AI to better use
    // nitro and zippers.
    // This works well for some tracks (math, lighthouse
    // sandtrack), but worse in others (zen, xr591). The good result
    // is caused by long straights in which the AI will now use zippers,
    // but in the other tracks the high speed causes karts to crash in
    // tight corners. In some cases it would need a reasonable large 'lookahead'
    // to know that at a certain spot a zipper or skidding should not be used
    // (e.g. in xr591, left at the fork - new AI will nearly always fall off the
    // track after the curve). Here the summarised results of 10 laps runs with 
    // 4 old against 4 new AIs in time trail mode. The 'gain' is the sum of all
    // (star_positions-end_positions). So a gain of -10 means that basically
    // the new AI fell from start positions 2,4,6,8 to end positions 6,7,8,9.
    // On the other hand, +10 means the new AI took positions 1,2,3,4. 
    // name                gain     Time              Skid  Resc Rsc  Brake Expl Exp Itm Ban SNitLNit Bub Off Energy
    // xr591               -10      504.279           204.70 0.00  15  4011 0.00   0   0  26 135   0   0  4881 0.00
    // zengarden           -10      287.496           141.81 0.00  13  2489 0.00   0   0   7   2   0   0  5683 0.00
    // cocoa_temple         -6      511.008           170.67 0.00  17  3112 0.00   0   0   5  77  10   0  5405 0.00
    // mansion              -5      358.171           130.34 0.00  38  8077 0.00   0   0  28 100   0   0 13050 1.00
    // farm                 -4      332.542            60.61 0.00   1   390 0.00   0   0  44 118  15   0   547 2.74
    // mines                -4      488.225           233.00 0.00  30  1785 0.00   2   0   6 153   0   0  7661 0.00
    // snowmountain         -4      430.525           160.22 0.00   4   681 0.00   0   0   5 104  30   0  1842 0.00
    // city                 -3      491.625            60.69 0.00   3  1143 0.00   0   0   9 119  31   0  1075 0.69
    // greenvalley          -2      652.688            60.41 0.00  22  4292 0.00   0   0  19 133  48   0  7610 1.89
    // snowtuxpeak           0      405.667           163.87 0.00   6  1420 0.00   0   0   7  28   0   0  7984 0.00
    // stk_enterprise        2      576.479           104.78 0.00   5  2549 0.00   0   0   7 250   5   0  6293 0.00
    // fortmagma             3      432.117           179.53 0.00  14  3244 0.00   2   0   0  89   0   0 13091 0.00
    // scotland              3      444.075           255.65 0.00   3   308 0.00   0   0   0 133   0   0  1353 0.00
    // abyss                 7      480.771           226.85 0.00   1   163 0.00   0   0   2   2   0   0  2684 0.00
    // hacienda              7      442.142           188.21 0.00   4   863 0.00   0   0   0 145   0   0  2731 0.00
    // gran_paradiso_island  9      508.292           247.99 0.00   2  1277 0.00   0   0   5 103   0   0  2179 0.00
    // lighthouse           10      316.346           194.38 0.00   0  1192 0.00   0   0   0  69   0   0  3062 0.00
    // olivermath           10          188            83.93 0.00   1  2498 0.00   0   0   0  54   0   0  2072 1.00
    // sandtrack            10      489.963            64.18 0.00   3  1009 0.00   0   0   1 135   0   0  1407 0.00

    // Don't use nitro when the AI has a plunger in the face!
    if(m_kart->getBlockedByPlungerTicks()>0) return;

    // Don't use nitro if we are braking
    if(m_controls->getBrake()) return;

    // Don't use nitro if the kart is not on ground or has finished the race
    if(!m_kart->isOnGround() || m_kart->hasFinishedRace()) return;

    // Don't compute nitro usage if we don't have nitro or are not supposed
    // to use it, and we don't have a zipper or are not supposed to use
    // it (calculated).
    if( (m_kart->getEnergy()==0 ||
        m_ai_properties->m_nitro_usage == 0)  &&
        (m_kart->getPowerup()->getType()!=PowerupManager::POWERUP_ZIPPER ||
         m_ai_properties->m_item_usage_skill <= 1 )                         )
        return;

    // If there are items to avoid close, and we only have zippers, don't
    // use them (since this make it harder to avoid items).
    if(m_avoid_item_close &&
        (m_kart->getEnergy()==0 ||
         m_ai_properties->m_nitro_usage == 0) )
        return;
    // If a parachute or anvil is attached, the nitro doesn't give much
    // benefit. Better wait till later.
    const bool has_slowdown_attachment =
        m_kart->getAttachment()->getType()==Attachment::ATTACH_PARACHUTE ||
        m_kart->getAttachment()->getType()==Attachment::ATTACH_ANVIL;
    if(has_slowdown_attachment) return;

    // If the kart is very slow (e.g. after rescue), use nitro
    if(m_kart->getSpeed()<5)
    {
        m_controls->setNitro(true);
        return;
    }

    // If this kart is the last kart, and we have enough
    // (i.e. more than 2) nitro, use it.
    // -------------------------------------------------
    const unsigned int num_karts = m_world->getCurrentNumKarts();
    if(m_kart->getPosition()== (int)num_karts &&
        num_karts>1 && m_kart->getEnergy()>2.0f)
    {
        m_controls->setNitro(true);
        return;
    }

    // On the last track shortly before the finishing line, use nitro
    // anyway. Since the kart is faster with nitro, estimate a 50% time
    // decrease (additionally some nitro will be saved when top speed
    // is reached).
    if(m_world->getLapForKart(m_kart->getWorldKartId())
                        ==RaceManager::get()->getNumLaps()-1 &&
       m_ai_properties->m_nitro_usage >= 2)
    {
        float finish =
            m_world->getEstimatedFinishTime(m_kart->getWorldKartId());
        if( 1.5f*m_kart->getEnergy() >= finish - m_world->getTime() )
        {
            m_controls->setNitro(true);
            return;
        }
    }

    // A kart within this distance is considered to be overtaking (or to be
    // overtaken).
    const float overtake_distance = 10.0f;

    // Try to overtake a kart that is close ahead, except
    // when we are already much faster than that kart
    // --------------------------------------------------
    if(m_kart_ahead                                       &&
        m_distance_ahead < overtake_distance              &&
        m_kart_ahead->getSpeed()+5.0f > m_kart->getSpeed()   )
    {
            m_controls->setNitro(true);
            return;
    }

    if(m_kart_behind                                   &&
        m_distance_behind < overtake_distance          &&
        m_kart_behind->getSpeed() > m_kart->getSpeed()    )
    {
        // Only prevent overtaking on highest level
        m_controls->setNitro(m_ai_properties->m_nitro_usage >= 2 );
        return;
    }

    if(m_kart->getPowerup()->getType()==PowerupManager::POWERUP_ZIPPER &&
        m_kart->getSpeed()>1.0f &&
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
    if(m_ai_properties->m_make_use_of_slipstream &&
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
/** Find the sector that at the longest distance from the kart, that can be
 *  driven to without crashing with the track, then find towards which of
 *  the two edges of the track is closest to the next curve afterwards,
 *  and return the position of that edge.
 *  \param aim_position The point to aim for, i.e. the point that can be
 *         driven to in a straight line.
 *  \param last_node The graph node index in which the aim_position is.
 */
void SkiddingAI::findNonCrashingPointFixed(Vec3 *aim_position, int *last_node)
{
#ifdef AI_DEBUG_KART_HEADING
    const Vec3 eps(0,0.5f,0);
    m_curve[CURVE_KART]->clear();
    m_curve[CURVE_KART]->addPoint(m_kart->getXYZ()+eps);
    Vec3 forw(0, 0, 50);
    m_curve[CURVE_KART]->addPoint(m_kart->getTrans()(forw)+eps);
#endif
    *last_node = m_next_node_index[m_track_node];

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
                 > DriveGraph::get()->getNode(*last_node)->getPathWidth()*0.5f )
            {
                *aim_position = DriveGraph::get()->getNode(*last_node)
                                                ->getCenter();
                return;
            }
        }
        *last_node = target_sector;
    }   // for i<100
    *aim_position = DriveGraph::get()->getNode(*last_node)->getCenter();
}   // findNonCrashingPointFixed

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
    if(m_kart->getBlockedByPlungerTicks()>0)
    {
        if     (steer_fraction >  0.5f) steer_fraction =  0.5f;
        else if(steer_fraction < -0.5f) steer_fraction = -0.5f;
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
