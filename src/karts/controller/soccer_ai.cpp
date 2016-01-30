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

#include "karts/controller/soccer_ai.hpp"

#include "items/attachment.hpp"
#include "items/powerup.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/controller/kart_control.hpp"
#include "karts/kart_properties.hpp"
#include "modes/soccer_world.hpp"
#include "tracks/battle_graph.hpp"

#ifdef AI_DEBUG
#include "irrlicht.h"
#include <iostream>
using namespace irr;
using namespace std;
#endif

SoccerAI::SoccerAI(AbstractKart *kart,
                   StateManager::ActivePlayer *player)
         : ArenaAI(kart, player)
{

    reset();

#ifdef AI_DEBUG
    video::SColor col_debug(128, 128, 0, 0);
    m_debug_sphere = irr_driver->addSphere(1.0f, col_debug);
    m_debug_sphere->setVisible(true);
#endif
    m_world = dynamic_cast<SoccerWorld*>(World::getWorld());
    m_track = m_world->getTrack();

    // Don't call our own setControllerName, since this will add a
    // billboard showing 'AIBaseController' to the kart.
    Controller::setControllerName("SoccerAI");

}   // SoccerAI

//-----------------------------------------------------------------------------

SoccerAI::~SoccerAI()
{
#ifdef AI_DEBUG
    irr_driver->removeNode(m_debug_sphere);
#endif
}   //  ~SoccerAI

//-----------------------------------------------------------------------------
/** Resets the AI when a race is restarted.
 */
void SoccerAI::reset()
{
    ArenaAI::reset();
    AIBaseController::reset();

    m_saving_ball = false;
    if (race_manager->getNumPlayers() == 1)
    {
        // Same handle in SoccerWorld::createKart
        if (race_manager->getKartInfo(0).getSoccerTeam() == SOCCER_TEAM_RED)
        {
            m_cur_team = (m_kart->getWorldKartId() % 2 == 0 ?
               SOCCER_TEAM_BLUE : SOCCER_TEAM_RED);
        }
        else
        {
            m_cur_team = (m_kart->getWorldKartId() % 2 == 0 ?
               SOCCER_TEAM_RED : SOCCER_TEAM_BLUE);
        }
    }
    else
    {
        m_cur_team = (m_kart->getWorldKartId() % 2 == 0 ?
            SOCCER_TEAM_BLUE : SOCCER_TEAM_RED);
    }
}   // reset

//-----------------------------------------------------------------------------
void SoccerAI::update(float dt)
{
    m_saving_ball = false;
    if (World::getWorld()->getPhase() == World::GOAL_PHASE)
    {
        m_controls->m_brake = false;
        m_controls->m_accel = 0.0f;
        AIBaseController::update(dt);
        return;
    }

    ArenaAI::update(dt);
}   // update

//-----------------------------------------------------------------------------
void SoccerAI::findClosestKart(bool use_difficulty)
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

        if (m_world->getKartTeam(kart
            ->getWorldKartId()) == m_world->getKartTeam(m_kart
            ->getWorldKartId()))
            continue; // Skip the kart with the same team

        Vec3 d = kart->getXYZ() - m_kart->getXYZ();
        if (d.length_2d() <= distance)
        {
            distance = d.length_2d();
            closest_kart_num = i;
        }
    }

    const AbstractKart* closest_kart = m_world->getKart(closest_kart_num);
    m_closest_kart_node = m_world->getKartNode(closest_kart_num);
    m_closest_kart_point = closest_kart->getXYZ();
    m_closest_kart = m_world->getKart(closest_kart_num);
    checkPosition(m_closest_kart_point, &m_closest_kart_pos_data);

}   // findClosestKart

//-----------------------------------------------------------------------------
void SoccerAI::findTarget()
{
    // Check whether any defense is needed
    if ((m_world->getBallPosition() - NavMesh::get()->getNavPoly(m_world
        ->getGoalNode(m_cur_team)).getCenter()).length_2d() < 50.0f &&
        m_world->getDefender(m_cur_team) == (signed)m_kart->getWorldKartId())
    {
        m_target_node = m_world->getBallNode();
        m_target_point = correctBallPosition(m_world->getBallPosition());
        return;
    }

    // Find a suitable target to drive to, either ball or powerup
    if ((m_world->getBallPosition() - m_kart->getXYZ()).length_2d() > 20.0f &&
        (m_kart->getPowerup()->getType() == PowerupManager::POWERUP_NOTHING &&
         m_kart->getAttachment()->getType() != Attachment::ATTACH_SWATTER))
        collectItemInArena(&m_target_point , &m_target_node);
    else
    {
        m_target_node = m_world->getBallNode();
        m_target_point = correctBallPosition(m_world->getBallPosition());
    }

}   // findTarget

//-----------------------------------------------------------------------------
Vec3 SoccerAI::correctBallPosition(const Vec3& orig_pos)
{
    // Notice: Build with AI_DEBUG and change camera target to an AI kart,
    // to debug or see how AI steer with the ball

    posData ball_pos = {0};
    posData goal_pos = {0};
    Vec3 ball_lc(0, 0, 0);
    checkPosition(orig_pos, &ball_pos, &ball_lc);

    // opposite team goal
    checkPosition(NavMesh::get()->getNavPoly(m_world
        ->getGoalNode(m_cur_team == SOCCER_TEAM_BLUE ?
        SOCCER_TEAM_RED : SOCCER_TEAM_BLUE)).getCenter(), &goal_pos);

    if (goal_pos.behind)
    {
        if (goal_pos.angle > 0.3f && ball_pos.distance < 3.0f &&
            !ball_pos.behind)
        {
            // Only steer with ball if same sides for ball and goal
            if (ball_pos.on_side && goal_pos.on_side)
            {
                ball_lc = ball_lc + Vec3 (1, 0, 1);
                return m_kart->getTrans()(ball_lc);
            }
            else if (!ball_pos.on_side && !goal_pos.on_side)
            {
                ball_lc = ball_lc - Vec3 (1, 0, 0) + Vec3 (0, 0, 1);
                return m_kart->getTrans()(ball_lc);
            }
            else
                m_controls->m_brake = true;
        }
        else
        {
            // This case is facing straight ahead opposite goal
            // (which is straight behind itself), apply more
            // offset for skidding, to save the ball from behind
            // scored.
            // Notice: this assume map maker make soccer field
            // with two goals facing each other straight
            ball_lc = (goal_pos.on_side ? ball_lc - Vec3 (2, 0, 0) +
                       Vec3 (0, 0, 2) : ball_lc + Vec3 (2, 0, 2));

            if (ball_pos.distance < 3.0f &&
               (m_cur_difficulty == RaceManager::DIFFICULTY_HARD ||
                m_cur_difficulty == RaceManager::DIFFICULTY_BEST))
                m_saving_ball = true;
            return m_kart->getTrans()(ball_lc);
        }
    }

    if (ball_pos.distance < 3.0f &&
        !ball_pos.behind && !goal_pos.behind)
    {
        if (goal_pos.angle < 0.5f)
            return orig_pos;
        else
        {
            // Same with above
            if (ball_pos.on_side && goal_pos.on_side)
            {
                ball_lc = ball_lc + Vec3 (1, 0, 1);
                return m_kart->getTrans()(ball_lc);
            }
            else if (!ball_pos.on_side && !goal_pos.on_side)
            {
                ball_lc = ball_lc - Vec3 (1, 0, 0) + Vec3 (0, 0, 1);
                return m_kart->getTrans()(ball_lc);
            }
            else
                m_controls->m_brake = true;
        }
    }
    return orig_pos;
}   // correctBallPosition

// ------------------------------------------------------------------------
int SoccerAI::getCurrentNode() const
{
    return m_world->getKartNode(m_kart->getWorldKartId());
}   // getCurrentNode
// ------------------------------------------------------------------------
bool SoccerAI::isWaiting() const
{
    return m_world->isStartPhase();
}   // isWaiting
