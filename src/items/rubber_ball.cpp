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

RubberBall::RubberBall(Kart *kart) 
          : Flyable(kart, PowerupManager::POWERUP_RUBBERBALL, 0.0f /* mass */),
            TrackSector()
{
    float forw_offset = 0.5f*kart->getKartLength() + m_extend.getZ()*0.5f+5.0f;
    
    createPhysics(forw_offset, btVector3(0.0f, 0.0f, m_speed*2),
                  new btSphereShape(0.5f*m_extend.getY()), 
                  -70.0f /*gravity*/, 
                  true /*rotates*/);

    // Do not adjust the up velocity 
    setAdjustUpVelocity(false);
    m_max_lifespan = 9999;
    m_target       = NULL;

    computeTarget();

    // Get 4 points for the interpolation
    // Determine distance along track
    TrackSector::update(getXYZ());
    m_control_points[0]     = m_owner->getXYZ();
    m_control_points[1]     = getXYZ();
    m_last_aimed_graph_node = getSuccessorToHitTarget(getCurrentGraphNode());
    // This call defined m_control_points[3], but also sets a new 
    // m_last_aimed_graph_node, which is further away from the current point,
    // which avoids the problem that the ball might go too quickly to the 
    // left or right when firing the ball off track.
    static int xx=0;
    xx++; if(xx>1) xx=0;
    if(xx) getNextControlPoint();
    m_control_points[2]     = 
        QuadGraph::get()->getQuadOfNode(m_last_aimed_graph_node).getCenter();

    // This updates m_last_aimed_graph_node, and sets m_control_points[3]
    getNextControlPoint();
    m_length_cp_1_2 = (m_control_points[2]-m_control_points[1]).length();

    m_t = 0;
    m_t_increase = m_speed/m_length_cp_1_2;

    // At the start the ball aims at quads till it gets close enough to the
    // target:
    m_aiming_at_target     = false;
    m_wrapped_around       = false;
    m_timer                = 0.0f;
    m_interval             = m_st_interval;
    m_current_max_height   = m_max_height;
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
    while(d<30 && d>0)
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

    if(!m_target)        // Remove this item from the game
    {
        hit(NULL);
        return;
    }
    // FIXME: do we want to test if we have overtaken the target kart?    

    // If we have reached or overshot the next control point, move to the
    // the next section of the spline
    m_t += m_t_increase * dt;
    if(m_t > 1.0f)
    {
        // Move the control points and estimated distance forward.
        for(unsigned int i=1; i<4; i++)
            m_control_points[i-1] = m_control_points[i];
        m_length_cp_1_2 = m_length_cp_2_3;
        int old = m_last_aimed_graph_node;
        // 
        // This automatically sets m_control_points[3]
        getNextControlPoint();
        //printf("1_2 %f  length %f\n", m_length_cp_1_2, 
        //    (m_control_points[2]-m_control_points[1]).length());
        m_t_increase = m_speed/m_length_cp_1_2;
        m_t -= 1.0f;
    }

    Vec3 next_xyz = 0.5f * ((-m_control_points[0] + 3*m_control_points[1] -3*m_control_points[2] + m_control_points[3])*m_t*m_t*m_t
               + (2*m_control_points[0] -5*m_control_points[1] + 4*m_control_points[2] - m_control_points[3])*m_t*m_t
               + (-m_control_points[0]+m_control_points[2])*m_t
               + 2*m_control_points[1]);

    float old_distance = getDistanceFromStart();

    // Determine new distance along track
    TrackSector::update(next_xyz);

    float track_length = World::getWorld()->getTrack()->getTrackLength();

    // Detect if the ball crossed the start line
    m_wrapped_around = old_distance > 0.9f * track_length && 
                       getDistanceFromStart()< 10.0f;

    m_timer += dt;
    float height = updateHeight();
    next_xyz.setY(getHoT()+height);
     
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
        if(distance<50)
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
    GraphNode *gn  = &(QuadGraph::get()->getNode(m_last_aimed_graph_node));
    
    // At this stage getDistanceFromStart() is already the new distance (set
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
    if(!ball_ahead && gn_distance < getDistanceFromStart())
        // The distance test only applies if the graph node hasn't wrapped 
        // around
        ball_ahead = true;

    while(ball_ahead)
    {
        // FIXME: aim better if necessary!
        m_last_aimed_graph_node = getSuccessorToHitTarget(m_last_aimed_graph_node);
        gn = &(QuadGraph::get()->getNode(m_last_aimed_graph_node));
     
        // Detect a wrap around of the graph node. We could just test if
        // the index of the new graph node is 0, but since it's possible
        // that we might have tracks with a more complicated structure, e.g
        // with two different start lines, we use this more general test:
        // If the previous distance was close to the end of the track, and
        // the new distance is close to 0, the graph node wrapped around.
        // This test prevents an infinite loop if the ball is on the last 
        // quad, in which case no graph node would fulfill the distance test.
        if(gn_distance > 0.9f*track_length && 
            gn->getDistanceFromStart()<10.0f)
            break;
        gn_distance  = gn->getDistanceFromStart();
        ball_ahead   = m_wrapped_around && gn_distance >0.9f*track_length;
        ball_ahead   = !ball_ahead && 
                       gn->getDistanceFromStart() < getDistanceFromStart();
    }

    const LinearWorld *world = dynamic_cast<LinearWorld*>(World::getWorld());
    if(!world) return;   // FIXME battle mode

    float target_distance = 
        world->getDistanceDownTrackForKart(m_target->getWorldKartId());

    // Handle wrap around of distance if target crosses finishing line
    if(getDistanceFromStart() > target_distance)
        target_distance += world->getTrack()->getTrackLength();

    // If the ball is close enough, start aiming directly at the target kart
    // ---------------------------------------------------------------------
    if(target_distance-getDistanceFromStart()< 20)
    {
        m_aiming_at_target = true;
        *aim_xyz           = m_target->getXYZ();
        return;
    }

    // ------------------------------
    *aim_xyz = gn->getQuad().getCenter();

}   // determineTargetCoordinates

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
