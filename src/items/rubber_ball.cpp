//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011 Joerg Henrichs
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

#include "items/rubber_ball.hpp"

#include "items/projectile_manager.hpp"
#include "karts/kart.hpp"
#include "modes/linear_world.hpp"
#include "tracks/track.hpp"

float RubberBall::m_st_interval;
float RubberBall::m_st_min_interpolation_distance;
float RubberBall::m_st_squash_duration;
float RubberBall::m_st_squash_slowdown;
float RubberBall::m_st_target_distance;

RubberBall::RubberBall(Kart *kart) 
          : Flyable(kart, PowerupManager::POWERUP_RUBBERBALL, 0.0f /* mass */),
            TrackSector()
{
    // The rubber ball often misses the terrain on steep uphill slopes, e.g.
    // the ramp in sand track. Add an Y offset so that the terrain raycast
    // will always be done from a position high enough to avoid this.
    setPositionOffset(Vec3(0, 1.0f, 0));
    float forw_offset = 0.5f*kart->getKartLength() + m_extend.getZ()*0.5f+5.0f;
    
    createPhysics(forw_offset, btVector3(0.0f, 0.0f, m_speed*2),
                  new btSphereShape(0.5f*m_extend.getY()), 
                  -70.0f /*gravity*/, 
                  true /*rotates*/);

    // Do not adjust the up velocity 
    setAdjustUpVelocity(false);
    m_max_lifespan = 9999;
    m_target       = NULL;
    // Just init the previoux coordinates with some value that's not getXYZ()
    m_previous_xyz = m_owner->getXYZ();

    computeTarget();

    // initialises the current graph node
    TrackSector::update(getXYZ());
    initializeControlPoints(m_owner->getXYZ());

    // At the start the ball aims at quads till it gets close enough to the
    // target:
    m_aiming_at_target     = false;
    m_timer                = 0.0f;
    m_interval             = m_st_interval;
    m_current_max_height   = m_max_height;
}   // RubberBall

// -----------------------------------------------------------------------------
/** Sets up the control points for the interpolation. The parameter contains
 *  the coordinates of the first control points (i.e. a control point that 
 *  influences the direction the ball is flying only, not the actual 
 *  coordinates - see details about Catmull-Rom splines). This function will 
 *  then set the 2nd control point to be the current coordinates of the ball,
 *  and find two more appropriate control points for a smooth movement.
 *  \param xyz Coordinates of the first control points.
 */
void RubberBall::initializeControlPoints(const Vec3 &xyz)
{
    m_control_points[0]     = xyz;
    m_control_points[1]     = getXYZ();
    m_last_aimed_graph_node = getSuccessorToHitTarget(getCurrentGraphNode());
    // This call defined m_control_points[3], but also sets a new 
    // m_last_aimed_graph_node, which is further away from the current point,
    // which avoids the problem that the ball might go too quickly to the 
    // left or right when firing the ball off track.
    getNextControlPoint();
    m_control_points[2]     = 
        QuadGraph::get()->getQuadOfNode(m_last_aimed_graph_node).getCenter();

    // This updates m_last_aimed_graph_node, and sets m_control_points[3]
    getNextControlPoint();
    m_length_cp_1_2 = (m_control_points[2]-m_control_points[1]).length();
    m_t             = 0;
    m_t_increase    = m_speed/m_length_cp_1_2;
}   // initialiseControlPoints

// -----------------------------------------------------------------------------
/** Determines the first kart. If a target has already been identified in an
 *  earlier call, it is tested first, avoiding a loop over all karts.
 */
void RubberBall::computeTarget()
{
    LinearWorld *world = dynamic_cast<LinearWorld*>(World::getWorld());
    // FIXME: what does the rubber ball do in case of battle mode??
    if(!world) return;

    for(unsigned int p = race_manager->getFinishedKarts()+1; 
                     p < world->getNumKarts()+1; p++)
    {
        m_target = world->getKartAtPosition(p);
        if(!m_target->isEliminated() && !m_target->hasFinishedRace())
        {
            // Don't aim at yourself
            if(m_target == m_owner) m_target = NULL;
            return;
        }
    }
}   // computeTarget

// -----------------------------------------------------------------------------
/** Determines the successor of a graph node. For now always a successor on
 *  the main driveline is returned, but a more sophisticated implementation
 *  might check if the target kart is on a shortcut, and select the path  so
 *  that it will get to the target even in this case.
 *  \param  node_index The node for which the successor is searched.
 *  \param  dist If not NULL a pointer to a float. The distance between
 *          node_index and the selected successor is added to this float.
 *  \return The node index of a successor node.
 */
unsigned int RubberBall::getSuccessorToHitTarget(unsigned int node_index,
                                                 float *dist)
{
    // For now: always pick a successor on the main driveline.
    int succ = 0;
    if(dist)
        *dist += QuadGraph::get()->getNode(node_index)
                .getDistanceToSuccessor(succ);
    return QuadGraph::get()->getNode(node_index).getSuccessor(succ);
}   // getSuccessorToHitTarget

// ----------------------------------------------------------------------------
/** Determines the next control points to aim at. The control points must not
 *  be too close to each other (since otherwise  the interpolation is still
 *  not smooth enough), so keep on picking graph nodes till the distance 
 *  between the currently aimed at graph node and the next one is above a
 *  certain threshold. It uses getSuccessorToHitTarget to determine which
 *  graph node to select.
 */
void RubberBall::getNextControlPoint()
{
    // Accumulate the distance between the current last graph node point
    // and the next one. This is used to approximate the length of the
    // spline between the control points.
    float dist=0;

    float f = QuadGraph::get()->getDistanceFromStart(m_last_aimed_graph_node);

    int next = getSuccessorToHitTarget(m_last_aimed_graph_node, &dist);
    float d = QuadGraph::get()->getDistanceFromStart(next)-f;
    while(d<m_st_min_interpolation_distance && d>0)
    {
        next = getSuccessorToHitTarget(next, &dist);
        d = QuadGraph::get()->getDistanceFromStart(next)-f;
    }

    m_last_aimed_graph_node = next;
    m_length_cp_2_3         = dist;
    m_control_points[3]     = 
        QuadGraph::get()->getQuadOfNode(m_last_aimed_graph_node).getCenter();
}   // getNextControlPoint

// -----------------------------------------------------------------------------
/** Initialises this object with data from the power.xml file (this is a static
 *  function). 
 *  \param node XML Node
 *  \param bowling The bowling ball mesh
 */
void RubberBall::init(const XMLNode &node, scene::IMesh *bowling)
{
    m_st_interval                   = 1.0f;
    m_st_squash_duration            = 3.0f;
    m_st_squash_slowdown            = 0.5f;
    m_st_target_distance            = 50.0f;
    m_st_min_interpolation_distance = 30.0f;
    if(!node.get("interval", &m_st_interval))
        printf("[powerup] Warning: no interval specified for rubber ball.\n");
    if(!node.get("squash-duration", &m_st_squash_duration))
        printf(
        "[powerup] Warning: no squash-duration specified for rubber ball.\n");
    if(!node.get("squash-slowdown", &m_st_squash_slowdown))
        printf(
        "[powerup] Warning: no squash-slowdown specified for rubber ball.\n");
    if(!node.get("target-distance", &m_st_target_distance))
        printf(
        "[powerup] Warning: no target-distance specified for rubber ball.\n");
    if(!node.get("min-interpolation-distance", 
                 &m_st_min_interpolation_distance))
        printf(
               "[powerup] Warning: no min-interpolation-distance specified "
               "for rubber ball.\n");

    Flyable::init(node, bowling, PowerupManager::POWERUP_RUBBERBALL);
}   // init

// -----------------------------------------------------------------------------
/** Updates the rubber ball.
 *  \param dt Time step size.
 */
void RubberBall::update(float dt)
{
    Flyable::update(dt);

    // Update the target in case that the first kart was overtaken (or has
    // finished the race).
    computeTarget();

    if(!m_target)        // Remove this item from the game
    {
        hit(NULL);
        return;
    }

    checkDistanceToTarget();

    // FIXME: do we want to test if we have overtaken the target kart?    
    m_previous_xyz = getXYZ();

    Vec3 next_xyz;
    if(m_aiming_at_target)
    {
        // If the rubber ball is already close to a target, i.e. aiming
        // at it directly, stop interpolating, instead fly straight
        // towards it.
        Vec3 diff = m_target->getXYZ()-getXYZ();
        next_xyz = getXYZ() + (dt*m_speed/diff.length())*diff;
    }
    else
    {
        interpolate(&next_xyz, dt);
    }
    m_timer += dt;
    float height = updateHeight();
    next_xyz.setY(getHoT()+height);

    // Determine new distance along track
    TrackSector::update(next_xyz);

    // Ball squashing:
    // ===============
    // If we start squashing the ball as soon as the height is smaller than
    // then height of the ball, it looks to extreme. So a ratio r of the 
    // height of the ball and the current height of the object is used to 
    // tweak the look a bit.
    float r = 2.0f;
    if(r*height<m_extend.getY())
        m_node->setScale(core::vector3df(1.0f, r*height/m_extend.getY(),
        1.0f));
    else
        m_node->setScale(core::vector3df(1.0f, 1.0f, 1.0f));

    setXYZ(next_xyz);
}   // update

// ----------------------------------------------------------------------------
/** Uses Hermite splines (Catmull-Rom) to interpolate the position of the
 *  ball between the control points. If the next point would be outside of
 *  the spline between control_points[1] and [2], a new control point is 
 *  added.
 *  \param next_xyz Returns the new position.
 *  \param The time step size.
 */
void RubberBall::interpolate(Vec3 *next_xyz, float dt)
{
    // If we have reached or overshot the next control point, move to the
    // the next section of the spline
    m_t += m_t_increase * dt;
    if(m_t > 1.0f)
    {
        // Move the control points and estimated distance forward.
        for(unsigned int i=1; i<4; i++)
            m_control_points[i-1] = m_control_points[i];
        m_length_cp_1_2 = m_length_cp_2_3;

        // This automatically sets m_control_points[3]
        getNextControlPoint();
        m_t_increase = m_speed/m_length_cp_1_2;
        m_t -= 1.0f;
    }

    *next_xyz = 0.5f * ((-m_control_points[0] + 3*m_control_points[1] -3*m_control_points[2] + m_control_points[3])*m_t*m_t*m_t
               + (2*m_control_points[0] -5*m_control_points[1] + 4*m_control_points[2] - m_control_points[3])*m_t*m_t
               + (-m_control_points[0]+m_control_points[2])*m_t
               + 2*m_control_points[1]);
}   // interpolate

// ----------------------------------------------------------------------------
/** Updates the height of the rubber ball, and if necessary also adjusts the 
 *  maximum height of the ball depending on distance from the target. The 
 *  height is decreased when the ball is getting closer to the target so it 
 *  hops faster and faster. This function modifies m_current_max_height.
 *  \return Returns the new height of the ball.
 */
float RubberBall::updateHeight()
{
    // When the ball hits the floor, we adjust maximum height and 
    // interval so that the ball bounces faster when it is getting
    // closer to the target.
    if(m_timer>m_interval)
    {
        m_timer -= m_interval;

        LinearWorld *world = dynamic_cast<LinearWorld*>(World::getWorld());
        float target_distance = 
            world->getDistanceDownTrackForKart(m_target->getWorldKartId());

        float distance = target_distance - getDistanceFromStart();

        if(distance<0)
            distance+=World::getWorld()->getTrack()->getTrackLength();;
        if(distance<m_st_target_distance)
        {
            // Some experimental formulas
            m_current_max_height   = 0.5f*sqrt(distance);
            m_interval = m_current_max_height / 10.0f;
	        // Avoid too small hops and esp. a division by zero
            if(m_interval<0.01f)
                m_interval = 0.01f;      
        }
        else
        {
            // Reset the values in case that the ball was already trying
            // to get closer to the target, and then the target disappears
            // (e.g. is eliminated or finishes the race).
            m_interval           = m_st_interval;
            m_current_max_height = m_max_height;
        }
    }   // if m_timer > m_interval


    // Determine the height of the ball
    // ================================
    // Consider f(x) = s * x*(x-m_intervall), which is a parabolic function
    // with f(0) = 0, f(m_intervall)=0. We then scale this function to
    // fulfill: f(m_intervall/2) = max_height, or:
    // f(m_interval/2) = s*(-m_interval^2)/4 = max_height
    // --> s =  max_height / -m_interval^2/4
    float f = m_current_max_height / (-0.25f*m_interval*m_interval);
    return m_timer * (m_timer-m_interval) * f;
}   // updateHeight

// ----------------------------------------------------------------------------
void RubberBall::checkDistanceToTarget()
{
    // If aiming at target phase, keep on aiming at target.
    // ----------------------------------------------------
    if(m_aiming_at_target) return;

    const LinearWorld *world = dynamic_cast<LinearWorld*>(World::getWorld());
    if(!world) return;   // FIXME battle mode

    float target_distance = 
        world->getDistanceDownTrackForKart(m_target->getWorldKartId());
    float ball_distance = getDistanceFromStart();

    float diff = target_distance - ball_distance;
    if(diff < 0)
    {
        diff += world->getTrack()->getTrackLength();
    }

    if(diff < m_st_target_distance)
    {
        m_aiming_at_target = true;
        return;
    }
    else if(m_aiming_at_target)
    {
        // It appears that we have lost the target. It was within
        // the target distance, and now it isn't. That means either
        // the original target escaped, or perhaps that there is a 
        // new target. In this case we have to reset the control
        // points, since it's likely that the ball is (after some time
        // going directly towards the target) far outside of the
        // old control points.

        // We use the previous XYZ point to define the first control
        // point, which results in a smooth transition from aiming
        // directly at the target back to interpolating again.
        initializeControlPoints(m_previous_xyz);
        m_aiming_at_target = false;
    }
    
    return;
}   // checkDistanceToTarget

// ----------------------------------------------------------------------------
/** Callback from the physics in case that a kart or object is hit. The rubber
 *  ball will only be removed if it hits it target, all other karts it might
 *  hit earlier will only be flattened.
 *  kart The kart hit (NULL if no kart was hit).
 *  object The object that was hit (NULL if none).
 */
void RubberBall::hit(Kart* kart, PhysicalObject* object)
{
    if(kart)
    {
        // If the object is not the main target, only flatten the kart
        if(kart!=m_target)
            kart->setSquash(m_st_squash_duration, m_st_squash_slowdown);
        else
        {
            // Else trigger the full explosion animation
            kart->handleExplosion(kart->getXYZ(), /*direct hit*/true);
            projectile_manager->notifyRemove();
            setHasHit();
        }
        return;
    }
    Flyable::hit(kart, object);
}   // hit
