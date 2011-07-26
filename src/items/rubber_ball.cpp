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
float RubberBall::m_st_squash_duration;
float RubberBall::m_st_squash_slowdown;

RubberBall::RubberBall(Kart *kart) : Flyable(kart, PowerupManager::POWERUP_RUBBERBALL, 
                                             0.0f /* mass */)
{
    float forw_offset = 0.5f*kart->getKartLength() + m_extend.getZ()*0.5f+5.0f;
    
    createPhysics(forw_offset, btVector3(0.0f, 0.0f, m_speed*2),
                  new btSphereShape(0.5f*m_extend.getY()), 
                  -70.0f /*gravity*/, 
                  true /*rotates*/);

    // Do not adjust the up velocity 
    setAdjustUpVelocity(false);

    // should not live forever, auto-destruct after 30 seconds
    m_max_lifespan = 30;

    m_target = NULL;
    computeTarget();

    const LinearWorld *world = dynamic_cast<LinearWorld*>(World::getWorld());
    m_quad_graph         = &(world->getTrack()->getQuadGraph());
    m_current_graph_node = world->getSectorForKart(kart->getWorldKartId());

    // Determine distance along track
    Vec3 ball_distance_vec;
    m_quad_graph->spatialToTrack(&ball_distance_vec, getXYZ(), 
                                 m_current_graph_node);
    m_distance_along_track = ball_distance_vec[2];

    std::vector<unsigned int> succ;
    m_quad_graph->getSuccessors(m_current_graph_node, succ, /* ai */ true);

    // We have to start aiming at the next sector (since it might be possible
    // that the kart is ahead of the center of the current sector).
    m_aimed_graph_node     = succ[0];

    // At the start the ball aims at quads till it gets close enough to the
    // target:
    m_aiming_at_target     = false;
    m_wrapped_around       = false;
    m_timer                = 0.0f;
    m_interval             = m_st_interval;
    m_height               = m_max_height;
}   // RubberBall

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
/** Initialises this object with data from the power.xml file (this is a static
 *  function). 
 *  \param node XML Node
 *  \param bowling The bowling ball mesh
 */
void RubberBall::init(const XMLNode &node, scene::IMesh *bowling)
{
    m_st_interval        = 1.0f;
    m_st_squash_duration = 3.0f;
    m_st_squash_slowdown = 0.5f;
    if(!node.get("interval", &m_st_interval))
        printf("[powerup] Warning: no interval specific for rubber ball.\n");
    if(!node.get("squash-duration", &m_st_squash_duration))
        printf(
        "[powerup] Warning: no squash-duration specific for rubber ball.\n");
    if(!node.get("squash-slowdown", &m_st_squash_slowdown))
        printf(
        "[powerup] Warning: no squash-slowdown specific for rubber ball.\n");

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

    if(!m_target)
    {
        // Remove this item from the game
        hit(NULL);
        return;
    }

    Vec3 aim_xyz;
    determineTargetCoordinates(dt, &aim_xyz);

    Vec3 delta = aim_xyz - getXYZ();

    // Determine the next point for the ball.
    // FIXME: use interpolation here for smooth curves
    Vec3 next_xyz = getXYZ() + delta * (m_speed * dt / delta.length());

    GraphNode &gn = m_quad_graph->getNode(m_current_graph_node);
    int indx = gn.getSuccessor(0);
    GraphNode &gn_next = m_quad_graph->getNode(indx);

    // Determine new distance along track
    Vec3 ball_distance_vec;
    m_quad_graph->findRoadSector(next_xyz, &m_current_graph_node);
    if(m_current_graph_node == QuadGraph::UNKNOWN_SECTOR)
    {
        m_current_graph_node = 
            m_quad_graph->findOutOfRoadSector(next_xyz,
                                              QuadGraph::UNKNOWN_SECTOR );
    }
    m_quad_graph->spatialToTrack(&ball_distance_vec, getXYZ(), 
                                 m_current_graph_node);

    // Detect wrap around, i.e. crossing the start line
    float old_distance     = m_distance_along_track;
    m_distance_along_track = ball_distance_vec[2];

    float track_length = World::getWorld()->getTrack()->getTrackLength();

    // Detect if the ball crossed the start line
    m_wrapped_around = old_distance > 0.9f * track_length && 
                       m_distance_along_track < 10.0f;


    // FIXME: do we want to test if we have overtaken the target kart?    


    // When the ball hits the floor, we adjust maximum height and 
    // interval so that the ball bounces faster when it is getting
    // closer to the target.
    m_timer += dt;
    if(m_timer>m_interval)
    {
        m_timer -= m_interval;

        LinearWorld *world = dynamic_cast<LinearWorld*>(World::getWorld());
        float target_distance = 
            world->getDistanceDownTrackForKart(m_target->getWorldKartId());

        float distance = target_distance - m_distance_along_track;
        if(distance<0)
            distance+=track_length;
        if(distance<50)
        {
            // Some experimental formulas
            m_height   = 0.5f*sqrt(distance);
            m_interval = m_height / 10.0f;
	    // Avoid too small hops and esp. a division by zero
            if(m_interval<0.01f)
                m_interval = 0.01f;      
        }
        else
        {
            // Reset the values in case that the ball was already trying
            // to get closer to the target, and then the target disappears
            // (e.g. is eliminated or finishes the race).
            m_interval = m_st_interval;
            m_height   = m_max_height;
        }

    }

    // Consider f(x) = x*(x-m_intervall), which is a parabolic function 
    // with f(0) = 0, f(m_intervall)=0. We then scale this function to
    // fulfill: f(m_intervall/2) = m_height, or:
    // f(m_interval/2) = -m_interval^2/4 = m_height
    // --> scale with m_height / -m_interval^2/4
    float f      = m_height / (-0.25f*m_interval*m_interval);
    float height = m_timer * (m_timer-m_interval) * f;
    next_xyz.setY(getHoT() + height);

    // If we start squashing the ball as soon as the height is smaller than
    // height of the ball, it looks to extreme. So a ratio r of the height
    // of the ball and the current height of the object is used to tweak 
    // the look a bit.
    float r = 2.0f;
    if(r*height<m_extend.getY())
        m_node->setScale(core::vector3df(1.0f, r*height/m_extend.getY(),
        1.0f));
    else
        m_node->setScale(core::vector3df(1.0f, 1.0f, 1.0f));

    setXYZ(next_xyz);
}   // update

// -----------------------------------------------------------------------------
/** Determines which coordinates the ball should aim at next. If the ball is
 *  still 'far' away from the target (>20), then it will aim at the next
 *  graph node. If it's closer, the ball will aim directly at the kart and
 *  keep on aiming at the kart, it will not follow the drivelines anymore.
 *  \param aim_xyz On return contains the xyz coordinates to aim at.
 */
void RubberBall::determineTargetCoordinates(float dt, Vec3 *aim_xyz)
{
    // If aiming at target phase, keep on aiming at target.
    // ----------------------------------------------------
    if(m_aiming_at_target)
    {
        *aim_xyz        = m_target->getXYZ();
        return;
    }

    // Aiming at a graph node
    // ----------------------
    GraphNode *gn  = &(m_quad_graph->getNode(m_aimed_graph_node));
    
    // At this stage m_distance_along track is already the new distance (set
    // in the previous time step when aiming). It has to be detected if the
    // ball is now ahead of the graph node, and if so, the graph node has to
    // be updated till it is again ahead of the ball. Three distinct cases
    // have to be considered:
    // 1) The ball just crossed the start line (-> distance close to 0),
    //    but the graph node is still before the start line, in which case
    //    a new graph node has to be determined.
    // 2) The ball hasn't crossed the start line, but the graph node has
    //    (i.e. graph node is 0), in which case the graph node is correct.
    //    This happens after the first iteration, i.e. graph node initially
    //    is the last one (distance close to track length), but if the ball
    //    is ahead (distance of a graph node is measured to the beginning of
    //    the quad, so if the ball is in the middle of the last quad it will
    //    be ahead of the graph node!) the graph node will be set to the 
    //    first graph node (distance close to 0). In this case the graph node
    //    should not be changed anymore.
    // 3) All other cases that do not involve the start line at all 
    //    (including e.g. ball and graph node crossed start line, neither
    //    ball nor graph node crossed start line), which means that a new
    //    graph node need to be determined only if the distance along track
    //    of the ball is greater than the distance for
    float gn_distance  = gn->getDistanceFromStart();
    float track_length = World::getWorld()->getTrack()->getTrackLength();

    // Test 1: ball wrapped around, and graph node is close to end of track
    bool  ball_ahead   = m_wrapped_around && gn_distance >0.9f*track_length;

    // Test 3: distance of ball greater than distance of graph node
    if(!ball_ahead && gn_distance < m_distance_along_track)
        // The distance test only applies if the graph node hasn't wrapped 
        // around
        ball_ahead = true;

    while(ball_ahead)
    {
        // FIXME: aim better if necessary!
        m_aimed_graph_node = gn->getSuccessor(0);
        gn = &(m_quad_graph->getNode(m_aimed_graph_node));

        // Detect a wrap around of the graph node. We could just test if
        // the index of the new graph node is 0, but since it's possible
        // that we might have tracks with a more complicated structure, e.g
        // with two different start lines, we use this more general test:
        // If the previous distance was close to the end of the track, and
        // the new distance is close to 0, the graph node wrapped around.
        // This test prevents an infinite loop if the ball is on the last 
        // quad, in which case no grpah node would fulfill the distance test.
        if(gn_distance > 0.9f*track_length && 
            gn->getDistanceFromStart()<10.0f)
            break;
        gn_distance  = gn->getDistanceFromStart();
        ball_ahead   = m_wrapped_around && gn_distance >0.9f*track_length;
        ball_ahead   = !ball_ahead && 
                        gn->getDistanceFromStart() < m_distance_along_track;
    }

    const LinearWorld *world = dynamic_cast<LinearWorld*>(World::getWorld());
    if(!world) return;   // FIXME battle mode

    float target_distance = 
        world->getDistanceDownTrackForKart(m_target->getWorldKartId());

    // Handle wrap around of distance if target crosses finishing line
    if(m_distance_along_track > target_distance)
        target_distance += world->getTrack()->getTrackLength();

    // If the ball is close enough, start aiming directly at the target kart
    // ---------------------------------------------------------------------
    if(target_distance-m_distance_along_track < 20)
    {
        m_aiming_at_target = true;
        *aim_xyz           = m_target->getXYZ();
        return;
    }

    // ------------------------------
    *aim_xyz = gn->getQuad().getCenter();

}   // determineTargetCoordinates
// ----------------------------------------------------------------------------
/** Callback from the phycis in case that a kart or object is hit. The rubber
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
