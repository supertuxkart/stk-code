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
#include "tracks/arena_graph.hpp"
#include "tracks/track.hpp"

#ifdef AI_DEBUG
#include "irrlicht.h"
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
    m_track = Track::getCurrentTrack();
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

    m_overtake_ball = false;
    m_force_brake = false;
    m_chasing_ball = false;

    m_front_transform.setOrigin(m_kart->getFrontXYZ());
    m_front_transform.setBasis(m_kart->getTrans().getBasis());

}   // reset

//-----------------------------------------------------------------------------
/** Update \ref m_front_transform for ball aiming functions, also make AI stop
 *  after goal.
 *  \param dt Time step size.
 */
void SoccerAI::update(float dt)
{
#ifdef BALL_AIM_DEBUG
    Vec3 red = m_world->getBallAimPosition(SOCCER_TEAM_RED);
    Vec3 blue = m_world->getBallAimPosition(SOCCER_TEAM_BLUE);
    m_red_sphere->setPosition(red.toIrrVector());
    m_blue_sphere->setPosition(blue.toIrrVector());
#endif
    m_force_brake = false;
    m_chasing_ball = false;
    m_front_transform.setOrigin(m_kart->getFrontXYZ());
    m_front_transform.setBasis(m_kart->getTrans().getBasis());

    if (m_world->getPhase() == World::GOAL_PHASE)
    {
        resetAfterStop();
        m_controls->setBrake(false);
        m_controls->setAccel(0.0f);
        AIBaseController::update(dt);
        return;
    }

    ArenaAI::update(dt);
}   // update

//-----------------------------------------------------------------------------
/** Find the closest kart around this AI, it won't find the kart with same
 *  team, consider_difficulty and find_sta are not used here.
 *  \param consider_difficulty If take current difficulty into account.
 *  \param find_sta If find \ref SpareTireAI only.
 */
void SoccerAI::findClosestKart(bool consider_difficulty, bool find_sta)
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

    m_closest_kart = m_world->getKart(closest_kart_num);
    m_closest_kart_node = m_world->getSectorForKart(m_closest_kart);
    m_closest_kart_point = m_closest_kart->getXYZ();

}   // findClosestKart

//-----------------------------------------------------------------------------
/** Find a suitable target to follow, it will first call
 *  \ref SoccerWorld::getBallChaser to check if this AI should go chasing the
 *  ball and try to score, otherwise it will call \ref tryCollectItem if
 *  needed. After that it will call \ref SoccerWorld::getAttacker to see if
 *  this AI should attack the kart in opposite team which is chasing the ball,
 *  if not go for the closest kart found by \ref findClosestKart.
 */
void SoccerAI::findTarget()
{
    findClosestKart(true/*consider_difficulty*/, false/*find_sta*/);
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
        tryCollectItem(&m_target_point , &m_target_node);
    }
    else if (m_world->getAttacker(m_cur_team) == (signed)m_kart
        ->getWorldKartId())
    {
        // This AI will attack the other team ball chaser
        int id = m_world->getBallChaser(m_opp_team);
        const AbstractKart* kart = m_world->getKart(id);
        m_target_point = kart->getXYZ();
        m_target_node  = m_world->getSectorForKart(kart);
    }
    else
    {
        m_target_point = m_closest_kart_point;
        m_target_node  = m_closest_kart_node;
    }

}   // findTarget

//-----------------------------------------------------------------------------
/** Determine the point for aiming when try to steer or overtake the ball.
 *  AI will overtake the ball if the aiming position calculated by world is
 *  non-reachable.
 *  \return The coordinates to aim at.
 */
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

    Vec3 ball_lc = m_front_transform.inverse()(orig_pos);
    Vec3 aim_lc = m_front_transform.inverse()(ball_aim_pos);

    // Too far from the ball,
    // use path finding from arena ai to get close
    // ie no extra braking is needed
    if (aim_lc.length_2d() > 10.0f) return ball_aim_pos;

    if (m_overtake_ball)
    {
        Vec3 overtake_lc;
        const bool can_overtake = determineOvertakePosition(ball_lc, aim_lc,
            &overtake_lc);
        if (!can_overtake)
        {
            m_overtake_ball = false;
            return ball_aim_pos;
        }
        else
            return m_front_transform(overtake_lc);
    }
    else
    {
        // Check whether the aim point is non-reachable
        // ie the ball is in front of the kart, which the aim position
        // is behind the ball , if so m_overtake_ball is true
        if (aim_lc.z() > 0 && aim_lc.z() > ball_lc.z())
        {
            if (isOvertakable(ball_lc))
            {
                m_overtake_ball = true;
                return ball_aim_pos;
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

        m_chasing_ball = true;
        // Check if reached aim point, which is behind aiming position and
        // in front of the ball, if so use another aiming method
        if (aim_lc.z() < 0 && ball_lc.z() > 0)
        {
            // Return the behind version of aim position, allow pushing to
            // ball towards the it
            return m_world->getBallAimPosition(m_opp_team, true/*reverse*/);
        }
    }

    // Otherwise keep steering until reach aim position
    return ball_aim_pos;

}   // determineBallAimingPosition

//-----------------------------------------------------------------------------
/** Used in \ref determineBallAimingPosition to test if AI can overtake the
 *  ball by testing distance.
 *  \param ball_lc Local coordinates of the ball.
 *  \return False if the kart is too close to the ball which can't overtake
 */
bool SoccerAI::isOvertakable(const Vec3& ball_lc)
{
    // No overtake if ball is behind
    if (ball_lc.z() < 0.0f) return false;

    // Circle equation: (x-a)2 + (y-b)2 = r2
    const float r2 = (ball_lc.length_2d() / 2) * (ball_lc.length_2d() / 2);
    const float a = ball_lc.x();
    const float b = ball_lc.z();

    // Check first if the kart is lies inside the circle, if so no tangent
    // can be drawn ( so can't overtake), minus 0.1 as epslion
    const float test_radius_2 = ((a * a) + (b * b)) - 0.1f;
    if (test_radius_2 < r2)
    {
        return false;
    }
    return true;

}   // isOvertakable

//-----------------------------------------------------------------------------
/** Used in \ref determineBallAimingPosition to pick a correct point to
 *  overtake the ball
 *  \param ball_lc Local coordinates of the ball.
 *  \param aim_lc Local coordinates of the aiming position.
 *  \param[out] overtake_lc Local coordinates of the overtaking position.
 *  \return True if overtaking is possible.
 */
bool SoccerAI::determineOvertakePosition(const Vec3& ball_lc,
                                         const Vec3& aim_lc,
                                         Vec3* overtake_lc)
{
    // This done by drawing a circle using the center of ball local coordinates
    // and the distance / 2 from kart to ball center as radius (which allows
    // more offset for overtaking), then find tangent line from kart (0, 0, 0)
    // to the circle. The intercept point will be used as overtake position

    // Check if overtakable at current location
    if (!isOvertakable(ball_lc)) return false;

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

    // Circle equation: (x-a)2 + (y-b)2 = r2
    const float r = ball_lc.length_2d() / 2;
    const float r2 = r * r;
    const float a = ball_lc.x();
    const float b = ball_lc.z();

    const float d = -2 * a;
    const float e = -2 * b;
    const float f = (d * d / 4) + (e * e / 4) - r2;
    const float discriminant = (2 * 2 * d * d * e * e) -
        (4 * ((e * e) - (4 * f)) * ((d * d) - (4 * f)));

    assert(discriminant > 0.0f);
    float t_slope_1 = (-(2 * d * e) + sqrtf(discriminant)) /
        (2 * ((e * e) - (4 * f)));
    float t_slope_2 = (-(2 * d * e) - sqrtf(discriminant)) /
        (2 * ((e * e) - (4 * f)));

    assert(!std::isnan(t_slope_1));
    assert(!std::isnan(t_slope_2));

    // Make the slopes in correct order, allow easier rotate later
    float slope_1 = 0.0f;
    float slope_2 = 0.0f;
    if ((t_slope_1 > 0 && t_slope_2 > 0) || (t_slope_1 < 0 && t_slope_2 < 0))
    {
        if (t_slope_1 > t_slope_2)
        {
            slope_1 = t_slope_1;
            slope_2 = t_slope_2;
        }
        else
        {
            slope_1 = t_slope_2;
            slope_2 = t_slope_1;
        }
    }
    else
    {
        if (t_slope_1 > t_slope_2)
        {
            slope_1 = t_slope_2;
            slope_2 = t_slope_1;
        }
        else
        {
            slope_1 = t_slope_1;
            slope_2 = t_slope_2;
        }
    }

    // Calculate two intercept points, as we already put y=mx into circle
    // equation and know that only one root for each slope, so x can be
    // calculated easily with -b / 2a
    // From (1+m2)x2 + (D+Em)x +F = 0:
    const float x1 = -(d + (e * slope_1)) / (2 * (1 + (slope_1 * slope_1)));
    const float x2 = -(d + (e * slope_2)) / (2 * (1 + (slope_2 * slope_2)));
    const float y1 = slope_1 * x1;
    const float y2 = slope_2 * x2;

    const Vec3 point1(x1, 0, y1);
    const Vec3 point2(x2, 0, y2);

    const float d1 = (point1 - aim_lc).length_2d();
    const float d2 = (point2 - aim_lc).length_2d();

    // Use the tangent closest to the ball aiming position to aim
    const bool use_tangent_one = d1 < d2;

    // Adjust x and y if r < ball diameter,
    // which will likely push to ball forward
    // Notice: we cannot increase the radius before, as the kart location
    // will likely lie inside the enlarged circle
    if (r < m_world->getBallDiameter())
    {
        // Constuctor a equation using y = (rotateSlope(old_m)) x which is
        // a less steep or steeper line, and find out the new adjusted position
        // using the distance to the original point * 2 at new line

        // Determine if the circle is drawn around the side of kart
        // ie point1 or point2 z() < 0, if so reverse the below logic
        const float m = ((point1.z() < 0 || point2.z() < 0) ?
            (use_tangent_one ? rotateSlope(slope_1, false/*rotate_up*/) :
            rotateSlope(slope_2, true/*rotate_up*/)) :
            (use_tangent_one ? rotateSlope(slope_1, true/*rotate_up*/) :
            rotateSlope(slope_2, false/*rotate_up*/)));

        // Calculate new distance from kart to new adjusted position
        const float dist = (use_tangent_one ? point1 : point2).length_2d() * 2;
        const float dist2 = dist * dist;

        // x2 + y2 = dist2
        // so y = m * sqrtf (dist2 - y2)
        // y = sqrtf(m2 * dist2 / (1 + m2))
        const float y = sqrtf((m * m * dist2) / (1 + (m * m)));
        const float x = y / m;
        *overtake_lc = Vec3(x, 0, y);
    }
    else
    {
        // Use the calculated position depends on distance to aim position
        if (use_tangent_one)
            *overtake_lc = point1;
        else
            *overtake_lc = point2;
    }

    return true;
}   // determineOvertakePosition

//-----------------------------------------------------------------------------
/** Used in \ref determineOvertakePosition to adjust the overtake position
 *  which is calculated by slope of line if it's too close.
 *  \param old_slope Old slope calculated.
 *  \param rotate_up If adjust the slope upwards.
 *  \return A newly calculated slope.
 */
float SoccerAI::rotateSlope(float old_slope, bool rotate_up)
{
    const float theta = atan(old_slope) + (old_slope < 0 ? M_PI : 0);
    float new_theta = theta + (rotate_up ? M_PI / 6 : -M_PI /6);
    if (new_theta > ((M_PI / 2) - 0.02f) && new_theta < ((M_PI / 2) + 0.02f))
    {
        // Avoid almost tan 90
        new_theta = (M_PI / 2) - 0.02f;
    }
    // Check if over-rotated
    if (new_theta > M_PI)
        new_theta = M_PI - 0.1f;
    else if (new_theta < 0)
        new_theta = 0.1f;

    return tan(new_theta);
}   // rotateSlope

//-----------------------------------------------------------------------------
int SoccerAI::getCurrentNode() const
{
    return m_world->getSectorForKart(m_kart);
}   // getCurrentNode

//-----------------------------------------------------------------------------
bool SoccerAI::isWaiting() const
{
    return m_world->isStartPhase();
}   // isWaiting

//-----------------------------------------------------------------------------
float SoccerAI::getKartDistance(const AbstractKart* kart) const
{
    return m_graph->getDistance(getCurrentNode(),
        m_world->getSectorForKart(kart));
}   // getKartDistance

//-----------------------------------------------------------------------------
bool SoccerAI::isKartOnRoad() const
{
    return m_world->isOnRoad(m_kart->getWorldKartId());
}   // isKartOnRoad
