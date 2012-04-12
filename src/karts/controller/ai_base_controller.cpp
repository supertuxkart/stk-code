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

#include "karts/abstract_kart.hpp"
#include "karts/kart_properties.hpp"
#include "karts/skidding_properties.hpp"
#include "modes/linear_world.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"

AIBaseController::AIBaseController(AbstractKart *kart,
                                   StateManager::ActivePlayer *player) 
                : Controller(kart, player)
{
    m_kart        = kart;
    m_kart_length = m_kart->getKartLength();
    m_kart_width  = m_kart->getKartWidth();

    if(race_manager->getMinorMode()!=RaceManager::MINOR_MODE_3_STRIKES)
    {
        m_world       = dynamic_cast<LinearWorld*>(World::getWorld());
        m_track       = m_world->getTrack();
        computePath();
    }
    else
    {
        // Those variables are not defined in a battle mode (m_world is
        // a linear world, since it assumes the existance of drivelines)
        m_world           = NULL;
        m_track           = NULL;
        m_next_node_index.clear();
        m_all_look_aheads.clear();
        m_successor_index.clear();
    }   // if battle mode

}   // AIBaseController

//-----------------------------------------------------------------------------
void AIBaseController::reset()
{
	m_stuck_trigger_rescue = false;
	m_collision_times.clear();
}   // reset 

//-----------------------------------------------------------------------------
/** Triggers a recomputation of the path to use, so that the AI does not
 *  always use the same way.
 */
void  AIBaseController::newLap(int lap)
{
    if(lap>0)
    {
        computePath();
    }
}   // newLap

//-----------------------------------------------------------------------------
/** Computes a path for the AI to follow.
 */
void AIBaseController::computePath()
{
    m_next_node_index.resize(QuadGraph::get()->getNumNodes());
    m_successor_index.resize(QuadGraph::get()->getNumNodes());
    std::vector<unsigned int> next;
    for(unsigned int i=0; i<QuadGraph::get()->getNumNodes(); i++)
    {
        next.clear();
        // Get all successors the AI is allowed to take.
        QuadGraph::get()->getSuccessors(i, next, /*for_ai*/true);
        // In case of short cuts hidden for the AI it can be that a node
        // might not have a successor (since the first and last edge of
        // a hidden shortcut is ignored). Since in the case that the AI
        // ends up on a short cut (e.g. by accident) and doesn't have an
        // allowed way to drive, it should still be able to drive, so add 
        // the non-AI successors of that node in this case.
        if(next.size()==0)
            QuadGraph::get()->getSuccessors(i, next, /*for_ai*/false);
        // For now pick one part on random, which is not adjusted during the 
        // race. Long term statistics might be gathered to determine the
        // best way, potentially depending on race position etc.
        int r = rand();
        int indx = (int)( r / ((float)(RAND_MAX)+1.0f) * next.size() );
        // In case of rounding errors0
        if(indx>=(int)next.size()) indx--;
        m_successor_index[i] = indx;
        assert(indx <(int)next.size() && indx>=0);
        m_next_node_index[i] = next[indx];
    }

    const unsigned int look_ahead=10;
    // Now compute for each node in the graph the list of the next 'look_ahead'
    // graph nodes. This is the list of node that is tested in checkCrashes.
    // If the look_ahead is too big, the AI can skip loops (see 
    // QuadGraph::findRoadSector for details), if it's too short the AI won't
    // find too good a driveline. Note that in general this list should
    // be computed recursively, but since the AI for now is using only 
    // (randomly picked) path this is fine
    m_all_look_aheads.resize(QuadGraph::get()->getNumNodes());
    for(unsigned int i=0; i<QuadGraph::get()->getNumNodes(); i++)
    {
        std::vector<int> l;
        int current = i;
        for(unsigned int j=0; j<look_ahead; j++)
        {
            assert(current < (int)m_next_node_index.size());
            l.push_back(m_next_node_index[current]);
            current = m_next_node_index[current];
        }   // for j<look_ahead
        m_all_look_aheads[i] = l;
    }
}   // computePath

//-----------------------------------------------------------------------------
/** Updates the ai base controller each time step. Note that any calls to
 *  isStuck() must be done before update is called, since update will reset
 *  the isStuck flag!
 *  \param dt Time step size.
 */
void AIBaseController::update(float dt)
{
	m_stuck_trigger_rescue = false;

    if(QuadGraph::get())
    {
        // Update the current node:
        int old_node = m_track_node;
        if(m_track_node!=QuadGraph::UNKNOWN_SECTOR)
        {
            QuadGraph::get()->findRoadSector(m_kart->getXYZ(), &m_track_node, 
                &m_all_look_aheads[m_track_node]);
        }
        // If we can't find a proper place on the track, to a broader search
        // on off-track locations.
        if(m_track_node==QuadGraph::UNKNOWN_SECTOR)
        {
            m_track_node = QuadGraph::get()->findOutOfRoadSector(m_kart->getXYZ());
        }
        // IF the AI is off track (or on a branch of the track it did not
        // select to be on), keep the old position.
        if(m_track_node==QuadGraph::UNKNOWN_SECTOR ||
            m_next_node_index[m_track_node]==-1)
            m_track_node = old_node;
    }
}   // update

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

	float time = m_world->getTime();
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
		m_stuck_trigger_rescue = true;
	}

}   // crashed(Material)

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
    QuadGraph::get()->getSuccessors(index, successors);
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
    float angle = QuadGraph::get()->getAngleToNext(sector, 
                                                   getNextSector(sector));

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
    m_controls->m_skid   = fabsf(steer_fraction)>=m_skidding_threshold;
    if(m_kart->hasViewBlockedByPlunger()) m_controls->m_skid = false;
    // FIXME: Disable skidding for now if the new skidding
    // code is activated, since the AI can not handle this
    // properly.
    if(m_kart->getKartProperties()->getSkiddingProperties()->getSkidVisualTime()>0)
        m_controls->m_skid = false;
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
