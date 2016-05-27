//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2016 SuperTuxKart-Team
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

#include "karts/controller/arena_ai.hpp"

#include "items/attachment.hpp"
#include "items/item_manager.hpp"
#include "items/powerup.hpp"
#include "items/projectile_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/controller/player_controller.hpp"
#include "karts/controller/ai_properties.hpp"
#include "karts/kart_properties.hpp"
#include "tracks/battle_graph.hpp"
#include "utils/log.hpp"

ArenaAI::ArenaAI(AbstractKart *kart)
       : AIBaseController(kart)
{
    m_debug_sphere = NULL;
    m_debug_sphere_next = NULL;
}   // ArenaAI

//-----------------------------------------------------------------------------
/** Resets the AI when a race is restarted.
 */
void ArenaAI::reset()
{
    m_target_node = BattleGraph::UNKNOWN_POLY;
    m_current_forward_node = BattleGraph::UNKNOWN_POLY;
    m_current_forward_point = Vec3(0, 0, 0);
    m_adjusting_side = false;
    m_closest_kart = NULL;
    m_closest_kart_node = BattleGraph::UNKNOWN_POLY;
    m_closest_kart_point = Vec3(0, 0, 0);
    m_closest_kart_pos_data = {0};
    m_cur_kart_pos_data = {0};
    m_is_stuck = false;
    m_is_uturn = false;
    m_avoid_eating_banana = false;
    m_target_point = Vec3(0, 0, 0);
    m_time_since_last_shot = 0.0f;
    m_time_since_driving = 0.0f;
    m_time_since_reversing = 0.0f;
    m_time_since_uturn = 0.0f;
    m_on_node.clear();
    m_aiming_points.clear();

    m_cur_difficulty = race_manager->getDifficulty();
    AIBaseController::reset();
}   // reset

//-----------------------------------------------------------------------------
/** This is the main entry point for the AI.
 *  It is called once per frame for each AI and determines the behaviour of
 *  the AI, e.g. steering, accelerating/braking, firing.
 */
void ArenaAI::update(float dt)
{
    // This is used to enable firing an item backwards.
    m_controls->m_look_back = false;
    m_controls->m_nitro     = false;

    // Don't do anything if there is currently a kart animations shown.
    if (m_kart->getKartAnimation())
    {
        resetAfterStop();
        return;
    }

    if (isWaiting())
    {
        AIBaseController::update(dt);
        return;
    }

    checkIfStuck(dt);
    if (handleArenaUnstuck(dt))
        return;

    findClosestKart(true);
    findTarget();
    handleArenaItems(dt);
    handleArenaBanana();

    if (m_kart->getSpeed() > 15.0f && m_cur_kart_pos_data.angle < 0.2f)
    {
        // Only use nitro when target is straight
        m_controls->m_nitro = true;
    }

    if (m_is_uturn)
    {
        resetAfterStop();
        handleArenaUTurn(dt);
    }
    else
    {
        handleArenaAcceleration(dt);
        handleArenaSteering(dt);
        handleArenaBraking();
    }

    AIBaseController::update(dt);

}   // update

//-----------------------------------------------------------------------------
bool ArenaAI::getAimingPoints()
{
    m_current_forward_point =
        m_kart->getTrans()(Vec3(0, 0, m_kart->getKartLength()));
    m_current_forward_node = BattleGraph::get()->pointToNode
        (m_current_forward_node, m_current_forward_point,
        false/*ignore_vertical*/);

    if (m_current_forward_node == BattleGraph::UNKNOWN_POLY ||
        m_current_forward_node == m_target_node)
        m_current_forward_node = getCurrentNode();

    int first = m_current_forward_node;
    int last = m_target_node;

    if (first == BattleGraph::UNKNOWN_POLY ||
        last == BattleGraph::UNKNOWN_POLY)
        return false;

    if (m_current_forward_node == m_target_node)
    {
        m_aiming_points.push_back(BattleGraph::get()
            ->getPolyOfNode(first).getCenter());
        m_aiming_points.push_back(m_target_point);
        return true;
    }

    m_aiming_points.push_back(BattleGraph::get()
        ->getPolyOfNode(first).getCenter());
    const int next_node = BattleGraph::get()
        ->getNextShortestPathPoly(first, last);
    assert(next_node != BattleGraph::UNKNOWN_POLY);

    m_aiming_points.push_back(BattleGraph::get()
        ->getPolyOfNode(next_node).getCenter());

    return true;
}   // getAimingPoints

//-----------------------------------------------------------------------------
/** This function sets the steering.
 *  \param dt Time step size.
 */
void ArenaAI::handleArenaSteering(const float dt)
{
    const int current_node = getCurrentNode();

    if (current_node == BattleGraph::UNKNOWN_POLY ||
        m_target_node == BattleGraph::UNKNOWN_POLY) return;

    m_aiming_points.clear();
    const bool found_points = getAimingPoints();
    if (ignorePathFinding())
    {
        // Steer directly
        checkPosition(m_target_point, &m_cur_kart_pos_data);
#ifdef AI_DEBUG
        m_debug_sphere->setPosition(m_target_point.toIrrVector());
#endif
        if (m_cur_kart_pos_data.behind)
        {
            m_adjusting_side = m_cur_kart_pos_data.lhs;
            m_is_uturn = true;
        }
        else
        {
            float target_angle = steerToPoint(m_target_point);
            setSteering(target_angle, dt);
        }
        return;
    }
    else if (found_points)
    {
        m_target_point = m_aiming_points[1];
        checkPosition(m_target_point, &m_cur_kart_pos_data);
#ifdef AI_DEBUG
        m_debug_sphere->setVisible(true);
        m_debug_sphere_next->setVisible(true);
        m_debug_sphere->setPosition(m_aiming_points[0].toIrrVector());
        m_debug_sphere_next->setPosition(m_aiming_points[1].toIrrVector());
#endif
        if (m_cur_kart_pos_data.behind)
        {
            m_adjusting_side = m_cur_kart_pos_data.lhs;
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
void ArenaAI::checkIfStuck(const float dt)
{
    if (m_is_stuck) return;

    if (m_kart->getKartAnimation() || isWaiting())
    {
        m_on_node.clear();
        m_time_since_driving = 0.0f;
    }

    m_on_node.insert(getCurrentNode());
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
/** Handles acceleration.
 *  \param dt Time step size.
 */
void ArenaAI::handleArenaAcceleration(const float dt)
{

    if (m_controls->m_brake)
    {
        m_controls->m_accel = 0.0f;
        return;
    }

    const float handicap =
        (m_cur_difficulty == RaceManager::DIFFICULTY_EASY ? 0.7f : 1.0f);
    m_controls->m_accel = stk_config->m_ai_acceleration * handicap;

}   // handleArenaAcceleration

//-----------------------------------------------------------------------------
void ArenaAI::handleArenaUTurn(const float dt)
{
    const float turn_side = (m_adjusting_side ? 1.0f : -1.0f);

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
}   // handleArenaUTurn

//-----------------------------------------------------------------------------
bool ArenaAI::handleArenaUnstuck(const float dt)
{
    if (!m_is_stuck || m_is_uturn) return false;

    resetAfterStop();
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
    return true;

}   // handleArenaUnstuck

//-----------------------------------------------------------------------------
void ArenaAI::handleArenaBanana()
{
    m_avoid_eating_banana = false;

    if (m_is_uturn) return;

    const std::vector< std::pair<const Item*, int> >& item_list =
        BattleGraph::get()->getItemList();
    const unsigned int items_count = item_list.size();
    for (unsigned int i = 0; i < items_count; ++i)
    {
        const Item* item = item_list[i].first;
        if (item->getType() == Item::ITEM_BANANA && !item->wasCollected())
        {
            posData banana_pos = {0};
            Vec3 banana_lc(0, 0, 0);
            checkPosition(item->getXYZ(), &banana_pos, &banana_lc);
            if (banana_pos.angle < 0.2f && banana_pos.distance < 7.5f &&
               !banana_pos.behind)
            {
                // Check whether it's straight ahead towards a banana
                // If so, adjust target point
                banana_lc = (banana_pos.lhs ? banana_lc + Vec3 (2, 0, 0) :
                    banana_lc - Vec3 (2, 0, 0));
                m_target_point = m_kart->getTrans()(banana_lc);
                m_avoid_eating_banana = true;
                // Handle one banana only
                break;
            }
        }
    }

}   // handleArenaBanana

//-----------------------------------------------------------------------------
/** This function handles braking. It calls determineTurnRadius() to find out
 *  the curve radius. Depending on the turn radius, it finds out the maximum
 *  speed. If the current speed is greater than the max speed and a set minimum
 *  speed, brakes are applied.
 */
void ArenaAI::handleArenaBraking()
{
    // A kart will not brake when the speed is already slower than this
    // value. This prevents a kart from going too slow (or even backwards)
    // in tight curves.
    const float MIN_SPEED = 5.0f;

    if (forceBraking() && m_kart->getSpeed() > MIN_SPEED)
    {
        // Brake now
        m_controls->m_brake = true;
        return;
    }

    m_controls->m_brake = false;

    if (getCurrentNode() == BattleGraph::UNKNOWN_POLY ||
        m_target_node    == BattleGraph::UNKNOWN_POLY) return;

    if (m_aiming_points.empty()) return;

    float current_curve_radius = determineTurnRadius(m_kart->getXYZ(),
        m_aiming_points[0], m_aiming_points[1]);

    float max_turn_speed = m_kart->getSpeedForTurnRadius(current_curve_radius);

    if (m_kart->getSpeed() > max_turn_speed &&
        m_kart->getSpeed() > MIN_SPEED &&
        fabsf(m_controls->m_steer) > 0.3f)
    {
        m_controls->m_brake = true;
    }

}   // handleArenaBraking

//-----------------------------------------------------------------------------
/** The turn radius is determined by fitting a parabola to 3 points: current
 *  location of AI, first corner and the second corner. Once the constants are
 *  computed, a formula is used to find the radius of curvature at the vertex
 *  of the parabola.
 */
float ArenaAI::determineTurnRadius(const Vec3& p1, const Vec3& p2,
                                   const Vec3& p3)
{
    // The parabola function is as following: y=ax2+bx+c
    // No need to calculate c as after differentiating c will be zero
    const float eps = 0.01f;
    const float denominator = (p1.x() - p2.x()) * (p1.x() - p3.x()) *
                              (p2.x() - p3.x());

    // Avoid nan, this will happen if three values of coordinates x are too
    // close together, ie a straight line, return a large radius
    // so no braking is needed
    if (fabsf(denominator) < eps) return 40.0f;

	const float a = (p3.x() * (p2.z() - p1.z()) +
	                 p2.x() * (p1.z() - p3.z()) +
	                 p1.x() * (p3.z() - p2.z())) / denominator;

    // Should not happen, otherwise y=c which is a straight line
    if (fabsf(a) < eps) return 40.0f;

	const float b = (p3.x() * p3.x() * (p1.z() - p2.z()) +
	                 p2.x() * p2.x() * (p3.z() - p1.z()) +
	                 p1.x() * p1.x() * (p2.z() - p3.z())) / denominator;

    // Vertex: -b / 2a
    const float vertex_x = -b / (2 * a);

    // Differentiate the function, so y=ax2+bx+c will become y=2ax+b for dy_dx,
    // y=2a for d2y_dx2
    // Use the vertex of the parabola as x
    const float dy_dx = 2 * a * vertex_x + b;
    const float d2y_dx2 = 2 * a;

    // Calculate the radius of curvature at current location of AI
    const float radius = pow(1 + pow(dy_dx, 2), 1.5f) / fabsf(d2y_dx2);
    assert(!std::isnan(radius));

    // Avoid returning too large radius
    return (radius > 40.0f ? 40.0f : radius);
}   // determineTurnRadius

//-----------------------------------------------------------------------------
void ArenaAI::handleArenaItems(const float dt)
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

            // Check if a flyable (cake, ...) is close or a kart nearby
            // has a swatter attachment. If so, use bubblegum
            // as shield
            if ((!m_kart->isShielded() &&
                projectile_manager->projectileIsClose(m_kart,
                                    m_ai_properties->m_shield_incoming_radius)) ||
               (m_closest_kart_pos_data.distance < 15.0f &&
               ((m_closest_kart->getAttachment()->
                getType() == Attachment::ATTACH_SWATTER) ||
               (m_closest_kart->getAttachment()->
                getType() == Attachment::ATTACH_NOLOKS_SWATTER))))
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
                (difficulty || perfect_aim) &&
                !m_closest_kart->isInvulnerable())
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

    // Below powerups won't appear in arena, so skip them
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
        Log::error("ArenaAI",
                "Invalid or unhandled powerup '%d' in default AI.",
                m_kart->getPowerup()->getType());
        assert(false);
    }
    if (m_controls->m_fire)
        m_time_since_last_shot  = 0.0f;
}   // handleArenaItems

//-----------------------------------------------------------------------------
void ArenaAI::collectItemInArena(Vec3* aim_point, int* target_node) const
{
    float distance = 99999.9f;
    const std::vector< std::pair<const Item*, int> >& item_list =
        BattleGraph::get()->getItemList();
    const unsigned int items_count = item_list.size();

    if (item_list.empty())
    {
        // Notice: this should not happen, as it makes no sense
        // for an arean without items, if so how can attack happen?
        Log::fatal ("ArenaAI",
                    "AI can't find any items in the arena, "
                    "maybe there is something wrong with the navmesh, "
                    "make sure it lies closely to the ground.");
        return;
    }

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

        float test_distance = BattleGraph::get()
            ->getDistance(item_list[i].second, getCurrentNode());
        if (test_distance <= distance               &&
           (item->getType() == Item::ITEM_BONUS_BOX ||
            item->getType() == Item::ITEM_NITRO_BIG ||
            item->getType() == Item::ITEM_NITRO_SMALL))
        {
            closest_item_num = i;
            distance = test_distance;
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
}   // collectItemInArena
