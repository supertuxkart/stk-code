//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2015  Eduardo Hernandez Munoz
//  Copyright (C) 2009-2015  Joerg Henrichs
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

#include "karts/controller/ai_base_controller.hpp"

#include "config/user_config.hpp"
#include "graphics/camera.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/kart_properties.hpp"
#include "karts/controller/ai_properties.hpp"
#include "modes/world.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"

#include <assert.h>

bool AIBaseController::m_ai_debug = false;
int  AIBaseController::m_test_ai  = 0;

AIBaseController::AIBaseController(AbstractKart *kart)
                : Controller(kart)
{
    m_kart          = kart;
    m_kart_length   = m_kart->getKartLength();
    m_kart_width    = m_kart->getKartWidth();
    m_ai_properties = m_kart->getKartProperties()
                            ->getAIPropertiesForDifficulty();
}   // AIBaseController

//-----------------------------------------------------------------------------

void AIBaseController::reset()
{
    m_stuck = false;
    m_collision_times.clear();
}   // reset

//-----------------------------------------------------------------------------

void AIBaseController::update(float dt)
{
    m_stuck = false;
}

//-----------------------------------------------------------------------------
/** In debug mode when the user specified --ai-debug on the command line set
 *  the name of the controller as on-screen text, so that the different AI
 *  controllers can be distinguished.
 *  \param name Name of the controller.
*/
void AIBaseController::setControllerName(const std::string &name)
{
#ifdef DEBUG
    if(m_ai_debug && Camera::getActiveCamera()->getType()==Camera::CM_TYPE_NORMAL)
        m_kart->setOnScreenText(core::stringw(name.c_str()).c_str());
#endif
    Controller::setControllerName(name);
}   // setControllerName

//-----------------------------------------------------------------------------
/** Computes the steering angle to reach a certain point. The function will
 *  request steering by setting the steering angle to maximum steer angle
 *  times skidding factor.
 *  \param point Point to steer towards.
 *  \param skidding_factor Increase factor for steering when skidding.
 *  \return Steer angle to use to reach this point.
 */
float AIBaseController::steerToPoint(const Vec3 &point)
{

    // First translate and rotate the point the AI is aiming
    // at into the kart's local coordinate system.
    Vec3 lc = m_kart->getTrans().inverse()(point);

    // The point the kart is aiming at can be reached 'incorrectly' if the
    // point is below the y=x line: Instead of aiming at that point directly
    // the point will be reached on its way 'back' after a more than 90
    // degree turn in the circle, i.e.:
    // |                 So the point p (belolw the y=x line) can not be
    // |  ---\           reached on any circle directly, so it is reached
    // | /    \          on the indicated way. Since this is not the way
    // |/      p         we expect a kart to drive (it will result in the
    // +--------------   kart doing slaloms, not driving straight), the
    // kart will trigger skidding to allow for sharper turns, and hopefully
    // the situation will change so that the point p can then be reached
    // with a normal turn (it usually works out this way quite easily).
    if(fabsf(lc.getX()) > fabsf(lc.getZ()))
    {
        // Explicitely set the steering angle high enough to that the
        // steer function will request skidding. 0.1 is added in case
        // of floating point errors.
        if(lc.getX()>0)
            return  m_kart->getMaxSteerAngle()
                   *m_ai_properties->m_skidding_threshold+0.1f;
        else
            return -m_kart->getMaxSteerAngle()
                   *m_ai_properties->m_skidding_threshold-0.1f;
    }

    // Now compute the nexessary radius for the turn. After getting the
    // kart local coordinates for the point to aim at, the kart is at
    // (0,0) facing straight ahead. The center of the rotation is then
    // on the X axis and can be computed by the fact that the distance
    // to the kart and to the point to aim at must be the same:
    // r*r = (r-x)*(r-x) + y*y
    // where r is the radius (= position on the X axis), and x, y are the
    // local coordinates of the point to aim at. Solving for r
    // results in r = (x*x+y*y)/2x
    float radius = (lc.getX()*lc.getX() + lc.getZ()*lc.getZ())
                 / (2.0f*lc.getX());

    // sin(steern_angle) = wheel_base / radius:
    float sin_steer_angle = m_kart->getKartProperties()->getWheelBase()/radius;

    // If the wheel base is too long (i.e. the minimum radius is too large
    // to actually reach the target), make sure that skidding is used
    if(sin_steer_angle <= -1.0f)
        return -m_kart->getMaxSteerAngle()
               *m_ai_properties->m_skidding_threshold-0.1f;
    if(sin_steer_angle >=  1.0f)
        return  m_kart->getMaxSteerAngle()
               *m_ai_properties->m_skidding_threshold+0.1f;
    float steer_angle     = asin(sin_steer_angle);

    // After doing the exact computation, we now return an 'oversteered'
    // value. This actually helps in making tighter turns, and also in
    // very tight turns on narrow roads (where following the circle might
    // actually take the kart off track) it forces smaller turns.
    // It does not actually hurt to steer too much, since the steering
    // will be adjusted every frame.
    return steer_angle*2.0f;
}   // steerToPoint

//-----------------------------------------------------------------------------
/** Normalises an angle to be between -pi and _ pi.
 *  \param angle Angle to normalise.
 *  \return Normalised angle.
 */
float AIBaseController::normalizeAngle(float angle)
{
    // Add an assert here since we had cases in which an invalid angle
    // was given, resulting in an endless loop (floating point precision,
    // e.g.: 1E17 - 2*M_PI = 1E17
    assert(angle >= -4*M_PI && angle <= 4*M_PI);
    while( angle >  2*M_PI ) angle -= 2*M_PI;
    while( angle < -2*M_PI ) angle += 2*M_PI;

    if( angle > M_PI ) angle -= 2*M_PI;
    else if( angle < -M_PI ) angle += 2*M_PI;

    return angle;
}   // normalizeAngle

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
void AIBaseController::setSteering(float angle, float dt)
{
    float steer_fraction = angle / m_kart->getMaxSteerAngle();
    if(!canSkid(steer_fraction))
        m_controls->setSkidControl(KartControl::SC_NONE);
    else
        m_controls->setSkidControl(steer_fraction > 0 ? KartControl::SC_RIGHT
                                                      : KartControl::SC_LEFT );
    float old_steer      = m_controls->getSteer();

    if     (steer_fraction >  1.0f) steer_fraction =  1.0f;
    else if(steer_fraction < -1.0f) steer_fraction = -1.0f;

    if(m_kart->getBlockedByPlungerTime()>0)
    {
        if     (steer_fraction >  0.5f) steer_fraction =  0.5f;
        else if(steer_fraction < -0.5f) steer_fraction = -0.5f;
    }

    // The AI has its own 'time full steer' value (which is the time
    float max_steer_change = dt/m_ai_properties->m_time_full_steer;
    if(old_steer < steer_fraction)
    {
        m_controls->setSteer(( old_steer+max_steer_change > steer_fraction)
                             ? steer_fraction : old_steer+max_steer_change);
    }
    else
    {
        m_controls->setSteer( (old_steer-max_steer_change < steer_fraction)
                               ? steer_fraction : old_steer-max_steer_change );
    }
}   // setSteering

// ------------------------------------------------------------------------
/** Certain AI levels will not receive a slipstream bonus in order to
 *  be not as hard.
 */
bool AIBaseController::disableSlipstreamBonus() const
{
    return m_ai_properties->disableSlipstreamUsage();
}   // disableSlipstreamBonus

//-----------------------------------------------------------------------------
/** This is called when the kart crashed with the terrain. This subroutine
 *  tries to detect if the AI is stuck by determining if a certain number
 *  of collisions happened in a certain amount of time, and if so rescues
 *  the kart.
 *  \paran m Pointer to the material that was hit (NULL if no specific
 *         material was used for the part of the track that was hit).
 */
void AIBaseController::crashed(const Material *m)
{
    // Defines how many collision in what time will trigger a rescue.
    // Note that typically it takes ~0.5 seconds for the AI to hit
    // the track again if it is stuck (i.e. time for the push back plus
    // time for the AI to accelerate and hit the terrain again).
    const unsigned int NUM_COLLISION = 3;
    const float COLLISION_TIME       = 1.5f;

    float time = World::getWorld()->getTimeSinceStart();
    if(m_collision_times.size()==0)
    {
        m_collision_times.push_back(time);
        return;
    }

    // Filter out multiple collisions report caused by single collision
    // (bullet can report a collision more than once per frame, and
    // resolving it can take a few frames as well, causing more reported
    // collisions to happen). The time of 0.2 seconds was experimentally
    // found, typically it takes 0.5 seconds for a kart to be pushed back
    // from the terrain and accelerate to hit the same terrain again.
    if(time - m_collision_times.back() < 0.2f)
        return;

    // Remove all outdated entries, i.e. entries that are older than the
    // collision time plus 1 second. Older entries must be deleted,
    // otherwise a collision that happened (say) 10 seconds ago could
    // contribute to a stuck condition.
    while(m_collision_times.size()>0 &&
           time - m_collision_times[0] > 1.0f+COLLISION_TIME)
           m_collision_times.erase(m_collision_times.begin());

    m_collision_times.push_back(time);

    // Now detect if there are enough collision records in the
    // specified time interval.
    if(time - m_collision_times.front() > COLLISION_TIME
        && m_collision_times.size()>=NUM_COLLISION)
    {
        // We can't call m_kart->forceRescue here, since crased() is
        // called during physics processing, and forceRescue() removes the
        // chassis from the physics world, which would then cause
        // inconsistencies and potentially a crash during the physics
        // processing. So only set a flag, which is tested during update.
        m_stuck = true;
    }

}   // crashed(Material)

// ----------------------------------------------------------------------------
/** Determine the center point and radius of a circle given two points on
 *  the circle and the tangent at the first point. This is done as follows:
 *  1. Determine the line going through the center point start+end, which is
 *     orthogonal to the vector from start to end. This line goes through the
 *     center of the circle.
 *  2. Determine the line going through the first point and is orthogonal
 *     to the given tangent.
 *  3. The intersection of these two lines is the center of the circle.
 *  \param[in] end Second point on circle.
 *  \param[out] center Center point of the circle (local coordinate).
 *  \param[out] radius Radius of the circle.
 */
void AIBaseController::determineTurnRadius(const Vec3 &end, Vec3 *center,
                                           float *radius) const
{
    // Convert end point to local coordinate, so start will be 0, 0, 0
    Vec3 lc = m_kart->getTrans().inverse()(end);

    // 1) Line through middle of start+end
    Vec3 mid = 0.5f * lc;
    Vec3 direction = lc;

    Vec3 orthogonal(direction.getZ(), 0, -direction.getX());
    Vec3 q1 = mid + orthogonal;
    irr::core::line2df line1(mid.getX(), mid.getZ(),
                             q1.getX(),  q1.getZ()  );

    irr::core::line2df line2(0, 0, 1, 0);
    irr::core::vector2df result;
    if (line1.intersectWith(line2, result, /*checkOnlySegments*/false))
    {
        Vec3 lc_center(result.X, 0, result.Y);
        if (center)
            *center = lc_center;
        *radius = lc_center.length();
    }
    else
    {
        // No intersection. In this case assume that the two points are
        // on a semicircle, in which case the center is at 0.5*(start+end):
        if (center)
            *center = mid;
        *radius = 0.5f*(lc).length();
    }

}   // determineTurnRadius
