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

#ifndef HEADER_SOCCER_AI_HPP
#define HEADER_SOCCER_AI_HPP

#include "karts/controller/arena_ai.hpp"

#include "LinearMath/btTransform.h"

#undef BALL_AIM_DEBUG
#ifdef BALL_AIM_DEBUG
#include "graphics/irr_driver.hpp"
#endif

class SoccerWorld;

/** The actual soccer AI.
 * \ingroup controller
 */
class SoccerAI : public ArenaAI
{
private:

#ifdef BALL_AIM_DEBUG
    irr::scene::ISceneNode *m_red_sphere;
    irr::scene::ISceneNode *m_blue_sphere;
#endif

    /** Keep a pointer to world. */
    SoccerWorld *m_world;

    /** Save the team this AI belongs to. */
    KartTeam m_cur_team;

    /** Save the opposite team of this AI team. */
    KartTeam m_opp_team;

    /** Define which way to handle to ball, either steer with it,
     *  or overtake it (Defense). */
    bool m_overtake_ball;

    /** True if \ref forceBraking() is needed to be called. */
    bool m_force_brake;

    /** True if AI should steer with the ball. */
    bool m_chasing_ball;

    /** The front point of kart with the same rotation of center mass, used
     *  to determine point for aiming with ball */
    btTransform m_front_transform;

    // ------------------------------------------------------------------------
    Vec3  determineBallAimingPosition();
    // ------------------------------------------------------------------------
    bool  determineOvertakePosition(const Vec3& ball_lc, const Vec3& aim_lc,
                                    Vec3* overtake_lc);
    // ------------------------------------------------------------------------
    bool  isOvertakable(const Vec3& ball_lc);
    // ------------------------------------------------------------------------
    float rotateSlope(float old_slope, bool rotate_up);
    // ------------------------------------------------------------------------
    virtual bool  canSkid(float steer_fraction) OVERRIDE
                { return m_mini_skid && !(m_overtake_ball || m_chasing_ball); }
    // ------------------------------------------------------------------------
    virtual void  findClosestKart(bool consider_difficulty,
                                  bool find_sta) OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void  findTarget() OVERRIDE;
    // ------------------------------------------------------------------------
    virtual bool  forceBraking() OVERRIDE             { return m_force_brake; }
    // ------------------------------------------------------------------------
    virtual int   getCurrentNode() const OVERRIDE;
    // ------------------------------------------------------------------------
    virtual float getKartDistance(const AbstractKart* kart) const OVERRIDE;
    // ------------------------------------------------------------------------
    virtual bool  ignorePathFinding() OVERRIDE
                                 { return  m_overtake_ball || m_chasing_ball; }
    // ------------------------------------------------------------------------
    virtual bool  isKartOnRoad() const OVERRIDE;
    // ------------------------------------------------------------------------
    virtual bool  isWaiting() const OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void  resetAfterStop() OVERRIDE        { m_overtake_ball = false; }

public:
                 SoccerAI(AbstractKart *kart);
                ~SoccerAI();
    virtual void update (int ticks) OVERRIDE;
    virtual void reset() OVERRIDE;

};

#endif
