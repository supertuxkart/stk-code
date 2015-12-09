//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006-2007 Eduardo Hernandez Munoz
//  Copyright (C) 2008-2012 Joerg Henrichs
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

#include "karts/controller/battle_ai.hpp"

#include "items/attachment.hpp"
#include "items/item_manager.hpp"
#include "items/powerup.hpp"
#include "items/projectile_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/controller/kart_control.hpp"
#include "karts/controller/player_controller.hpp"
#include "karts/controller/ai_properties.hpp"
#include "karts/kart_properties.hpp"
#include "karts/max_speed.hpp"
#include "karts/rescue_animation.hpp"
#include "karts/skidding.hpp"
#include "modes/three_strikes_battle.hpp"
#include "tracks/nav_poly.hpp"
#include "tracks/navmesh.hpp"
#include "utils/log.hpp"

#ifdef AI_DEBUG
#include "irrlicht.h"
#include <iostream>
using namespace irr;
using namespace std;
#endif

#if defined(WIN32) && !defined(__CYGWIN__) && !defined(__MINGW32__)
#define isnan _isnan
#else
#include <math.h>
#endif

BattleAI::BattleAI(AbstractKart *kart,
                   StateManager::ActivePlayer *player)
         : AIBaseController(kart, player)
{

    reset();

#ifdef AI_DEBUG
    video::SColor col_debug(128, 128, 0, 0);
    m_debug_sphere = irr_driver->addSphere(1.0f, col_debug);
    m_debug_sphere->setVisible(true);
#endif

    if (race_manager->getMinorMode() == RaceManager::MINOR_MODE_3_STRIKES)
    {
        m_world     = dynamic_cast<ThreeStrikesBattle*>(World::getWorld());
        m_track     = m_world->getTrack();
    }
    else
    {
        // Those variables are not defined in a battle mode (m_world is
        // a linear world, since it assumes the existance of drivelines)
        m_world           = NULL;
        m_track           = NULL;
    }

    // Don't call our own setControllerName, since this will add a
    // billboard showing 'AIBaseController' to the kart.
    Controller::setControllerName("BattleAI");

}   // BattleAI

//-----------------------------------------------------------------------------

BattleAI::~BattleAI()
{
#ifdef AI_DEBUG
    irr_driver->removeNode(m_debug_sphere);
#endif
}   //  ~BattleAI

//-----------------------------------------------------------------------------
/** Resets the AI when a race is restarted.
 */
void BattleAI::reset()
{
    m_current_node = BattleGraph::UNKNOWN_POLY;
    m_target_node = BattleGraph::UNKNOWN_POLY;
    m_closest_kart = NULL;
    m_closest_kart_node = BattleGraph::UNKNOWN_POLY;
    m_closest_kart_point = Vec3(0, 0, 0);
    m_closest_kart_pos_data = {0};
    m_cur_kart_pos_data = {0};
    m_is_stuck = false;
    m_is_uturn = false;
    m_target_point = Vec3(0, 0, 0);
    m_time_since_last_shot = 0.0f;
    m_time_since_driving = 0.0f;
    m_time_since_reversing = 0.0f;
    m_time_since_uturn = 0.0f;
    m_on_node.clear();
    m_path_corners.clear();
    m_portals.clear();

    m_cur_difficulty = race_manager->getDifficulty();
    AIBaseController::reset();
}   // reset

//-----------------------------------------------------------------------------
/** This is the main entry point for the AI.
 *  It is called once per frame for each AI and determines the behaviour of
 *  the AI, e.g. steering, accelerating/braking, firing.
 */
void BattleAI::update(float dt)
{
    // This is used to enable firing an item backwards.
    m_controls->m_look_back = false;
    m_controls->m_nitro     = false;

    // Don't do anything if there is currently a kart animations shown.
    if (m_kart->getKartAnimation())
        return;

    if (m_world->isStartPhase())
    {
        AIBaseController::update(dt);
        return;
    }

    checkIfStuck(dt);
    if (m_is_stuck && !m_is_uturn)
    {
        setSteering(0.0f, dt);

        if (fabsf(m_kart->getSpeed()) >
            (m_kart->getKartProperties()->getEngineMaxSpeed() / 5)
            && m_kart->getSpeed() < 0)
            m_controls->m_accel = -0.06f;
        else
            m_controls->m_accel = -4.0f;

        m_time_since_reversing += dt;

        if (m_time_since_reversing >= 1.0f)
        {
            m_is_stuck = false;
            m_time_since_reversing = 0.0f;
        }
        AIBaseController::update(dt);
        return;
    }

    findClosestKart(true);
    findTarget();
    handleItems(dt);

    if (m_kart->getSpeed() > 15.0f && m_cur_kart_pos_data.angle < 0.2f)
    {
        // Only use nitro when target is straight
        m_controls->m_nitro = true;
    }

    if (m_is_uturn)
    {
        handleUTurn(dt);
    }
    else
    {
        handleAcceleration(dt);
        handleSteering(dt);
        handleBraking();
    }

    AIBaseController::update(dt);

}   // update

//-----------------------------------------------------------------------------
void BattleAI::checkIfStuck(const float dt)
{
    if (m_is_stuck) return;

    if (m_kart->getKartAnimation() || m_world->isStartPhase())
    {
        m_on_node.clear();
        m_time_since_driving = 0.0f;
    }

    m_on_node.insert(m_current_node);
    m_time_since_driving += dt;

    if ((m_time_since_driving >=
        (m_cur_difficulty == RaceManager::DIFFICULTY_EASY ? 2.0f : 1.5f)
        && m_on_node.size() < 2 && !m_is_uturn &&
        fabsf(m_kart->getSpeed()) < 3.0f) || isStuck() == true)
    {
        // Check whether a kart stay on the same node for a period of time
        // Or crashed 3 times
        m_on_node.clear();
        m_time_since_driving = 0.0f;
        AIBaseController::reset();
        m_is_stuck = true;
    }
    else if (m_time_since_driving >=
        (m_cur_difficulty == RaceManager::DIFFICULTY_EASY ? 2.0f : 1.5f))
    {
        m_on_node.clear(); // Reset for any correct movement
        m_time_since_driving = 0.0f;
    }

}   //  checkIfStuck

//-----------------------------------------------------------------------------
void BattleAI::checkPosition(const Vec3 &point, posData *pos_data)
{
    // Convert to local coordinates from the point of view of current kart
    btQuaternion q(btVector3(0, 1, 0), -m_kart->getHeading());
    Vec3 p  = point - m_kart->getXYZ();
    Vec3 local_coordinates = quatRotate(q, p);

    // on_side: tell whether it's left or right hand side
    if (local_coordinates.getX() < 0)
        pos_data->on_side = true;
    else
        pos_data->on_side = false;

    // behind: tell whether it's behind or not
    if (local_coordinates.getZ() < 0)
        pos_data->behind = true;
    else
        pos_data->behind = false;

    pos_data->angle = atan2(fabsf(local_coordinates.getX()),
        fabsf(local_coordinates.getZ()));
    pos_data->distance = p.length_2d();
}   //  checkPosition

//-----------------------------------------------------------------------------
void BattleAI::findClosestKart(bool difficulty)
{
    float distance = 99999.9f;
    const unsigned int n = m_world->getNumKarts();
    int closest_kart_num = 0;

    for (unsigned int i = 0; i < n; i++)
    {
        const AbstractKart* kart = m_world->getKart(i);
        if (kart->isEliminated()) continue;

        if (kart->getWorldKartId() == m_kart->getWorldKartId())
            continue; // Skip the same kart

        // Test whether takes current difficulty into account for closest kart
        // Notice: it don't affect aiming, this function will be called once
        // more in handleItems, which ignore difficulty.
        if (m_cur_difficulty == RaceManager::DIFFICULTY_EASY && difficulty)
        {
            // Skip human players for novice mode unless only human players left
            const AbstractKart* temp = m_world->getKart(i);
            if (temp->getController()->isPlayerController() &&
               (m_world->getCurrentNumKarts() -
                m_world->getCurrentNumPlayers()) > 1)
                continue;
        }
        else if (m_cur_difficulty == RaceManager::DIFFICULTY_BEST && difficulty)
        {
            // Skip AI players for supertux mode
            const AbstractKart* temp = m_world->getKart(i);
            if (!(temp->getController()->isPlayerController()))
                continue;
        }

        Vec3 d = kart->getXYZ() - m_kart->getXYZ();
        if (d.length_2d() <= distance)
        {
            distance = d.length_2d();
            closest_kart_num = i;
        }
    }

    const AbstractKart* closest_kart = m_world->getKart(closest_kart_num);
    if (!closest_kart->getController()->isPlayerController())
    {
        BattleAI* controller = (BattleAI*)(closest_kart->getController());
        m_closest_kart_node = controller->getCurrentNode();
        m_closest_kart_point = closest_kart->getXYZ();
    }

    else if (closest_kart->getController()->isPlayerController())
    {
        PlayerController* controller = (PlayerController*)(closest_kart->getController());
        m_closest_kart_node = controller->getCurrentNode();
        m_closest_kart_point = closest_kart->getXYZ();
    }

    if (!difficulty)
    {
        m_closest_kart = m_world->getKart(closest_kart_num);
        checkPosition(m_closest_kart_point, &m_closest_kart_pos_data);
    }
}   // findClosestKart

//-----------------------------------------------------------------------------
void BattleAI::findTarget()
{
    // Find a suitable target to drive to, either powerup or kart
    if (m_kart->getPowerup()->getType() == PowerupManager::POWERUP_NOTHING &&
        m_kart->getAttachment()->getType() != Attachment::ATTACH_SWATTER)
        handleItemCollection(&m_target_point , &m_target_node);
    else
    {
        m_target_point = m_closest_kart_point;
        m_target_node  = m_closest_kart_node;
    }
}   // findTarget

//-----------------------------------------------------------------------------
/** Handles acceleration.
 *  \param dt Time step size.
 */
void BattleAI::handleAcceleration(const float dt)
{
    if (m_controls->m_brake)
    {
        m_controls->m_accel = 0.0f;
        return;
    }

    const float handicap =
        (m_cur_difficulty == RaceManager::DIFFICULTY_EASY ? 0.7f : 1.0f);
    m_controls->m_accel = stk_config->m_ai_acceleration * handicap;

}   // handleAcceleration

//-----------------------------------------------------------------------------
void BattleAI::handleUTurn(const float dt)
{
    const float turn_side = (m_cur_kart_pos_data.on_side ? 1.0f : -1.0f);

    if (fabsf(m_kart->getSpeed()) >
        (m_kart->getKartProperties()->getEngineMaxSpeed() / 5)
        && m_kart->getSpeed() < 0)    // Try to emulate reverse like human players
        m_controls->m_accel = -0.06f;
    else
        m_controls->m_accel = -5.0f;

    if (m_time_since_uturn >=
        (m_cur_difficulty == RaceManager::DIFFICULTY_EASY ? 2.0f : 1.5f))
        setSteering(-(turn_side), dt); // Preventing keep going around circle
    else
        setSteering(turn_side, dt);
    m_time_since_uturn += dt;

    checkPosition(m_target_point, &m_cur_kart_pos_data);
    if (!m_cur_kart_pos_data.behind || m_time_since_uturn >
        (m_cur_difficulty == RaceManager::DIFFICULTY_EASY ? 3.5f : 3.0f))
    {
        m_is_uturn = false;
        m_time_since_uturn = 0.0f;
    }
    else
        m_is_uturn = true;
}   // handleUTurn

//-----------------------------------------------------------------------------
/** This function sets the steering.
 *  \param dt Time step size.
 */
void BattleAI::handleSteering(const float dt)
{
    if (m_current_node == BattleGraph::UNKNOWN_POLY ||
        m_target_node == BattleGraph::UNKNOWN_POLY) return;

    if (m_target_node == m_current_node)
    {
        // Very close to the item, steer directly
        checkPosition(m_target_point, &m_cur_kart_pos_data);
#ifdef AI_DEBUG
        m_debug_sphere->setPosition(m_target_point.toIrrVector());
#endif
        if (m_cur_kart_pos_data.behind)
        {
            m_is_uturn = true;
        }
        else
        {
            float target_angle = steerToPoint(m_target_point);
            setSteering(target_angle, dt);
        }
        return;
    }

    else if (m_target_node != m_current_node)
    {
        findPortals(m_current_node, m_target_node);
        stringPull(m_kart->getXYZ(), m_target_point);
        if (m_path_corners.size() > 0)
            m_target_point = m_path_corners[0];

        checkPosition(m_target_point, &m_cur_kart_pos_data);
#ifdef AI_DEBUG
        m_debug_sphere->setPosition(m_target_point.toIrrVector());
#endif
        if (m_cur_kart_pos_data.behind)
        {
            m_is_uturn = true;
        }
        else
        {
            float target_angle = steerToPoint(m_target_point);
            setSteering(target_angle, dt);
        }
        return;
    }

    else
    {
        // Do nothing (go straight) if no targets found
        setSteering(0.0f, dt);
        return;
    }
}   // handleSteering

//-----------------------------------------------------------------------------
/** This function finds the polyon edges(portals) that the AI will cross before
 *  reaching its destination. We start from the current polygon and call
 *  BattleGraph::getNextShortestPathPoly() to find the next polygon on the shortest
 *  path to the destination. Then find the common edge between the current
 *  poly and the next poly, store it and step through the channel.
 *
 *       1----2----3            In this case, the portals are:
 *       |strt|    |            (2,5) (4,5) (10,7) (10,9) (11,12)
 *       6----5----4
 *            |    |
 *            7----10----11----14
 *            |    |     | end |
 *            8----9-----12----13
 *
 *  \param start The start node(polygon) of the channel.
 *  \param end The end node(polygon) of the channel.
 */
void BattleAI::findPortals(int start, int end)
{
    int this_node = start;

    // We can't use NULL because NULL==0 which is a valid node, so we initialize
    // with a value that is always invalid.
    int next_node = -999;

    m_portals.clear();

    while (next_node != end && this_node != -1 && next_node != -1 && this_node != end)
    {
        next_node = BattleGraph::get()->getNextShortestPathPoly(this_node, end);
        if (next_node == BattleGraph::UNKNOWN_POLY || next_node == -999) return;

        std::vector<int> this_node_verts =
                         NavMesh::get()->getNavPoly(this_node).getVerticesIndex();
        std::vector<int> next_node_verts=
                         NavMesh::get()->getNavPoly(next_node).getVerticesIndex();

        // this_node_verts and next_node_verts hold vertices of polygons in CCW order
        // We reverse next_node_verts so it becomes easy to compare edges in the next step
        std::reverse(next_node_verts.begin(),next_node_verts.end());

        Vec3 portalLeft, portalRight;
        //bool flag = 0;
        for (unsigned int n_i = 0; n_i < next_node_verts.size(); n_i++)
        {
            for (unsigned int t_i = 0; t_i < this_node_verts.size(); t_i++)
            {
                if ((next_node_verts[n_i] == this_node_verts[t_i]) &&
                    (next_node_verts[(n_i+1)%next_node_verts.size()] ==
                     this_node_verts[(t_i+1)%this_node_verts.size()]))
                {
                     portalLeft = NavMesh::get()->
                         getVertex(this_node_verts[(t_i+1)%this_node_verts.size()]);

                     portalRight = NavMesh::get()->getVertex(this_node_verts[t_i]);
                }
            }
        }
        m_portals.push_back(std::make_pair(portalLeft, portalRight));
        // for debugging:
        //m_debug_sphere->setPosition((portalLeft).toIrrVector());
        this_node = next_node;
    }
}   // findPortals

//-----------------------------------------------------------------------------
/** This function implements the funnel algorithm for finding shortest paths
 *  through a polygon channel. This means that we should move from corner to
 *  corner to move on the most straight and shortest path to the destination.
 *  This can be visualized as pulling a string from the end point to the start.
 *  The string will bend at the corners, and this algorithm will find those
 *  corners using portals from findPortals(). The AI will aim at the first
 *  corner and the rest can be used for estimating the curve (braking).
 *
 *       1----2----3            In this case, the corners are:
 *       |strt|    |            <5,10,end>
 *       6----5----4
 *            |    |
 *            7----10----11----14
 *            |    |     | end |
 *            8----9-----12----13
 *
 *  \param start_pos The start position (usually the AI's current position).
 *  \param end_pos The end position (m_target_point).
 */
void BattleAI::stringPull(const Vec3& start_pos, const Vec3& end_pos)
{
    Vec3 funnel_apex = start_pos;
    Vec3 funnel_left = m_portals[0].first;
    Vec3 funnel_right = m_portals[0].second;
    unsigned int apex_index=0, fun_left_index=0, fun_right_index=0;
    m_portals.push_back(std::make_pair(end_pos, end_pos));
    m_path_corners.clear();
    const float eps=0.0001f;

    for (unsigned int i = 0; i < m_portals.size(); i++)
    {
        Vec3 portal_left = m_portals[i].first;
        Vec3 portal_right = m_portals[i].second;

        //Compute for left edge
        if ((funnel_left == funnel_apex) ||
             portal_left.sideOfLine2D(funnel_apex, funnel_left) <= -eps)
        {
            funnel_left = 0.98f*portal_left + 0.02f*portal_right;
            //funnel_left = portal_left;
            fun_left_index = i;

            if (portal_left.sideOfLine2D(funnel_apex, funnel_right) < -eps)
            {
                funnel_apex = funnel_right;
                apex_index = fun_right_index;
                m_path_corners.push_back(funnel_apex);

                funnel_left = funnel_apex;
                funnel_right = funnel_apex;
                i = apex_index;
                continue;
            }
        }

        //Compute for right edge
        if ((funnel_right == funnel_apex) ||
             portal_right.sideOfLine2D(funnel_apex, funnel_right) >= eps)
        {
            funnel_right = 0.98f*portal_right + 0.02f*portal_left;
            //funnel_right = portal_right;
            fun_right_index = i;

            if (portal_right.sideOfLine2D(funnel_apex, funnel_left) > eps)
            {
                funnel_apex = funnel_left;
                apex_index = fun_left_index;
                m_path_corners.push_back(funnel_apex);

                funnel_left = funnel_apex;
                funnel_right = funnel_apex;
                i = apex_index;
                continue;
            }
        }

    }

    //Push end_pos to m_path_corners so if no corners, we aim at target
    m_path_corners.push_back(end_pos);
}   // stringPull

//-----------------------------------------------------------------------------
/** This function handles braking. It calls determineTurnRadius() to find out
 *  the curve radius. Depending on the turn radius, it finds out the maximum
 *  speed. If the current speed is greater than the max speed and a set minimum
 *  speed, brakes are applied.
 */
void BattleAI::handleBraking()
{
    m_controls->m_brake = false;

    // A kart will not brake when the speed is already slower than this
    // value. This prevents a kart from going too slow (or even backwards)
    // in tight curves.
    const float MIN_SPEED = 5.0f;

    std::vector<Vec3> points;

    if (m_current_node == BattleGraph::UNKNOWN_POLY ||
        m_target_node == BattleGraph::UNKNOWN_POLY) return;

    points.push_back(m_kart->getXYZ());
    points.push_back(m_path_corners[0]);
    points.push_back((m_path_corners.size()>=2) ? m_path_corners[1] : m_path_corners[0]);

    float current_curve_radius = BattleAI::determineTurnRadius(points);

    Vec3 d1 = m_kart->getXYZ() - m_target_point;
    Vec3 d2 = m_kart->getXYZ() - m_path_corners[0];
    if (d1.length2_2d() < d2.length2_2d())
        current_curve_radius = d1.length_2d();

    float max_turn_speed = m_kart->getSpeedForTurnRadius(current_curve_radius);

    if (m_kart->getSpeed() > max_turn_speed &&
        m_kart->getSpeed() > MIN_SPEED)
    {
        m_controls->m_brake = true;
    }

}   // handleBraking

//-----------------------------------------------------------------------------
/** The turn radius is determined by fitting a parabola to 3 points: current
 *  location of AI, first corner and the second corner. Once the constants are
 *  computed, a formula is used to find the radius of curvature at the kart's
 *  current location.
 *  NOTE: This method does not apply enough braking, should think of something
 *  else.
 */
float BattleAI::determineTurnRadius( std::vector<Vec3>& points )
{
    // Declaring variables
    float a, b;
    irr::core::CMatrix4<float> A;
    irr::core::CMatrix4<float> X;
    irr::core::CMatrix4<float> B;

    //Populating matrices
    for (unsigned int i = 0; i < 3; i++)
    {
        A(i, 0) = points[i].x()*points[i].x();
        A(i, 1) = points[i].x();
        A(i, 2) = 1.0f;
        A(i, 3) = 0.0f;
    }
    A(3, 0) = A(3, 1) = A(3, 2) = 0.0f;
    A(3, 3) = 1.0f;

    for (unsigned int i = 0; i < 3; i++)
    {
        B(i, 0) = points[i].z();
        B(i, 1) = 0.0f;
        B(i, 2) = 0.0f;
        B(i, 3) = 0.0f;
    }
    B(3, 0) = B(3, 1) = B(3, 2) = B(3, 3) = 0.0f;

    //Computing inverse : X = inv(A)*B
    irr::core::CMatrix4<float> invA;
    if (!A.getInverse(invA))
        return -1;

    X = invA*B;
    a = X(0, 0);
    b = X(0, 1);
    //c = X(0, 2);

    float x = points.front().x();
    //float z = a*pow(x, 2) + b*x + c;
    float dx_by_dz = 2*a*x + b;
    float d2x_by_dz = 2*a;

    float radius = pow(abs(1 + pow(dx_by_dz, 2)), 1.5f)/ abs(d2x_by_dz);

    return radius;
}   // determineTurnRadius

//-----------------------------------------------------------------------------
void BattleAI::handleItems(const float dt)
{
    m_controls->m_fire = false;
    if (m_kart->getKartAnimation() ||
        m_kart->getPowerup()->getType() == PowerupManager::POWERUP_NOTHING)
        return;

    // Find a closest kart again, this time we ignore difficulty
    findClosestKart(false);

    if (!m_closest_kart) return;

    m_time_since_last_shot += dt;

    float min_bubble_time = 2.0f;
    const bool difficulty = m_cur_difficulty == RaceManager::DIFFICULTY_EASY ||
                            m_cur_difficulty == RaceManager::DIFFICULTY_MEDIUM;

    const bool fire_behind = m_closest_kart_pos_data.behind && !difficulty;

    const bool perfect_aim = m_closest_kart_pos_data.angle < 0.2f;

    switch(m_kart->getPowerup()->getType())
    {
    case PowerupManager::POWERUP_BUBBLEGUM:
        {
            Attachment::AttachmentType type = m_kart->getAttachment()->getType();
            // Don't use shield when we have a swatter.
            if (type == Attachment::ATTACH_SWATTER       ||
                type == Attachment::ATTACH_NOLOKS_SWATTER)
                break;

            // Check if a flyable (cake, ...) is close. If so, use bubblegum
            // as shield
            if (!m_kart->isShielded() &&
                projectile_manager->projectileIsClose(m_kart,
                                    m_ai_properties->m_shield_incoming_radius))
            {
                m_controls->m_fire      = true;
                m_controls->m_look_back = false;
                break;
            }

            // Avoid dropping all bubble gums one after another
            if (m_time_since_last_shot < 3.0f) break;

            // Use bubblegum if the next kart behind is 'close' but not too close,
            // or can't find a close kart for too long time
            if ((m_closest_kart_pos_data.distance < 15.0f &&
                m_closest_kart_pos_data.distance > 3.0f) ||
                m_time_since_last_shot > 15.0f)
            {
                m_controls->m_fire      = true;
                m_controls->m_look_back = true;
                break;
            }

            break;   // POWERUP_BUBBLEGUM
        }
    case PowerupManager::POWERUP_CAKE:
        {
            // if the kart has a shield, do not break it by using a cake.
            if (m_kart->getShieldTime() > min_bubble_time)
                break;

            // Leave some time between shots
            if (m_time_since_last_shot < 1.0f) break;

            if (m_closest_kart_pos_data.distance < 25.0f &&
                !m_closest_kart->isInvulnerable())
            {
                m_controls->m_fire      = true;
                m_controls->m_look_back = fire_behind;
                break;
            }

            break;
        }   // POWERUP_CAKE

    case PowerupManager::POWERUP_BOWLING:
        {
            // if the kart has a shield, do not break it by using a bowling ball.
            if (m_kart->getShieldTime() > min_bubble_time)
                break;

            // Leave some time between shots
            if (m_time_since_last_shot < 1.0f) break;

            if (m_closest_kart_pos_data.distance < 6.0f &&
                (difficulty || perfect_aim))
            {
                m_controls->m_fire      = true;
                m_controls->m_look_back = fire_behind;
                break;
            }

            break;
        }   // POWERUP_BOWLING

    case PowerupManager::POWERUP_SWATTER:
        {
            // Squared distance for which the swatter works
            float d2 = m_kart->getKartProperties()->getSwatterDistance();
            // if the kart has a shield, do not break it by using a swatter.
            if (m_kart->getShieldTime() > min_bubble_time)
                break;

            if (!m_closest_kart->isSquashed()          &&
                 m_closest_kart_pos_data.distance < d2 &&
                 m_closest_kart->getSpeed() < m_kart->getSpeed())
            {
                m_controls->m_fire      = true;
                m_controls->m_look_back = false;
                break;
            }
            break;
        }

    // Below powerups won't appear in battle mode, so skip them
    case PowerupManager::POWERUP_ZIPPER:
        break;   // POWERUP_ZIPPER

    case PowerupManager::POWERUP_PLUNGER:
        break;   // POWERUP_PLUNGER

    case PowerupManager::POWERUP_SWITCH: // Don't handle switch
        m_controls->m_fire = true;       // (use it no matter what) for now
        break;   // POWERUP_SWITCH

    case PowerupManager::POWERUP_PARACHUTE:
        break;   // POWERUP_PARACHUTE

    case PowerupManager::POWERUP_ANVIL:
        break;   // POWERUP_ANVIL

    case PowerupManager::POWERUP_RUBBERBALL:
        break;

    default:
        Log::error("BattleAI",
                "Invalid or unhandled powerup '%d' in default AI.",
                m_kart->getPowerup()->getType());
        assert(false);
    }
    if (m_controls->m_fire)
        m_time_since_last_shot  = 0.0f;
}   // handleItems

//-----------------------------------------------------------------------------
void BattleAI::handleItemCollection(Vec3* aim_point, int* target_node)
{
    float distance = 99999.9f;
    const std::vector< std::pair<Item*, int> >& item_list =
        BattleGraph::get()->getItemList();
    const unsigned int items_count = item_list.size();
    unsigned int closest_item_num = 0;

    for (unsigned int i = 0; i < items_count; ++i)
    {
        const Item* item = item_list[i].first;

        if (item->wasCollected()) continue;

        if ((item->getType() == Item::ITEM_NITRO_BIG ||
             item->getType() == Item::ITEM_NITRO_SMALL) &&
            (m_kart->getEnergy() >
             m_kart->getKartProperties()->getNitroSmallContainer()))
                continue; // Ignore nitro when already has some

        Vec3 d = item->getXYZ() - m_kart->getXYZ();
        if (d.length_2d() <= distance               &&
           (item->getType() == Item::ITEM_BONUS_BOX ||
            item->getType() == Item::ITEM_NITRO_BIG ||
            item->getType() == Item::ITEM_NITRO_SMALL))
        {
            closest_item_num = i;
            distance = d.length_2d();
        }
    }

    const Item *item_selected = item_list[closest_item_num].first;
    if (item_selected->getType() == Item::ITEM_BONUS_BOX ||
        item_selected->getType() == Item::ITEM_NITRO_BIG ||
        item_selected->getType() == Item::ITEM_NITRO_SMALL)
    {
        *aim_point = item_selected->getXYZ();
        *target_node = item_list[closest_item_num].second;
    }
    else
    {
        // Handle when all targets are swapped, which make AIs follow karts
        *aim_point = m_closest_kart_point;
        *target_node = m_closest_kart_node;
    }
}   // handleItemCollection
