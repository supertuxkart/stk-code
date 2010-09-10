// $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2009  Eduardo Hernandez Munoz
//  Copyright (C) 2009, 2010 Joerg Henrichs
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

#include <assert.h>

#include "karts/kart.hpp"
#include "modes/linear_world.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"

AIBaseController::AIBaseController(Kart *kart,
                                   StateManager::ActivePlayer *player) 
                : Controller(kart, player)
{
    m_kart        = kart;
    m_kart_length = m_kart->getKartModel()->getLength();
    m_kart_width  = m_kart->getKartModel()->getWidth();
    m_world       = dynamic_cast<LinearWorld*>(World::getWorld());
    m_track       = m_world->getTrack();
    m_quad_graph  = &m_track->getQuadGraph();
}   // AIBaseController

//-----------------------------------------------------------------------------
/** Returns the next sector of the given sector index. This is used
 *  for branches in the quad graph to select which way the AI kart should
 *  go. This is a very simple implementation that always returns the first
 *  successor.
 *  \param index Index of the graph node for which the successor is searched.
 *  \return Returns the successor of this graph node.
 */
unsigned int AIBaseController::getNextSector(unsigned int index)
{
    std::vector<unsigned int> successors;
    m_quad_graph->getSuccessors(index, successors);
    return successors[0];
}   // getNextSector

//-----------------------------------------------------------------------------
/** This function steers towards a given angle. It also takes a plunger
 ** attached to this kart into account by modifying the actual steer angle
 *  somewhat to simulate driving without seeing.
 */
float AIBaseController::steerToAngle(const unsigned int sector, 
                                     const float add_angle)
{
    float angle = m_quad_graph->getAngleToNext(sector, getNextSector(sector));

    //Desired angle minus current angle equals how many angles to turn
    float steer_angle = angle - m_kart->getHeading();

    if(m_kart->hasViewBlockedByPlunger())
        steer_angle += add_angle*0.2f;
    else
        steer_angle += add_angle;
    steer_angle = normalizeAngle( steer_angle );

    return steer_angle;
}   // steerToAngle

//-----------------------------------------------------------------------------
/** Sets when skidding will be used: when the ratio of steering angle to
 *  maximumn steering angle is larger than the fraction set here,
 *  skidding will be used. This is used to set more aggressive skidding
 *  for higher level AIs.
 *  \param f Fraction with which steering angle / max steering angle is
 *           compared to determine if skidding is used.
 */
void AIBaseController::setSkiddingFraction(float f)
{
    m_skidding_threshold = f;
}   // setSkiddingFactor

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
    btQuaternion q(btVector3(0,1,0), -m_kart->getHeading());
    Vec3 p  = point - m_kart->getXYZ();
    Vec3 lc = quatRotate(q, p);

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
            return  m_kart->getMaxSteerAngle()*m_skidding_threshold+0.1f;
        else
            return -m_kart->getMaxSteerAngle()*m_skidding_threshold-0.1f;
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
        return -m_kart->getMaxSteerAngle()*m_skidding_threshold-0.1f;
    if(sin_steer_angle >=  1.0f) 
        return  m_kart->getMaxSteerAngle()*m_skidding_threshold+0.1f;
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
    m_controls->m_drift   = fabsf(steer_fraction)>=m_skidding_threshold;
    if(m_kart->hasViewBlockedByPlunger()) m_controls->m_drift = false;
    float old_steer      = m_controls->m_steer;

    if     (steer_fraction >  1.0f) steer_fraction =  1.0f;
    else if(steer_fraction < -1.0f) steer_fraction = -1.0f;

    if(m_kart->hasViewBlockedByPlunger())
    {
        if     (steer_fraction >  0.5f) steer_fraction =  0.5f;
        else if(steer_fraction < -0.5f) steer_fraction = -0.5f;
    }
    
    // The AI has its own 'time full steer' value (which is the time
    float max_steer_change = dt/m_kart->getKartProperties()->getTimeFullSteerAI();
    if(old_steer < steer_fraction)
    {
        m_controls->m_steer = (old_steer+max_steer_change > steer_fraction) 
                           ? steer_fraction : old_steer+max_steer_change;
    }
    else
    {
        m_controls->m_steer = (old_steer-max_steer_change < steer_fraction) 
                           ? steer_fraction : old_steer-max_steer_change;
    }
}   // setSteering
