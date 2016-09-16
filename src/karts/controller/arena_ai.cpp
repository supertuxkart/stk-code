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
#include "tracks/arena_graph.hpp"
#include "tracks/arena_node.hpp"
#include "utils/log.hpp"

ArenaAI::ArenaAI(AbstractKart *kart)
       : AIBaseController(kart)
{
    m_debug_sphere = NULL;
    m_debug_sphere_next = NULL;
    m_graph = ArenaGraph::get();
    assert(m_graph != NULL);
}   // ArenaAI

//-----------------------------------------------------------------------------
/** Resets the AI when a race is restarted.
 */
void ArenaAI::reset()
{
    m_target_node = Graph::UNKNOWN_SECTOR;
    m_current_forward_node = Graph::UNKNOWN_SECTOR;
    m_current_forward_point = Vec3(0, 0, 0);
    m_adjusting_side = false;
    m_closest_kart = NULL;
    m_closest_kart_node = Graph::UNKNOWN_SECTOR;
    m_closest_kart_point = Vec3(0, 0, 0);
    m_closest_kart_pos_data = {0};
    m_cur_kart_pos_data = {0};
    m_is_stuck = false;
    m_is_uturn = false;
    m_avoiding_item = false;
    m_target_point = Vec3(0, 0, 0);
    m_time_since_last_shot = 0.0f;
    m_time_since_driving = 0.0f;
    m_time_since_reversing = 0.0f;
    m_time_since_uturn = 0.0f;
    m_turn_radius = 0.0f;
    m_turn_angle = 0.0f;
    m_on_node.clear();
    m_aiming_points.clear();
    m_aiming_nodes.clear();

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
    m_avoiding_item = false;

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

    if (m_kart->getSpeed() > 15.0f && m_turn_angle < 20)
    {
        // Only use nitro when turn angle is big (180 - angle)
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
bool ArenaAI::updateAimingPosition()
{
    // Notice: we use the point ahead of kart to determine next node,
    // to compensate the time difference between steering
    m_current_forward_point =
        m_kart->getTrans()(Vec3(0, 0, m_kart->getKartLength()));

    std::vector<int>* test_nodes = NULL;
    if (m_current_forward_node != Graph::UNKNOWN_SECTOR)
    {
        test_nodes =
            m_graph->getNode(m_current_forward_node)->getNearbyNodes();
    }
    m_graph->findRoadSector(m_current_forward_point, &m_current_forward_node,
        test_nodes);

    // Use current node if forward node is unknown, or near the target
    const int forward = (m_current_forward_node == Graph::UNKNOWN_SECTOR ||
        m_current_forward_node == m_target_node ||
        getCurrentNode() == m_target_node ? getCurrentNode() :
        m_current_forward_node);

    if (forward == Graph::UNKNOWN_SECTOR ||
        m_target_node == Graph::UNKNOWN_SECTOR)
        return false;

    if (forward == m_target_node)
    {
        m_aiming_points.push_back(m_graph->getNode(forward)->getCenter());
        m_aiming_points.push_back(m_target_point);

        m_aiming_nodes.insert(forward);
        m_aiming_nodes.insert(getCurrentNode());
        return true;
    }

    const int next_node = m_graph->getNextNode(forward, m_target_node);
    if (next_node == Graph::UNKNOWN_SECTOR)
    {
        Log::error("ArenaAI", "Next node is unknown, did you forget to link"
                   "adjacent face in navmesh?");
        return false;
    }

    m_aiming_points.push_back(m_graph->getNode(forward)->getCenter());
    m_aiming_points.push_back(m_graph->getNode(next_node)->getCenter());

    m_aiming_nodes.insert(forward);
    m_aiming_nodes.insert(next_node);
    m_aiming_nodes.insert(getCurrentNode());

    return true;
}   // updateAimingPosition

//-----------------------------------------------------------------------------
/** This function sets the steering.
 *  \param dt Time step size.
 */
void ArenaAI::handleArenaSteering(const float dt)
{
    const int current_node = getCurrentNode();

    if (current_node == Graph::UNKNOWN_SECTOR ||
        m_target_node == Graph::UNKNOWN_SECTOR)
    {
        return;
    }

    m_aiming_points.clear();
    m_aiming_nodes.clear();
    const bool found_position = updateAimingPosition();
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
    else if (found_position)
    {
        updateBadItemLocation();
        assert(m_aiming_points.size() == 2);
        updateTurnRadius(m_kart->getXYZ(), m_aiming_points[0],
            m_aiming_points[1]);
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
        && m_kart->getSpeed() < 0) // Try to emulate reverse like human players
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
void ArenaAI::updateBadItemLocation()
{
    // Check if any nodes AI will cross will meet bad items
    for (const int& node : m_aiming_nodes)
    {
        assert(node != Graph::UNKNOWN_SECTOR);
        Item* selected = ItemManager::get()->getFirstItemInQuad(node);

        if (selected && !selected->wasCollected() &&
            (selected->getType() == Item::ITEM_BANANA ||
            selected->getType() == Item::ITEM_BUBBLEGUM ||
            selected->getType() == Item::ITEM_BUBBLEGUM_NOLOK))
        {
            Vec3 bad_item_lc;
            checkPosition(selected->getXYZ(), NULL, &bad_item_lc,
                true/*use_front_xyz*/);

            // If satisfy the below condition, AI should not be affected by it:
            // bad_item_lc.z() < 0.0f, behind the kart
            if (bad_item_lc.z() < 0.0f)
            {
                continue;
            }

            // If the node AI will pass has a bad item, adjust the aim position
            bad_item_lc = (bad_item_lc.x() < 0 ? bad_item_lc + Vec3(5, 0, 0) :
                           bad_item_lc - Vec3(5, 0, 0));
            m_aiming_points[1] = m_kart->getTrans()(bad_item_lc);
            m_avoiding_item = true;
            // Handle one banana only
            return;
        }
    }

}   // updateBadItemLocation

//-----------------------------------------------------------------------------
/** This function handles braking. It used the turn radius found by
 *  updateTurnRadius(). Depending on the turn radius, it finds out the maximum
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

    if (getCurrentNode() == Graph::UNKNOWN_SECTOR ||
        m_target_node    == Graph::UNKNOWN_SECTOR) return;

    if (m_aiming_points.empty()) return;

    const float max_turn_speed = m_kart->getSpeedForTurnRadius(m_turn_radius);

    if (m_kart->getSpeed() > 1.25f * max_turn_speed &&
        fabsf(m_controls->m_steer) > 0.95f &&
        m_kart->getSpeed() > MIN_SPEED)
    {
        m_controls->m_brake = true;
    }

}   // handleArenaBraking

//-----------------------------------------------------------------------------
void ArenaAI::updateTurnRadius(const Vec3& p1, const Vec3& p2,
                               const Vec3& p3)
{
    // First use cosine formula to find out the angle made by the distance
    // between kart (point one) to point two and point two between point three
    const float a = (p1 - p2).length();
    const float b = (p2 - p3).length();
    const float c = (p1 - p3).length();
    const float angle = 180 - findAngleFrom3Edges(a, b, c);

    // Only calculate radius if not almost straight line
    if (angle > 1 && angle < 179)
    {
        //     angle
        //      ^
        //  a /   \ b
        // 90/\   /\90
        //  \ /   \ /
        //   \     /
        //    \   /
        //     \ /
        //      |
        // Try to estimate the turn radius with the help of a kite-like
        // polygon as shown, find out the lowest angle which is
        // (4 - 2) * 180  - 90 - 90 - angle (180 - angle from above)
        // Then we use this value as the angle of a sector of circle,
        // a + b as the arc length, then the radius can be calculated easily
        m_turn_radius = ((a + b) / (angle / 360)) / M_PI / 2;
    }
    else
    {
        // Return large radius so no braking is needed otherwise
        m_turn_radius = 45.0f;
    }
    m_turn_angle = angle;

}   // updateTurnRadius

//-----------------------------------------------------------------------------
float ArenaAI::findAngleFrom3Edges(float a, float b, float c)
{
    // Cosine forumla : c2 = a2 + b2 - 2ab cos C
    float test_value = ((c * c) - (a * a) - (b * b)) / (-(2 * a * b));
    // Prevent error
    if (test_value < -1)
        test_value = -1;
    else if (test_value > 1)
        test_value = 1;

    return acosf(test_value) * RAD_TO_DEGREE;

}   // findAngleFrom3Edges

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
    float distance = 999999.9f;
    Item* selected = (*target_node == Graph::UNKNOWN_SECTOR ? NULL :
        ItemManager::get()->getFirstItemInQuad(*target_node));

    // Don't look for a new item unless it's collected or swapped
    if (selected && !(selected->wasCollected() ||
        selected->getType() == Item::ITEM_BANANA ||
        selected->getType() == Item::ITEM_BUBBLEGUM ||
        selected->getType() == Item::ITEM_BUBBLEGUM_NOLOK))
    {
        *aim_point = selected->getXYZ();
        return;
    }

    for (unsigned int i = 0; i < m_graph->getNumNodes(); i++)
    {
        Item* cur_item = ItemManager::get()->getFirstItemInQuad(i);
        if (cur_item == NULL) continue;
        if (cur_item->wasCollected() ||
            cur_item->getType() == Item::ITEM_BANANA ||
            cur_item->getType() == Item::ITEM_BUBBLEGUM ||
            cur_item->getType() == Item::ITEM_BUBBLEGUM_NOLOK)
            continue;

        if ((cur_item->getType() == Item::ITEM_NITRO_BIG ||
             cur_item->getType() == Item::ITEM_NITRO_SMALL) &&
            (m_kart->getEnergy() >
             m_kart->getKartProperties()->getNitroSmallContainer()))
                continue; // Ignore nitro when already has some

        const int cur_node = cur_item->getGraphNode();
        assert(cur_node != Graph::UNKNOWN_SECTOR);
        float test_distance = m_graph->getDistance(cur_node, getCurrentNode());
        if (test_distance <= distance)
        {
            selected = cur_item;
            distance = test_distance;
        }
    }

    if (selected != NULL)
    {
        *aim_point = selected->getXYZ();
        *target_node = selected->getGraphNode();
    }
    else
    {
        // Handle when all targets are swapped, which make AIs follow karts
        *aim_point = m_closest_kart_point;
        *target_node = m_closest_kart_node;
    }
}   // collectItemInArena
