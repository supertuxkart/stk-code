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

#ifndef HEADER_RUBBER_BALL_HPP
#define HEADER_RUBBER_BALL_HPP

#include <irrString.h>

#include "items/flyable.hpp"
#include "tracks/track_sector.hpp"
#include "utils/cpp2011.hpp"

class AbstractKart;
class SFXBase;

/**
  * \ingroup items
  */
class RubberBall: public Flyable, public TrackSector
{
private:
    /** Used in case of flyable debugging so that each output line gets
     *  a unique number for each ball. */
    int m_id;

    /** A class variable to store the default interval size. */
    static float m_st_interval;

    /** A class variable to store the default squash duration. */
    static float m_st_squash_duration;

    /** A class variable to store the default squash slowdown. */
    static float m_st_squash_slowdown;

    /** If the ball is closer than this distance to the target, it will
     *  start to aim directly at the target (and not interpolate anymore). */
    static float m_st_target_distance;

    /** In somce track, e.g. subsea track, it is possible that the karts
     *  are on different parts of the track (different shortcuts), but
     *  still close to each other - because one of the parts is just
     *  on top of the other part. Therefore use the height as additional
     *  criteria to determine if a ball is close to its target. */
    static float m_st_max_height_difference;

    /** Distance between ball and target at which the ball will start to
     *  bounce faster (which makes more 'ping' sfx for the driver to
     *  hear it coming closer, but also higher probability to hit the
     *  target and not fly over it). */
    static float m_st_fast_ping_distance;

    /** The maximum angle the ball can change per second when being close
     *  to the taregt. If the target angle is small, it makes it much harder
     *  to hit the target, giving it some chances to escape. */
    static float m_st_target_max_angle;

    /** Each control point chosen must be at least this far away from
     *  the previous one. This gives smooth 'overall' interpolation
     *  even if the quads should be close to each other. */
    static float m_st_min_interpolation_distance;

    /** If the ball overtakes its target or starts to aim at the kart which
     *  originally shot the rubber ball, after this amount of time the
     *  ball will be deleted. */
    static int16_t m_st_delete_ticks;

    /** If the ball is closer to its target than min_offset_distance, the speed
     *  in addition to the difficulty's default max speed. */
    static float m_st_min_speed_offset;

    /** If the ball is farther to its target than max_offset_distance, the speed
     *  in addition to the difficulty's default max speed. */
    static float m_st_max_speed_offset;

    /** The distance to target under which the ball is the slowest */
    static float m_st_min_offset_distance;

    /** The distance to target over which the ball is the fastest */
    static float m_st_max_offset_distance;

    /** This factor is used to influence how much the rubber ball should aim
     *  at its target early. It used the 'distance to center of track' of its
     *  target, and adjusts the interpolation control points to be more or
     *  less at the same (relative) distance from center. If the factor is
     *  1, the rubber ball will aim to be at the same relative distance,
     *  if the factor is 0, the rubber ball will aim directly at the
     *  driveline points. A factor of 1 usually means that by the time
     *  the ball starts aiming directly at the target it is (nearly) on the
     *  same 'line', meaning it only has to go straight. On the other hand
     *  in a tunnel this might result in the ball being too far to the
     *  side, increasing the likelihood of the ball tunneling through
     *  (which can happen when the ball switches to aim-at-target mode,
     *  in a tight curve, so that the direct line to the target goes through
     *  a wall. */
    static float m_st_early_target_factor;

    /** A pointer to the target kart. */
    const AbstractKart  *m_target;

    /** The last graph node who's coordinates are stored in
     *  m_control_points[3]. */
    int          m_last_aimed_graph_node;

    /** Keep the last two, current, and next aiming points
     *  for interpolation. */
    Vec3         m_control_points[4];

    /** Saves the previous location of the ball. This is needed if a ball
     *  should lose it target, and has to reinitialise the control points
     *  for the interpolation. */
    Vec3         m_previous_xyz;

    /** To simplify code this stores the previous height above terrain
     *  used. */
    float        m_previous_height;

    /** Estimated length of the spline between the control points
     *  1 and 2. */
    float        m_length_cp_1_2;

    /** Estimated length of the spline between the control points
     *  2 and 3. */
    float        m_length_cp_2_3;

    /** The parameter for the spline, m_t in [0,1]. This is not directly
     *  related to the time, since depending on the distance between
     *  the two control points different increments must be used for m_t.
     *  For example, if the distance is 10 m, and assuming a speed of
     *  10 m/s for the ball, then each second must add '1' to m_t. If
     *  the distance on the other hand is 200 m, then 10/200 = 1/20 per
     *  second must be added (so that it takes 20 seconds to move from 0
     *  to 1). */
    float        m_t;

    /** How much m_t must increase per second in order to maintain a
     *  constant speed, i.e. the speed of the ball divided by the
     *  distance between the control points. See m_t for more details. */
    float        m_t_increase;

    /** How long it takes from one bounce of the ball to the next. */
    float        m_interval;

    /** Distance to target. This is measured in terms of 'distance along
     *  track', but also takes the 3d distance and height difference into
     *  account (in case that the target is on a different part of the
     *  track) */
    float        m_distance_to_target;

    /** This timer is used to determine the height depending on the time.
     *  It is always between 0 and m_interval. */
    float        m_height_timer;

    /** The current maximum height of the ball. This value will be
     *  reduced if the ball gets closer to the target. */
    float        m_current_max_height;

    /** If the ball overtakes its target or starts to aim at the kart which
     *  originally shot the rubber ball, after a certain amount of time the
     *  ball will be deleted. This timer tracks this time. If it is < 0
     *  it indicates that the ball is targeting another kart atm. */
    int16_t      m_delete_ticks;

    /** This variable counts how often a ball tunneled (in consecutive
     *  frames). If a ball tunnels a certain number of times, it is
     *  considered stuck and will be removed. */
    uint8_t      m_tunnel_count;

    /** This flag is set if the target is within the fast ping distance. It
     *  will cause the rubber ball to decrese the jump height and intervall. */
    bool         m_fast_ping;

    /** Once the ball is close enough, it will aim for the kart. If the
     *  kart should be able to then increase the distance to the ball,
     *  the ball will be removed and the kart escapes. This boolean is
     *  used to keep track of the state of this ball. */
    bool         m_aiming_at_target;

    /** A 'ping' sound effect to be played when the ball hits the ground. */
    SFXBase     *m_ping_sfx;

    bool m_restoring_state;

    void         computeTarget();
    void         updateDistanceToTarget();
    unsigned int getSuccessorToHitTarget(unsigned int node_index,
                                         float *f=NULL);
    void         getNextControlPoint();
    float        updateHeight();
    void         interpolate(Vec3 *next_xyz, int ticks);
    void         moveTowardsTarget(Vec3 *next_xyz, int ticks);
    void         updateWeightedSpeed(int ticks);
    void         initializeControlPoints(const Vec3 &xyz);
    float        getTunnelHeight(const Vec3 &next_xyz, 
                                     const float vertical_offset) const;
    bool         checkTunneling();
    void removePingSFX();

public:
                 RubberBall  (AbstractKart* kart);
    virtual     ~RubberBall();
    static  void init(const XMLNode &node, scene::IMesh *rubberball);
    virtual bool updateAndDelete(int ticks) OVERRIDE;
    virtual bool hit(AbstractKart* kart, PhysicalObject* obj=NULL) OVERRIDE;
    virtual void setAnimation(AbstractKartAnimation *animation) OVERRIDE;
    // ------------------------------------------------------------------------
    /** This object does not create an explosion, all affects on
     *  karts are handled by this hit() function. */
    //virtual HitEffect *getHitEffect() const {return NULL; }
    // ------------------------------------------------------------------------
    virtual BareNetworkString* saveState(std::vector<std::string>* ru)
        OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void restoreState(BareNetworkString *buffer, int count) OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void onFireFlyable() OVERRIDE;

};   // RubberBall

#endif
