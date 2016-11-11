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
#include "karts/controller/ai_properties.hpp"
#include "karts/kart_properties.hpp"
#include "karts/rescue_animation.hpp"
#include "tracks/arena_graph.hpp"
#include "tracks/arena_node.hpp"

#include <algorithm>

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
    m_closest_kart = NULL;
    m_closest_kart_node = Graph::UNKNOWN_SECTOR;
    m_closest_kart_point = Vec3(0, 0, 0);
    m_is_stuck = false;
    m_is_uturn = false;
    m_mini_skid = false;
    m_target_point = Vec3(0, 0, 0);
    m_target_point_lc = Vec3(0, 0, 0);
    m_reverse_point = Vec3(0, 0, 0);
    m_time_since_last_shot = 0.0f;
    m_time_since_driving = 0.0f;
    m_time_since_off_road = 0.0f;
    m_time_since_reversing = 0.0f;
    m_time_since_uturn = 0.0f;
    m_turn_radius = 0.0f;
    m_steering_angle = 0.0f;
    m_on_node.clear();

    m_cur_difficulty = race_manager->getDifficulty();
    AIBaseController::reset();
}   // reset

//-----------------------------------------------------------------------------
/** This is the main entry point for the AI.
 *  It is called once per frame for each AI and determines the behaviour of
 *  the AI, e.g. steering, accelerating/braking, firing.
 *  \param dt Time step size.
 */
void ArenaAI::update(float dt)
{
    // This is used to enable firing an item backwards.
    m_controls->setLookBack(false);
    m_controls->setNitro(false);

    // Let the function below to reset it later
    m_controls->setAccel(0.0f);
    m_controls->setBrake(false);
    m_mini_skid = false;

    // Don't do anything if there is currently a kart animations shown.
    if (m_kart->getKartAnimation())
    {
        resetAfterStop();
        return;
    }

    if (!isKartOnRoad() && m_kart->isOnGround())
    {
        m_time_since_off_road += dt;
    }
    else if (m_time_since_off_road != 0.0f)
    {
        m_time_since_off_road = 0.0f;
    }

    // If the kart needs to be rescued, do it now (and nothing else)
    if (m_time_since_off_road > 5.0f && m_kart->isOnGround())
    {
        m_time_since_off_road = 0.0f;
        new RescueAnimation(m_kart);
        AIBaseController::update(dt);
        return;
    }

    if (isWaiting())
    {
        AIBaseController::update(dt);
        return;
    }

    checkIfStuck(dt);
    if (gettingUnstuck(dt))
        return;

    findTarget();

    // After found target, convert it to local coordinate, used for skidding or
    // u-turn
    if (!m_is_uturn)
    {
        m_target_point_lc = m_kart->getTrans().inverse()(m_target_point);
        doSkiddingTest();
        configSteering();
    }
    else
    {
        m_target_point_lc = m_kart->getTrans().inverse()(m_reverse_point);
    }
    useItems(dt);

    if (m_kart->getSpeed() > 15.0f && !m_is_uturn && m_turn_radius > 30.0f &&
        !ignorePathFinding())
    {
        // Only use nitro when turn angle is big (180 - angle)
        m_controls->setNitro(true);
    }

    if (m_is_uturn)
    {
        resetAfterStop();
        doUTurn(dt);
    }
    else
    {
        configSpeed();
        setSteering(m_steering_angle, dt);
    }

    AIBaseController::update(dt);

}   // update

//-----------------------------------------------------------------------------
/** Update aiming position, use path finding if necessary.
 *  \param[out] target_point Suitable target point.
 *  \return True if found a suitable target point.
 */
bool ArenaAI::updateAimingPosition(Vec3* target_point)
{
#ifdef AI_DEBUG
        m_debug_sphere_next->setVisible(false);
#endif

    m_current_forward_point = m_kart->getTrans()(Vec3(0, 0, m_kart_length));

    m_turn_radius = 0.0f;
    std::vector<int>* test_nodes = NULL;
    if (m_current_forward_node != Graph::UNKNOWN_SECTOR)
    {
        test_nodes =
            m_graph->getNode(m_current_forward_node)->getNearbyNodes();
    }
    m_graph->findRoadSector(m_current_forward_point, &m_current_forward_node,
        test_nodes);

    // Use current node if forward node is unknown, or near the target
    const int forward =
        m_current_forward_node == Graph::UNKNOWN_SECTOR ||
        m_current_forward_node == m_target_node ||
        getCurrentNode() == m_target_node ?
        getCurrentNode() : m_current_forward_node;

    if (forward == Graph::UNKNOWN_SECTOR ||
        m_target_node == Graph::UNKNOWN_SECTOR)
    {
        Log::error("ArenaAI", "Next node is unknown, path finding failed!");
        return false;
    }

    if (forward == m_target_node)
    {
        determineTurnRadius(m_target_point, NULL, &m_turn_radius);
        *target_point = m_target_point;
        return true;
    }

    std::vector<int> path;
    int next_node = m_graph->getNextNode(forward, m_target_node);

    if (next_node == Graph::UNKNOWN_SECTOR)
    {
        Log::error("ArenaAI", "Next node is unknown, did you forget to link"
                   " adjacent face in navmesh?");
        return false;
    }

    path.push_back(next_node);
    while (m_target_node != next_node)
    {
        int previous_node = next_node;
        next_node = m_graph->getNextNode(previous_node, m_target_node);
        if (next_node == Graph::UNKNOWN_SECTOR)
        {
            Log::error("ArenaAI", "Next node is unknown, did you forget to"
                       " link adjacent face in navmesh?");
            return false;
        }
        path.push_back(next_node);
    }

    determinePath(forward, &path);
    *target_point = m_graph->getNode(path.front())->getCenter();

    return true;

}   // updateAimingPosition

//-----------------------------------------------------------------------------
/** This function config the steering (\ref m_steering_angle) of AI.
 */
void ArenaAI::configSteering()
{
    m_steering_angle = 0.0f;
    const int current_node = getCurrentNode();

    if (current_node == Graph::UNKNOWN_SECTOR ||
        m_target_node == Graph::UNKNOWN_SECTOR)
    {
        return;
    }

    if (ignorePathFinding())
    {
        // Steer directly, don't brake
        m_turn_radius = 100.0f;
#ifdef AI_DEBUG
        m_debug_sphere->setPosition(m_target_point.toIrrVector());
#endif
        if (m_target_point_lc.z() < 0)
        {
            m_is_uturn = true;
            m_reverse_point = m_target_point;
        }
        else
        {
            m_steering_angle = steerToPoint(m_target_point);
        }
        return;
    }

    // Otherwise use path finding to get target point
    Vec3 target_point;
    const bool found_position = updateAimingPosition(&target_point);
    if (found_position)
    {
        m_target_point = target_point;
        m_target_point_lc = m_kart->getTrans().inverse()(m_target_point);
#ifdef AI_DEBUG
        m_debug_sphere->setVisible(true);
        m_debug_sphere->setPosition(m_target_point.toIrrVector());
#endif
        if (m_target_point_lc.z() < 0)
        {
            m_is_uturn = true;
            m_reverse_point = m_target_point;
        }
        else
        {
            m_steering_angle = steerToPoint(m_target_point);
        }
    }
}   // configSteering

//-----------------------------------------------------------------------------
/** Determine whether AI is stuck, by checking if it stays on the same node for
 *  a long period of time (see \ref m_on_node), or \ref isStuck() is true.
 *  \param dt Time step size.
 *  \return True if AI is stuck
 */
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
        // AI is stuck, reset now and try to get unstuck at next frame
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
/** Configure a suitable speed depends on current turn radius.
 */
void ArenaAI::configSpeed()
{
    // A kart will not brake when the speed is already slower than this
    // value. This prevents a kart from going too slow (or even backwards)
    // in tight curves.
    const float MIN_SPEED = 5.0f;
    const float handicap = (m_cur_difficulty == RaceManager::DIFFICULTY_EASY 
                            ? 0.7f : 1.0f                                   );

    const float max_turn_speed = m_kart->getSpeedForTurnRadius(m_turn_radius);
    if ((m_kart->getSpeed() > max_turn_speed || forceBraking()) &&
        m_kart->getSpeed() > MIN_SPEED * handicap)
    {
        // Brake if necessary
        m_controls->setBrake(true);
    }
    else
    {
        // Otherwise accelerate
        m_controls->setAccel(stk_config->m_ai_acceleration * handicap);
    }
}   // configSpeed

//-----------------------------------------------------------------------------
/** Make AI reverse so that it faces in front of the last target point.
 */
void ArenaAI::doUTurn(const float dt)
{
    float turn_angle = atan2f(m_target_point_lc.x(),
        fabsf(m_target_point_lc.z()));
    m_controls->setBrake(true);
    setSteering(turn_angle > 0.0f ? -1.0f : 1.0f, dt);
    m_time_since_uturn += dt;

    if ((m_target_point_lc.z() > 0 && fabsf(turn_angle) < 0.2f) ||
        m_time_since_uturn > 1.5f)
    {
        // End U-turn until target point is in front of this AI
        m_is_uturn = false;
        m_time_since_uturn = 0.0f;
        m_time_since_driving = 0.0f;
        m_reverse_point = Vec3(0, 0, 0);
    }
    else
        m_is_uturn = true;
}   // doUTurn

//-----------------------------------------------------------------------------
/** Function to let AI get unstuck.
 *  \param dt Time step size.
 *  \return True if getting stuck is needed to be done.
 */
bool ArenaAI::gettingUnstuck(const float dt)
{
    if (!m_is_stuck || m_is_uturn) return false;

    resetAfterStop();
    setSteering(0.0f, dt);
    m_controls->setBrake(true);

    m_time_since_reversing += dt;

    if (m_time_since_reversing >= 1.0f)
    {
        m_is_stuck = false;
        m_time_since_reversing = 0.0f;
    }
    AIBaseController::update(dt);
    return true;

}   // gettingUnstuck

//-----------------------------------------------------------------------------
/** Determine how AI should use its item, different \ref m_cur_difficulty will
 *  have a corresponding strategy.
 *  \param dt Time step size.
 */
void ArenaAI::useItems(const float dt)
{
    m_controls->setFire(false);
    if (m_kart->getKartAnimation() ||
        m_kart->getPowerup()->getType() == PowerupManager::POWERUP_NOTHING)
        return;

    // Find a closest kart again, this time we ignore difficulty
    findClosestKart(false/*consider_difficulty*/, false/*find_sta*/);
    if (!m_closest_kart) return;

    Vec3 closest_kart_point_lc =
        m_kart->getTrans().inverse()(m_closest_kart_point);

    m_time_since_last_shot += dt;

    float min_bubble_time = 2.0f;
    const bool difficulty = m_cur_difficulty == RaceManager::DIFFICULTY_EASY ||
                            m_cur_difficulty == RaceManager::DIFFICULTY_MEDIUM;

    const bool fire_behind = closest_kart_point_lc.z() < 0 && !difficulty;

    const float abs_angle = atan2f(fabsf(closest_kart_point_lc.x()),
        fabsf(closest_kart_point_lc.z()));
    const bool perfect_aim = abs_angle < 0.2f;

    // Compensate the distance because this distance is straight to straight
    // in graph node, so if kart to kart are not facing like so as, their real
    // distance maybe smaller
    const float dist_to_kart = getKartDistance(m_closest_kart) * 0.8f;

    switch(m_kart->getPowerup()->getType())
    {
    case PowerupManager::POWERUP_BUBBLEGUM:
        {
            Attachment::AttachmentType type = m_kart->getAttachment()->getType();
            // Don't use shield when we have a swatter.
            if (type == Attachment::ATTACH_SWATTER)
                break;

            // Check if a flyable (cake, ...) is close or a kart nearby
            // has a swatter attachment. If so, use bubblegum
            // as shield
            if ( (!m_kart->isShielded() &&
                   projectile_manager->projectileIsClose(m_kart,
                                    m_ai_properties->m_shield_incoming_radius)  ) ||
                 (dist_to_kart < 15.0f &&
                  (m_closest_kart->getAttachment()->
                                       getType() == Attachment::ATTACH_SWATTER)  )    )
            {
                m_controls->setFire(true);
                m_controls->setLookBack(false);
                break;
            }

            // Avoid dropping all bubble gums one after another
            if (m_time_since_last_shot < 3.0f) break;

            // Use bubblegum if the kart around is close,
            // or can't find a close kart for too long time
            if (dist_to_kart < 15.0f || m_time_since_last_shot > 15.0f)
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
            if (m_kart->getShieldTime() > min_bubble_time)
                break;

            // Leave some time between shots
            if (m_time_since_last_shot < 1.0f) break;

            if (dist_to_kart < 25.0f &&
                !m_closest_kart->isInvulnerable())
            {
                m_controls->setFire(true);
                m_controls->setLookBack(fire_behind);
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

            if (dist_to_kart < 6.0f &&
                (difficulty || perfect_aim) &&
                !m_closest_kart->isInvulnerable())
            {
                m_controls->setFire(true);
                m_controls->setLookBack(fire_behind);
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
                 dist_to_kart * dist_to_kart < d2 &&
                 m_closest_kart->getSpeed() < m_kart->getSpeed())
            {
                m_controls->setFire(true);
                m_controls->setLookBack(false);
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
        m_controls->setFire(true);       // (use it no matter what) for now
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
    if (m_controls->getFire())
        m_time_since_last_shot  = 0.0f;
}   // useItems

//-----------------------------------------------------------------------------
/** Try to collect item in arena, if no suitable item is found, like they are
 *  swapped, it will follow closest kart instead.
 *  \param[out] aim_point Location of item.
 *  \param[out] target_node The node which item lied on.
 */
void ArenaAI::tryCollectItem(Vec3* aim_point, int* target_node) const
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
}   // tryCollectItem

//-----------------------------------------------------------------------------
/** Determine if AI should skid: When it's close to target, but not straight
 *  ahead, in front of it, same steering side and with suitable difficulties
 *  which are in expert and supertux only.
 */
void ArenaAI::doSkiddingTest()
{
    const float abs_angle = atan2f(fabsf(m_target_point_lc.x()),
        fabsf(m_target_point_lc.z()));
    if ((m_cur_difficulty == RaceManager::DIFFICULTY_HARD ||
        m_cur_difficulty == RaceManager::DIFFICULTY_BEST) &&
        m_target_point_lc.z() > 0 && abs_angle > 0.15f &&
        m_target_point_lc.length() < 10.0f &&
        ((m_steering_angle < 0 && m_target_point_lc.x() < 0) ||
        (m_steering_angle > 0 && m_target_point_lc.x() > 0)))
    {
        m_mini_skid = true;
    }

}   // doSkiddingTest

//-----------------------------------------------------------------------------
/** Determine if the path to target needs to be changed to avoid bad items, it
 *  will also set the turn radius based on the new path if necessary.
 *  \param forward Forward node of current AI position.
 *  \param[in,out] path Default path to follow, will be changed if needed.
 */
void ArenaAI::determinePath(int forward, std::vector<int>* path)
{
    std::vector<int> bad_item_nodes;
    // First, test if the nodes AI will cross contain bad item
    for (unsigned int i = 0; i < path->size(); i++)
    {
        // Only test few nodes ahead
        if (i == 6) break;
        const int node = (*path)[i];
        Item* selected = ItemManager::get()->getFirstItemInQuad(node);

        if (selected && !selected->wasCollected() &&
            (selected->getType() == Item::ITEM_BANANA ||
            selected->getType() == Item::ITEM_BUBBLEGUM ||
            selected->getType() == Item::ITEM_BUBBLEGUM_NOLOK))
        {
            bad_item_nodes.push_back(node);
        }
    }

    // If so try to avoid
    if (!bad_item_nodes.empty())
    {
        bool failed_avoid = false;
        for (unsigned int i = 0; i < path->size(); i++)
        {
            if (failed_avoid) break;
            if (i == 6) break;
            // Choose any adjacent node that is in front of the AI to prevent
            // hitting bad item
            ArenaNode* cur_node =
                m_graph->getNode(i == 0 ? forward : (*path)[i - 1]);
            float dist = 99999.9f;
            const std::vector<int>& adj_nodes = cur_node->getAdjacentNodes();
            int chosen_node = Graph::UNKNOWN_SECTOR;
            for (const int& adjacent : adj_nodes)
            {
                if (std::find(bad_item_nodes.begin(), bad_item_nodes.end(),
                    adjacent) != bad_item_nodes.end())
                    continue;

                Vec3 lc = m_kart->getTrans().inverse()
                    (m_graph->getNode(adjacent)->getCenter());
                const float dist_to_target =
                    m_graph->getDistance(adjacent, m_target_node);
                if (lc.z() > 0 && dist > dist_to_target)
                {
                    chosen_node = adjacent;
                    dist = dist_to_target;
                }
                if (chosen_node == Graph::UNKNOWN_SECTOR)
                {
                    Log::debug("ArenaAI", "Too many bad items to avoid!");
                    failed_avoid = true;
                    break;
                }
                (*path)[i] = chosen_node;
            }
        }
    }

    // Now find the first turning corner to determine turn radius
    for (unsigned int i = 0; i < path->size() - 1; i++)
    {
        const Vec3& p1 = m_kart->getXYZ();
        const Vec3& p2 = m_graph->getNode((*path)[i])->getCenter();
        const Vec3& p3 = m_graph->getNode((*path)[i + 1])->getCenter();
        float edge1 = (p1 - p2).length();
        float edge2 = (p2 - p3).length();
        float to_target = (p1 - p3).length();

        // Triangle test
        if (fabsf(edge1 + edge2 - to_target) > 0.1f)
        {
            determineTurnRadius(p3, NULL, &m_turn_radius);
#ifdef AI_DEBUG
            m_debug_sphere_next->setVisible(true);
            m_debug_sphere_next->setPosition(p3.toIrrVector());
#endif
            return;
        }
    }

    // Fallback calculation
    determineTurnRadius(m_target_point, NULL, &m_turn_radius);

}   // determinePath
