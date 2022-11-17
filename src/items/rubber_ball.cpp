//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011-2015 Joerg Henrichs
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

#include "audio/sfx_base.hpp"
#include "audio/sfx_manager.hpp"
#include "config/stk_config.hpp"
#include "config/user_config.hpp"
#include "io/xml_node.hpp"
#include "items/attachment.hpp"
#include "items/projectile_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/kart_properties.hpp"
#include "modes/linear_world.hpp"
#include "network/network_string.hpp"
#include "network/rewind_info.hpp"
#include "network/rewind_manager.hpp"
#include "physics/btKart.hpp"
#include "physics/triangle_mesh.hpp"
#include "tracks/check_manager.hpp"
#include "tracks/drive_graph.hpp"
#include "tracks/drive_node.hpp"
#include "tracks/track.hpp"
#include "utils/stk_process.hpp"

#include "utils/log.hpp" //TODO: remove after debugging is done

#include <ISceneNode.h>

float RubberBall::m_st_interval;
float RubberBall::m_st_min_interpolation_distance;
float RubberBall::m_st_squash_duration;
float RubberBall::m_st_squash_slowdown;
float RubberBall::m_st_target_distance;
float RubberBall::m_st_target_max_angle;
int16_t RubberBall::m_st_delete_ticks;
float RubberBall::m_st_max_height_difference;
float RubberBall::m_st_fast_ping_distance;
float RubberBall::m_st_early_target_factor;
float RubberBall::m_st_min_speed_offset;
float RubberBall::m_st_max_speed_offset;
float RubberBall::m_st_min_offset_distance;
float RubberBall::m_st_max_offset_distance;

// Debug only, so that we can get a feel on how well balls are aiming etc.
#undef PRINT_BALL_REMOVE_INFO

RubberBall::RubberBall(AbstractKart *kart)
          : Flyable(kart, PowerupManager::POWERUP_RUBBERBALL, 0.0f /* mass */),
            TrackSector()
{
    // For debugging purpose: pre-fix each debugging line with the id of
    // the ball so that it's easy to collect all debug output for one
    // particular ball only.
    static int next_id[PT_COUNT] = {};
    m_id = next_id[STKProcess::getType()]++;

    m_target = NULL;
    m_ping_sfx = SFXManager::get()->createSoundSource("ball_bounce");
    fixSFXSplitscreen(m_ping_sfx);
}   // RubberBall

// ----------------------------------------------------------------------------
void RubberBall::onFireFlyable()
{
    Flyable::onFireFlyable();
    Track::getCurrentTrack()->getCheckManager()->addFlyableToCannons(this);
    // Don't let Flyable update the terrain information, since this object
    // has to do it earlier than that.
    setDoTerrainInfo(false);
    float forw_offset =
        0.5f * m_owner->getKartLength() + m_extend.getZ() * 0.5f + 5.0f;

    createPhysics(forw_offset, btVector3(0.0f, 0.0f, m_speed*2),
                  new btSphereShape(0.5f*m_extend.getY()), -70.0f,
                  btVector3(.0f,.0f,.0f) /*gravity*/,
                  true /*rotates*/);

    // add no_contact_response flags, so that the ball
    // will not explode if hitting the track or objects
    int flag = getBody()->getCollisionFlags();
    flag |= btCollisionObject::CF_NO_CONTACT_RESPONSE;
    getBody()->setCollisionFlags(flag);

    // Do not adjust the up velocity
    setAdjustUpVelocity(false);
    m_max_lifespan       = stk_config->time2Ticks(9999);
    m_target             = NULL;
    m_aiming_at_target   = false;
    m_fast_ping          = false;
    // At the start the ball aims at quads till it gets close enough to the
    // target:
    m_height_timer       = 0.0f;
    m_interval           = m_st_interval;
    m_current_max_height = m_max_height;
    // Just init the previous coordinates with some value that's not getXYZ()
    m_previous_xyz       = m_owner->getXYZ();
    m_previous_height    = 2.0f;  //
    // A negative value indicates that the timer is not active
    m_delete_ticks       = -1;
    m_tunnel_count       = 0;

    LinearWorld *world = dynamic_cast<LinearWorld*>(World::getWorld());
    // FIXME: what does the rubber ball do in case of battle mode??
    if(!world) return;

    m_restoring_state = true;
    computeTarget();

    // initialises the current graph node
    TrackSector::update(getXYZ());
    const Vec3& normal =
        DriveGraph::get()->getNode(getCurrentGraphNode())->getNormal();
    TerrainInfo::update(getXYZ(), -normal);
    initializeControlPoints(m_owner->getXYZ());
}   // onFireFlyable

// ----------------------------------------------------------------------------
/** Destructor, removes any playing sfx.
 */
RubberBall::~RubberBall()
{
    removePingSFX();
    Track::getCurrentTrack()->getCheckManager()->removeFlyableFromCannons(this);
}   // ~RubberBall

// ----------------------------------------------------------------------------
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
        DriveGraph::get()->getNode(m_last_aimed_graph_node)->getCenter();

    // This updates m_last_aimed_graph_node, and sets m_control_points[3]
    getNextControlPoint();
    m_length_cp_1_2 = (m_control_points[2]-m_control_points[1]).length();
    m_t             = 0;
    m_t_increase    = m_speed/m_length_cp_1_2;
}   // initializeControlPoints

// ----------------------------------------------------------------------------
void RubberBall::setAnimation(AbstractKartAnimation *animation)
{
    if (!animation)
    {
        initializeControlPoints(getXYZ());
        m_height_timer = 0;
    }
    Flyable::setAnimation(animation);
}   // setAnimation

// ----------------------------------------------------------------------------
/** Determines the first kart that is still in the race.
 */
void RubberBall::computeTarget()
{
    LinearWorld *world = dynamic_cast<LinearWorld*>(World::getWorld());

    if (m_restoring_state)
    {
        // Update kart position from rewind data 1st time
        world->updateTrackSectors();
        world->updateRacePosition();
        m_restoring_state = false;
    }

    for (unsigned int p = 1; p < world->getNumKarts() + 1; p++)
    {
        m_target = world->getKartAtPosition(p);
        if(!m_target->isEliminated() && !m_target->hasFinishedRace())
        {
            // If the firing kart itself is the first kart (that is
            // still driving), prepare to remove the rubber ball
            if(m_target==m_owner && m_delete_ticks < 0)
            {
#ifdef PRINT_BALL_REMOVE_INFO
                Log::debug("[RubberBall]",
                           "ball %d removed because owner is target.", m_id);
#endif
                m_delete_ticks = m_st_delete_ticks;
            }
            return;
        }
    }   // for p > num_karts

    // This means it must be the end-animation phase. Now just
    // aim at the owner (the ball is unlikely to hit it), and
    // this will trigger the usage of the delete time in updateAndDelete
#ifdef PRINT_BALL_REMOVE_INFO
    Log::debug("[RubberBall]" "ball %d removed because no more active target.",
               m_id);
#endif
    m_delete_ticks = m_st_delete_ticks;
    m_target       = m_owner;
}   // computeTarget

// ----------------------------------------------------------------------------
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
    int succ = 0;
    LinearWorld *lin_world = dynamic_cast<LinearWorld*>(World::getWorld());

    unsigned int sect =
        lin_world->getSectorForKart(m_target);
    succ = DriveGraph::get()->getNode(node_index)->getSuccessorToReach(sect);

    if(dist)
        *dist += DriveGraph::get()->getNode(node_index)
                 ->getDistanceToSuccessor(succ);
    return DriveGraph::get()->getNode(node_index)->getSuccessor(succ);
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

    float f = DriveGraph::get()->getDistanceFromStart(m_last_aimed_graph_node);

    int next = getSuccessorToHitTarget(m_last_aimed_graph_node, &dist);
    float d = DriveGraph::get()->getDistanceFromStart(next)-f;
    while(d<m_st_min_interpolation_distance && d>=0)
    {
        next = getSuccessorToHitTarget(next, &dist);
        d = DriveGraph::get()->getDistanceFromStart(next)-f;
    }

    m_last_aimed_graph_node = next;
    m_length_cp_2_3         = dist;
    const DriveNode* dn     =
        DriveGraph::get()->getNode(m_last_aimed_graph_node);
    m_control_points[3]     = dn->getCenter();
}   // getNextControlPoint

// ----------------------------------------------------------------------------
/** Initialises this object with data from the power.xml file (this is a static
 *  function).
 *  \param node XML Node
 *  \param rubberball The rubber ball mesh
 */
void RubberBall::init(const XMLNode &node, scene::IMesh *rubberball)
{
    m_st_interval                   =   1.0f;
    m_st_squash_duration            =   3.0f;
    m_st_squash_slowdown            =   0.5f;
    m_st_min_interpolation_distance =  30.0f;
    m_st_target_distance            =  50.0f;
    m_st_target_max_angle           =  25.0f;
    m_st_delete_ticks               = (int16_t)stk_config->time2Ticks(10.0f);
    m_st_max_height_difference      =  10.0f;
    m_st_fast_ping_distance         =  50.0f;
    m_st_early_target_factor        =   1.0f;
    m_st_min_speed_offset           =   8.0f;
    m_st_max_speed_offset           =  28.0f;
    m_st_min_offset_distance        =  50.0f;
    m_st_max_offset_distance        = 250.0f;


    if(!node.get("interval", &m_st_interval))
        Log::warn("powerup", "No interval specified for basket ball.");
    if(!node.get("squash-duration", &m_st_squash_duration))
        Log::warn("powerup",
                  "No squash-duration specified for basket ball.");
    if(!node.get("squash-slowdown", &m_st_squash_slowdown))
        Log::warn("powerup", "No squash-slowdown specified for basket ball.");
    if(!node.get("min-interpolation-distance",
                 &m_st_min_interpolation_distance))
        Log::warn("powerup", "No min-interpolation-distance specified "
                             "for basket ball.");
    if(!node.get("target-distance", &m_st_target_distance))
        Log::warn("powerup",
                  "No target-distance specified for basket ball.");
    float f;
    if(!node.get("delete-time", &f))
        Log::warn("powerup", "No delete-time specified for basket ball.");
    m_st_delete_ticks = stk_config->time2Ticks(f);
    if(!node.get("target-max-angle", &m_st_target_max_angle))
        Log::warn("powerup", "No target-max-angle specified for basket ball.");
    m_st_target_max_angle *= DEGREE_TO_RAD;
    if(!node.get("max-height-difference", &m_st_max_height_difference))
        Log::warn("powerup",
                  "No max-height-difference specified for basket ball.");
    if(!node.get("fast-ping-distance", &m_st_fast_ping_distance))
        Log::warn("powerup",
                  "No fast-ping-distance specified for basket ball.");
    if(m_st_fast_ping_distance < m_st_target_distance)
        Log::warn("powerup",
                   "Ping-distance is smaller than target distance.\n"
                   "That should not happen, but is ignored for now.");
    if(!node.get("early-target-factor", &m_st_early_target_factor))
        Log::warn("powerup",
                  "No early-target-factor specified for basket ball.");
    if(!node.get("min-speed-offset", &m_st_min_speed_offset))
        Log::warn("powerup", "No min-speed-offset specified for basket ball.");
    if(!node.get("max-speed-offset", &m_st_max_speed_offset))
        Log::warn("powerup", "No max-speed-offset specified for basket ball.");
    if(!node.get("min-offset-distance", &m_st_min_offset_distance))
        Log::warn("powerup", "No min-offset-distance specified for basket ball.");
    if(!node.get("max-offset-distance", &m_st_max_offset_distance))
        Log::warn("powerup", "No max-offset-distance specified for basket ball.");


    //sanity checks
    if (m_st_min_speed_offset < 10.0f)
        m_st_min_speed_offset = 10.0f;
    if (m_st_min_speed_offset > m_st_max_speed_offset)
    {
        m_st_max_speed_offset = m_st_min_speed_offset;
    }
    if (m_st_max_offset_distance <= (m_st_min_offset_distance + 10.0f))
    {
        m_st_max_offset_distance = m_st_min_offset_distance + 10.0f;
    }

    Flyable::init(node, rubberball, PowerupManager::POWERUP_RUBBERBALL);
}   // init

// ----------------------------------------------------------------------------
/** Updates the rubber ball.
 *  \param dt Time step size.
 *  \returns True if the rubber ball should be removed.
 */
bool RubberBall::updateAndDelete(int ticks)
{
    LinearWorld *world = dynamic_cast<LinearWorld*>(World::getWorld());
    // FIXME: what does the rubber ball do in case of battle mode??
    if(!world) return true;

    if(m_delete_ticks>0)
    {
        m_delete_ticks -= 1;
        if(m_delete_ticks<=0)
        {
            hit(NULL);
#ifdef PRINT_BALL_REMOVE_INFO
            Log::debug("[RubberBall]", "ball %d deleted.", m_id);
#endif
            removePingSFX();
            return true;
        }
    }

    if (hasAnimation())
    {
        // Flyable will call update() of the animation to 
        // update the ball's position.
        m_previous_xyz = getXYZ();
        bool can_be_deleted = Flyable::updateAndDelete(ticks);
        if (can_be_deleted)
            removePingSFX();
        return can_be_deleted;
    }

    bool can_be_deleted = Flyable::updateAndDelete(ticks);
    if (can_be_deleted)
    {
        removePingSFX();
        return true;
    }

    // Update normal from rewind first
    const Vec3& normal =
        DriveGraph::get()->getNode(getCurrentGraphNode())->getNormal();
    TerrainInfo::update(getXYZ(), -normal);

    // Update the target in case that the first kart was overtaken (or has
    // finished the race).
    computeTarget();
    updateDistanceToTarget();

    //Update the speed
    updateWeightedSpeed(ticks);

    // Determine the new position. This new position is only temporary,
    // since it still needs to be adjusted for the height of the terrain.
    Vec3 next_xyz;
    if(m_aiming_at_target)
        moveTowardsTarget(&next_xyz, ticks);
    else
        interpolate(&next_xyz, ticks);

    // If the ball is close to the ground, we have to start the raycast
    // slightly higher (to avoid that the ball tunnels through the floor).
    // But if the ball is close to the ceiling of a tunnel and we would
    // start the raycast slightly higher, the ball might end up on top
    // of the ceiling.
    // The ball is considered close to the ground if the height above the
    // terrain is less than half the current maximum height.
    bool close_to_ground = 2.0*m_previous_height < m_current_max_height;

    float vertical_offset = close_to_ground ? 4.0f : 2.0f;

    // Update height of terrain (which isn't done as part of
    // Flyable::update for basket balls.
    TerrainInfo::update(next_xyz + getNormal()*vertical_offset, -getNormal());

    m_height_timer += stk_config->ticks2Time(ticks);
    float height    = updateHeight()+m_extend.getY()*0.5f;
    
    if(UserConfigParams::logFlyable())
        Log::debug("[RubberBall]", "ball %d: %f %f %f height %f gethot %f terrain %d aim %d",
                m_id, next_xyz.getX(), next_xyz.getY(), next_xyz.getZ(), height, getHoT(),
			    isOnRoad(),
            m_aiming_at_target);

    // No need to check for terrain height if the ball is low to the ground
    if(height > 0.5f)
    {
        float tunnel_height = getTunnelHeight(next_xyz, vertical_offset)
                             - m_extend.getY();
        // If the current height of ball (above terrain) is higher than the 
        // tunnel height then set adjust max height and compute new height again.
        // Else reset the max height.
        if (height > tunnel_height)
        {
            m_max_height = tunnel_height;
            height = updateHeight();
        }
        else
            m_max_height = m_st_max_height[m_type];
    }

    if(UserConfigParams::logFlyable())
        Log::verbose("RubberBall", "newy2 %f gmth %f", height,
                     getTunnelHeight(next_xyz,vertical_offset));

    // Ball squashing:
    // ===============
    float scale = 1.0f;
    if (height < 1.0f * m_extend.getY())
        scale = height / m_extend.getY();

#ifndef SERVER_ONLY
    if (m_node && !RewindManager::get()->isRewinding())
        m_node->setScale(core::vector3df(1.0f, scale, 1.0f));
#endif

    next_xyz = getHitPoint() + getNormal() * (height * scale);
    m_previous_xyz = getXYZ();
    m_previous_height = (getXYZ() - getHitPoint()).length();
    setXYZ(next_xyz);

    if (checkTunneling())
    {
        removePingSFX();
        return true;
    }

    // Determine new distance along track
    TrackSector::update(next_xyz);

    return false;
}   // updateAndDelete

// ----------------------------------------------------------------------------
/** Moves the rubber ball in a straight line towards the target. This is used
 *  once the rubber ball is close to its target. It restricts the angle by
 *  which the rubber ball can change its direction per frame.
 *  \param next_xyz The position the ball should move to.
 *  \param ticks Number of physics steps - should be 1.
 */
void RubberBall::moveTowardsTarget(Vec3 *next_xyz, int ticks)
{
    // If the rubber ball is already close to a target, i.e. aiming
    // at it directly, stop interpolating, instead fly straight
    // towards it.
    Vec3 diff = m_target->getXYZ() - getXYZ();
    diff = diff - diff.dot(getNormal())*getNormal();
    // Avoid potential division by zero
    if(diff.length2()==0)
        *next_xyz = getXYZ() - getNormal()*m_previous_height;
    else
    {
        float dt = stk_config->ticks2Time(ticks);
        *next_xyz = getXYZ() - getNormal()*m_previous_height
                             + (dt*m_speed / diff.length())*diff;
    }

    // If ball is close to the target, then explode
    if (diff.length() < m_target->getKartLength())
        hit((AbstractKart*)m_target);

    assert(!std::isnan((*next_xyz)[0]));
    assert(!std::isnan((*next_xyz)[1]));
    assert(!std::isnan((*next_xyz)[2]));

}   // moveTowardsTarget

// ----------------------------------------------------------------------------
/** Calculates the current speed of the basket ball.
 *  The base basket ball speed is the target's normal max speed.
 *  Then an offset is applied.
 *  If closer than m_st_min_offset_distance ; it is m_st_min_speed_offset
 *  If farther than m_st_max_offset_distance ; it is m_st_max_speed_offset
 *  Else, it is a weighted average.
 *  It takes up to 5 seconds to fully change its speed
 *  (to avoid sudden big speed changes when the distance change suddenly
 *  at ball throwing, because of paths, shortcuts, etc.)
 *  If there is no target, it will keep its current speed
 */
void RubberBall::updateWeightedSpeed(int ticks)
{
    // Don't change the speed if there is no target
    if (m_delete_ticks >= 10)
    {
        return;
    }
    const KartProperties *kp = m_target->getKartProperties();
    // Get the speed depending on difficulty but not on kart type/handicap
    float targeted_speed = kp->getEngineGenericMaxSpeed();

    float dt = stk_config->ticks2Time(ticks);

    //Calculate the targeted weighted speed
    if (m_distance_to_target <= m_st_min_offset_distance)
    {
        targeted_speed += m_st_min_speed_offset;
    }
    else if (m_distance_to_target > m_st_max_offset_distance)
    {
        targeted_speed += m_st_max_speed_offset;
    }
    else
    {
        float weight = 1.0f;
        float offset = 0.0f;
        weight = (m_st_max_offset_distance - m_distance_to_target)
                /(m_st_max_offset_distance - m_st_min_offset_distance);
        offset = weight * m_st_min_speed_offset  + (1-weight) * m_st_max_speed_offset;
        targeted_speed += offset;
    }
    //Update the real speed
    //Always >= 0
    float max_change = (m_st_max_speed_offset - m_st_min_speed_offset)*dt/(5.0f);
    if (m_speed-targeted_speed <= max_change && m_speed-targeted_speed >= -max_change)
    {
        m_speed = targeted_speed;
    }
    else if (m_speed < targeted_speed)
    {
        m_speed += max_change;
    }
    else
    {
        m_speed -= max_change;
    }
}   // updateWeightedSpeed


// ----------------------------------------------------------------------------
/** Uses Hermite splines (Catmull-Rom) to interpolate the position of the
 *  ball between the control points. If the next point would be outside of
 *  the spline between control_points[1] and [2], a new control point is
 *  added.
 *  \param next_xyz Returns the new position.
 *  \param The time step size.
 */
void RubberBall::interpolate(Vec3 *next_xyz, int ticks)
{
    // If we have reached or overshot the next control point, move to the
    // the next section of the spline
    float dt = stk_config->ticks2Time(ticks);
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

    *next_xyz = 0.5f * ((-  m_control_points[0] + 3*m_control_points[1]
                         -3*m_control_points[2] +   m_control_points[3] )
                        *m_t*m_t*m_t
                      + ( 2*m_control_points[0] -5*m_control_points[1]
                         +4*m_control_points[2] -  m_control_points[3])*m_t*m_t
                      + (-  m_control_points[0] +  m_control_points[2])*m_t
                      +   2*m_control_points[1]                              );
                      
    assert(!std::isnan((*next_xyz)[0]));
    assert(!std::isnan((*next_xyz)[1]));
    assert(!std::isnan((*next_xyz)[2]));
}   // interpolate

// ----------------------------------------------------------------------------
/** Checks if the line from the previous ball position to the new position
 *  hits something, which indicates that the ball is tunneling through. If
 *  this happens, the ball position is adjusted so that it is just before
 *  the hit point. If tunneling happens four frames in a row the ball is
 *  considered stuck and explodes (e.g. the ball might try to tunnel through
 *  a wall to get to a 'close' target. In this case the ball would not
 *  move much anymore and be stuck).
 *  \return True if the ball tunneled often enough to be removed.
 */
bool RubberBall::checkTunneling()
{
    const TriangleMesh &tm = Track::getCurrentTrack()->getTriangleMesh();
    Vec3 hit_point;
    const Material *material;

    tm.castRay(m_previous_xyz, getXYZ(), &hit_point, &material);

    if(material)
    {
        // If there are three consecutive tunnelling
        m_tunnel_count++;
        if(m_tunnel_count > 3)
        {
#ifdef PRINT_BALL_REMOVE_INFO
            Log::debug("[RubberBall]",
                       "Ball %d nearly tunneled at %f %f %f -> %f %f %f",
                        m_id, m_previous_xyz.getX(),m_previous_xyz.getY(),
                        m_previous_xyz.getZ(),
                        getXYZ().getX(),getXYZ().getY(),getXYZ().getZ());
#endif
            hit(NULL);
            return true;
        }
        // In case of a hit, move the hit point towards the
        // previous point by the radius of the ball --> this
        // point will just allow the ball to avoid tunneling.
        Vec3 diff = m_previous_xyz - hit_point;
        hit_point += diff * (1.1f*m_extend.getY()/diff.length());
        setXYZ(hit_point);
        return false;
    }
    else
        m_tunnel_count = 0;
    return false;
}   // checkTunneling

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
    if(m_height_timer>m_interval)
    {
        m_height_timer -= m_interval;
        if (m_ping_sfx && m_ping_sfx->getStatus()!=SFXBase::SFX_PLAYING &&
            !RewindManager::get()->isRewinding())
        {
            m_ping_sfx->setPosition(getXYZ());
            m_ping_sfx->play();
        }

        if(m_fast_ping)
        {
            // Some experimental formulas
            m_current_max_height = 0.5f*sqrt(m_distance_to_target);
            // If the ball just missed the target, m_distance_to_target
            // can be huge (close to track length) due to the order in
            // which a lost target is detected. Avoid this by clamping
            // m_current_max_height.
            if(m_current_max_height>m_max_height)
                m_current_max_height = m_max_height;
            m_interval           = m_current_max_height / 10.0f;
            // Avoid too small hops and esp. a division by zero
            if(m_interval<0.01f)
                m_interval = 0.1f;
        }
        else
        {
            // Reset the values in case that the ball was already trying
            // to get closer to the target, and then the target disappears
            // (e.g. is eliminated or finishes the race).
            m_interval           = m_st_interval;
            m_current_max_height = m_max_height;
        }
    }   // if m_height_timer > m_interval


    // Determine the height of the ball
    // ================================
    // Consider f(x) = s * x*(x-m_intervall), which is a parabolic function
    // with f(0) = 0, f(m_intervall)=0. We then scale this function to
    // fulfill: f(m_intervall/2) = max_height, or:
    // f(m_interval/2) = s*(-m_interval^2)/4 = max_height
    // --> s =  4*max_height / -m_interval^2
    float s = 4.0f * m_current_max_height / (-m_interval*m_interval);
    return m_height_timer * (m_height_timer-m_interval) * s;
}   // updateHeight

// ----------------------------------------------------------------------------
/** When the ball is in a tunnel, this will return the tunnel height.
 *  NOTE: When this function is called next_xyz is usually the interpolated point 
 *  on the track and not the ball's current location. Look at updateAndDelete().
 *  
 *  \param vertical_offset A vertical offset which is added to the current
 *         position in order to avoid hitting the track when doing a raycast up.
 *  \returns The distance to the terrain element found by raycast in the up
              direction. If no terrain found, it returns 99990
 */
float RubberBall::getTunnelHeight(const Vec3 &next_xyz, 
                                  const float vertical_offset) const
{
    const TriangleMesh &tm = Track::getCurrentTrack()->getTriangleMesh();
    Vec3 from(next_xyz + vertical_offset*getNormal());
    Vec3 to(next_xyz + 10000.0f*getNormal());
    Vec3 hit_point;
    const Material *material;
    tm.castRay(from, to, &hit_point, &material);

    return (material) ? (hit_point - next_xyz).length() : 99999.f;
}   // getTunnelHeight

// ----------------------------------------------------------------------------
/** Determines the distance to the target kart. If the target is close, the
 *  rubber ball will switch from following the quad graph structure to
 *  directly aim at the target.
 */
void RubberBall::updateDistanceToTarget()
{
    const LinearWorld *world = dynamic_cast<LinearWorld*>(World::getWorld());

    float target_distance =
        world->getDistanceDownTrackForKart(m_target->getWorldKartId(), true);
    float ball_distance = getDistanceFromStart(/* account for checklines */ false);

    m_distance_to_target = target_distance - ball_distance;
    if(m_distance_to_target < 0)
    {
        m_distance_to_target += Track::getCurrentTrack()->getTrackLength();
    }
    if(UserConfigParams::logFlyable())
        Log::debug("[RubberBall]", "ball %d: target %f %f %f distance_2_target %f",
        m_id, m_target->getXYZ().getX(),m_target->getXYZ().getY(),
        m_target->getXYZ().getZ(),m_distance_to_target
        );

    float height_diff = fabsf((m_target->getXYZ() - getXYZ()).dot(getNormal().normalized()));

    if(m_distance_to_target < m_st_fast_ping_distance &&
        height_diff < m_st_max_height_difference)
    {
        m_fast_ping = true;
    }
    else
    {
        m_fast_ping = false;
    }
    if(m_distance_to_target < m_st_target_distance &&
        height_diff < m_st_max_height_difference)
    {
        m_aiming_at_target = true;
        return;
    }
    else if(m_aiming_at_target)
    {
        // It appears that we have lost the target. It was within
        // the target distance, and now it isn't. That means either
        // the original target escaped, or perhaps that there is a
        // new target. If the new distance is nearly the full track
        // length, assume that the rubber ball has overtaken the
        // original target, and start deleting it.
        if(m_distance_to_target > 0.9f * Track::getCurrentTrack()->getTrackLength())
        {
            m_delete_ticks = m_st_delete_ticks;
#ifdef PRINT_BALL_REMOVE_INFO
            Log::debug("[RubberBall]", "ball %d lost target (overtook?).",
                        m_id);
#endif

        }

        // Otherwise (target disappeared, e.g. has finished the race or
        // was eliminated) we have to reset the control points, since
        // it's likely that the ball is (after some time going directly
        // towards the target) far outside of the old control points.

        // We use the previous XYZ point to define the first control
        // point, which results in a smooth transition from aiming
        // directly at the target back to interpolating again.
        initializeControlPoints(m_previous_xyz);
        m_aiming_at_target = false;
    }

    return;
}   // updateDistanceToTarget

// ----------------------------------------------------------------------------
/** Callback from the physics in case that a kart or object is hit. The rubber
 *  ball will only be removed if it hits it target, all other karts it might
 *  hit earlier will only be flattened.
 *  \params kart The kart hit (NULL if no kart was hit).
 *  \params object The object that was hit (NULL if none).
 *  \returns True if
 */
bool RubberBall::hit(AbstractKart* kart, PhysicalObject* object)
{
    // When moved to infinity during cannon animation do nothing
    if (hasAnimation())
        return false;
#ifdef PRINT_BALL_REMOVE_INFO
    if(kart)
        Log::debug("[RuberBall]", "ball %d hit kart.", m_id);
#endif
    if(kart && kart!=m_target)
    {
        // If the squashed kart has a bomb, explode it.
        if(kart->getAttachment()->getType()==Attachment::ATTACH_BOMB)
        {
            // make bomb explode
            kart->getAttachment()->update(10000);
            return false;
        }
        kart->setSquash(m_st_squash_duration, m_st_squash_slowdown);
        return false;
    }
    bool was_real_hit = Flyable::hit(kart, object);
    if(was_real_hit)
    {
        if(kart && kart->isShielded())
        {
            kart->decreaseShieldTime();
        }
        else
            explode(kart, object);
    }
    return was_real_hit;
}   // hit

// ----------------------------------------------------------------------------
BareNetworkString* RubberBall::saveState(std::vector<std::string>* ru)
{
    BareNetworkString* buffer = Flyable::saveState(ru);
    if (!buffer)
        return NULL;

    buffer->addUInt16((int16_t)m_last_aimed_graph_node);
    buffer->add(m_control_points[0]);
    buffer->add(m_control_points[1]);
    buffer->add(m_control_points[2]);
    buffer->add(m_control_points[3]);
    buffer->add(m_previous_xyz);
    buffer->addFloat(m_previous_height);
    buffer->addFloat(m_length_cp_1_2);
    buffer->addFloat(m_length_cp_2_3);
    buffer->addFloat(m_t);
    buffer->addFloat(m_t_increase);
    buffer->addFloat(m_interval);
    buffer->addFloat(m_height_timer);
    buffer->addUInt16(m_delete_ticks);
    buffer->addFloat(m_current_max_height);
    buffer->addUInt8(m_tunnel_count | (m_aiming_at_target ? (1 << 7) : 0));
    TrackSector::saveState(buffer);
    return buffer;
}   // saveState

// ----------------------------------------------------------------------------
void RubberBall::restoreState(BareNetworkString *buffer, int count)
{
    Flyable::restoreState(buffer, count);
    m_restoring_state = true;
    int16_t last_aimed_graph_node = buffer->getUInt16();
    m_last_aimed_graph_node = last_aimed_graph_node;
    m_control_points[0] = buffer->getVec3();
    m_control_points[1] = buffer->getVec3();
    m_control_points[2] = buffer->getVec3();
    m_control_points[3] = buffer->getVec3();
    m_previous_xyz = buffer->getVec3();
    m_previous_height = buffer->getFloat();
    m_length_cp_1_2 = buffer->getFloat();
    m_length_cp_2_3 = buffer->getFloat();
    m_t = buffer->getFloat();
    m_t_increase = buffer->getFloat();
    m_interval = buffer->getFloat();
    m_height_timer = buffer->getFloat();
    m_delete_ticks = buffer->getUInt16();
    m_current_max_height = buffer->getFloat();
    uint8_t tunnel_and_aiming = buffer->getUInt8();
    m_tunnel_count = tunnel_and_aiming & 127;
    m_aiming_at_target = ((tunnel_and_aiming >> 7) & 1) == 1;
    TrackSector::rewindTo(buffer);
}   // restoreState

// ----------------------------------------------------------------------------
void RubberBall::removePingSFX()
{
    if (!m_ping_sfx)
        return;
    if (m_ping_sfx->getStatus() == SFXBase::SFX_PLAYING)
        m_ping_sfx->stop();
    m_ping_sfx->deleteSFX();
    m_ping_sfx = NULL;
}   // removePingSFX
