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

#ifdef BALL_AIM_DEBUG
#include "graphics/camera.hpp"
#endif

SoccerAI::SoccerAI(AbstractKart *kart)
        : ArenaAI(kart)
{

    reset();

#ifdef AI_DEBUG
    video::SColor col_debug(128, 128, 0, 0);
    video::SColor col_debug_next(128, 0, 128, 128);
    m_debug_sphere = irr_driver->addSphere(1.0f, col_debug);
    m_debug_sphere->setVisible(true);
    m_debug_sphere_next = irr_driver->addSphere(1.0f, col_debug_next);
    m_debug_sphere_next->setVisible(true);
#endif

#ifdef BALL_AIM_DEBUG
    video::SColor red(128, 128, 0, 0);
    video::SColor blue(128, 0, 0, 128);
    m_red_sphere = irr_driver->addSphere(1.0f, red);
    m_red_sphere->setVisible(true);
    m_blue_sphere = irr_driver->addSphere(1.0f, blue);
    m_blue_sphere->setVisible(true);
#endif

    m_world = dynamic_cast<SoccerWorld*>(World::getWorld());
    m_track = m_world->getTrack();
    m_cur_team = m_world->getKartTeam(m_kart->getWorldKartId());
    m_opp_team = (m_cur_team == SOCCER_TEAM_BLUE ?
        SOCCER_TEAM_RED : SOCCER_TEAM_BLUE);

    // Don't call our own setControllerName, since this will add a
    // billboard showing 'AIBaseController' to the kart.
    Controller::setControllerName("SoccerAI");

}   // SoccerAI

//-----------------------------------------------------------------------------

SoccerAI::~SoccerAI()
{
#ifdef AI_DEBUG
    irr_driver->removeNode(m_debug_sphere);
    irr_driver->removeNode(m_debug_sphere_next);
#endif

#ifdef BALL_AIM_DEBUG
    irr_driver->removeNode(m_red_sphere);
    irr_driver->removeNode(m_blue_sphere);
#endif

}   //  ~SoccerAI

//-----------------------------------------------------------------------------
/** Resets the AI when a race is restarted.
 */
void SoccerAI::reset()
{
    ArenaAI::reset();
    AIBaseController::reset();

    m_overtake_ball = false;
    m_force_brake = false;
}   // reset

//-----------------------------------------------------------------------------
void SoccerAI::update(float dt)
{
#ifdef BALL_AIM_DEBUG
    Vec3 red = m_world->getBallAimPosition(SOCCER_TEAM_RED);
    Vec3 blue = m_world->getBallAimPosition(SOCCER_TEAM_BLUE);
    m_red_sphere->setPosition(red.toIrrVector());
    m_blue_sphere->setPosition(blue.toIrrVector());
#endif
    m_force_brake = false;

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
    // Check if this AI kart is the one who will chase the ball
    if (m_world->getBallChaser(m_cur_team) == (signed)m_kart->getWorldKartId())
    {
        m_target_point = determineBallAimingPosition();
        m_target_node  = m_world->getBallNode();
        return;
    }

    // Otherwise do the same as in battle mode, attack other karts
    if (m_kart->getPowerup()->getType() == PowerupManager::POWERUP_NOTHING &&
        m_kart->getAttachment()->getType() != Attachment::ATTACH_SWATTER)
    {
        collectItemInArena(&m_target_point , &m_target_node);
    }
    else if (m_world->getAttacker(m_cur_team) == (signed)m_kart
        ->getWorldKartId())
    {
        // This AI will attack the other team ball chaser
        int id = m_world->getBallChaser(m_cur_team == SOCCER_TEAM_BLUE ?
            SOCCER_TEAM_RED : SOCCER_TEAM_BLUE);
        m_target_point = m_world->getKart(id)->getXYZ();
        m_target_node  = m_world->getKartNode(id);
    }
    else
    {
        m_target_point = m_closest_kart_point;
        m_target_node  = m_closest_kart_node;
    }

}   // findTarget

//-----------------------------------------------------------------------------
Vec3 SoccerAI::determineBallAimingPosition()
{
#ifdef BALL_AIM_DEBUG
    // Choose your favourite team to watch
    if (m_world->getKartTeam(m_kart->getWorldKartId()) == SOCCER_TEAM_BLUE)
    {
        Camera *cam = Camera::getActiveCamera();
        cam->setMode(Camera::CM_NORMAL);
        cam->setKart(m_kart);
    }
#endif

    const Vec3& ball_aim_pos = m_world->getBallAimPosition(m_opp_team);
    const Vec3& orig_pos = m_world->getBallPosition();
    posData ball_pos = {0};
    posData aim_pos = {0};
    Vec3 ball_lc;
    Vec3 aim_lc;
    checkPosition(orig_pos, &ball_pos);
    checkPosition(orig_pos, &aim_pos, &aim_lc);

    // Too far from the ball,
    // use path finding from arena ai to get close
    if (ball_pos.distance > 6.0f) return ball_aim_pos;

    const Vec3 dist_to_aim_point = m_kart->getFrontXYZ() - ball_aim_pos;

    // Prevent lost control when steering with ball
    const bool need_braking = ball_pos.angle > 0.1f &&
        m_kart->getSpeed() > 9.0f && ball_pos.distance < 3.0f;

    if (need_braking)
    {
        m_controls->m_brake = true;
        m_force_brake = true;
    }
    if (dist_to_aim_point.length_2d() < 0.4f)
    {
        //Log::info("","%f",dist_to_aim_point.length_2d());
        return m_world->getBallTrans()(Vec3(0, 0, 1));
    }
    return ball_aim_pos;
}   // determineBallAimingPosition

//-----------------------------------------------------------------------------
int SoccerAI::getCurrentNode() const
{
    return m_world->getKartNode(m_kart->getWorldKartId());
}   // getCurrentNode
//-----------------------------------------------------------------------------
bool SoccerAI::isWaiting() const
{
    return m_world->isStartPhase();
}   // isWaiting
