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
    m_red_sphere->setVisible(false);
    m_blue_sphere = irr_driver->addSphere(1.0f, blue);
    m_blue_sphere->setVisible(false);
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
    m_steer_with_ball = false;

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
    m_steer_with_ball = false;

    if (World::getWorld()->getPhase() == World::GOAL_PHASE)
    {
        resetAfterStop();
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

    // Always reset this flag,
    // in case the ball chaser lost the ball somehow
    m_overtake_ball = false;

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
    const float ball_diameter = m_world->getBallDiameter();
    posData ball_pos = {0};
    posData aim_pos = {0};
    Vec3 ball_lc;
    Vec3 aim_lc;
    checkPosition(orig_pos, &ball_pos, &ball_lc, true/*use_front_xyz*/);
    checkPosition(ball_aim_pos, &aim_pos, &aim_lc, true/*use_front_xyz*/);

    // Too far from the ball,
    // use path finding from arena ai to get close
    // ie no extra braking is needed
    if (aim_pos.distance > 10.0f) return ball_aim_pos;

    if (m_overtake_ball)
    {
        Vec3 overtake_wc;
        const bool can_overtake = determineOvertakePosition(ball_lc, ball_pos,
            &overtake_wc);
        if (!can_overtake)
        {
            m_overtake_ball = false;
            return ball_aim_pos;
        }
        else
            return overtake_wc;
    }
    else
    {
        // Check whether the aim point is non-reachable
        // ie the ball is in front of the kart, which the aim position
        // is behind the ball , if so m_overtake_ball is true
        if (aim_lc.z() > 0 && aim_lc.z() > ball_lc.z())
        {
            const bool can_overtake = determineOvertakePosition(ball_lc,
                ball_pos, NULL);
            if (can_overtake)
            {
                m_overtake_ball = true;
            }
            else
            {
                // Stop a while to wait for overtaking, prevent own goal too
                // Only do that if the ball is moving
                if (!m_world->ballNotMoving())
                    m_force_brake = true;
                return ball_aim_pos;
            }
        }

        // Otherwise use the aim position calculated by soccer world
        // Prevent lost control when steering with ball
        m_force_brake = ball_pos.angle > 0.15f &&
            m_kart->getSpeed() > 9.0f && ball_pos.distance < ball_diameter;

        if (aim_pos.behind && aim_pos.distance < (ball_diameter / 2))
        {
            // Reached aim point, aim forward
            m_steer_with_ball = true;
            return m_world->getBallAimPosition(m_opp_team, true/*reverse*/);
        }
        return ball_aim_pos;
    }

    // Make compiler happy
    return ball_aim_pos;

}   // determineBallAimingPosition

//-----------------------------------------------------------------------------
bool SoccerAI::determineOvertakePosition(const Vec3& ball_lc,
                                         const posData& ball_pos,
                                         Vec3* overtake_wc)
{
    // This done by drawing a circle using the center of ball local coordinates
    // and the distance / 2 from kart to ball center as radius (which allows more
    // offset for overtaking), then find tangent line from kart (0, 0, 0) to the
    // circle. The intercept point will be used as overtake position

    // No overtake if ball is behind
    if (ball_lc.z() < 0.0f) return false;

    // Circle equation: (x-a)2 + (y-b)2 = r2
    const float r2 = (ball_pos.distance / 2) * (ball_pos.distance / 2);

    // No overtake if sqrtf(r2) / 2 < ball radius,
    // which will likely push to ball forward
    if ((sqrtf(r2) / 2) < (m_world->getBallDiameter() / 2))
    {
        return false;
    }

    const float a = ball_lc.x();
    const float b = ball_lc.z();

    // Check first if the kart is lies inside the circle, if so no tangent
    // can be drawn ( so can't overtake), minus 0.1 as epslion
    const float test_radius_2 = ((a * a) + (b * b)) - 0.1f;
    if (test_radius_2 < r2)
    {
        return false;
    }

    // Otherwise calculate the tangent
    // As all are local coordinates, so center is 0,0 which is y = mx for the
    // tangent equation, and the m (slope) can be calculated by puting y = mx
    // into the general form of circle equation x2 + y2 + Dx + Ey + F = 0
    // This means:  x2 + m2x2 + Dx + Emx + F = 0
    //                 (1+m2)x2 + (D+Em)x +F = 0
    // As only one root for it, so discriminant b2 - 4ac = 0
    // So:              (D+Em)2 - 4(1+m2)(F) = 0
    //           D2 + 2DEm +E2m2 - 4F - 4m2F = 0
    //      (E2 - 4F)m2 + (2DE)m + (D2 - 4F) = 0
    // Now solve the above quadratic equation using
    // x = -b (+/-) sqrt(b2 - 4ac) / 2a
    const float d = -2 * a;
    const float e = -2 * b;
    const float f = (d * d / 4) + (e * e / 4) - r2;
    const float discriminant = (2 * 2 * d * d * e * e) -
        (4 * ((e * e) - (4 * f)) * ((d * d) - (4 * f)));

    assert(discriminant > 0.0f);
    const float slope_1 = (-(2 * d * e) + sqrtf(discriminant)) /
        (2 * ((e * e) - (4 * f)));
    const float slope_2 = (-(2 * d * e) - sqrtf(discriminant)) /
        (2 * ((e * e) - (4 * f)));

    assert(!std::isnan(slope_1));
    assert(!std::isnan(slope_2));

    // Calculate two intercept points, as we already put y=mx into circle
    // equation and know that only one root for each slope, so x can be
    // calculated easily with -b / 2a
    // From (1+m2)x2 + (D+Em)x +F = 0:
    const float x1 = -(d + (e * slope_1)) / (2 * (1 + (slope_1 * slope_1)));
    const float x2 = -(d + (e * slope_2)) / (2 * (1 + (slope_2 * slope_2)));

    // Use the closest point to aim
    float x = std::min(fabsf(x1), fabsf(x2));
    float y = 0.0f;
    if (-x == x1)
    {
        // x was negative
        x = -x;
        y = slope_1 * x;
    }
    else if (x == x1)
    {
        y = slope_1 * x;
    }
    else if (-x == x2)
    {
        x = -x;
        y = slope_2 * x;
    }
    else
    {
        y = slope_2 * x;
    }

    if (overtake_wc)
        *overtake_wc = m_kart->getTrans()(Vec3(x, 0, y));
    return true;
}   // determineOvertakePosition

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
