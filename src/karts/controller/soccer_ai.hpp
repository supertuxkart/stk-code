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

#undef BALL_AIM_DEBUG
#ifdef BALL_AIM_DEBUG
#include "graphics/irr_driver.hpp"
#endif

class SoccerWorld;
class Vec3;

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

    SoccerTeam m_cur_team;
    SoccerTeam m_opp_team;

    /** Define which way to handle to ball, either steer with it,
     *  or overtake it (Denfense).
     */
    bool m_overtake_ball;
    bool m_force_brake;
    bool m_steer_with_ball;

    Vec3 determineBallAimingPosition();
    bool determineOvertakePosition(const Vec3& ball_lc,
                                   const posData& ball_pos, Vec3* overtake_wc);

    virtual void findClosestKart(bool use_difficulty);
    virtual void findTarget();
    virtual void resetAfterStop() OVERRIDE    { m_overtake_ball = false; }
    virtual int  getCurrentNode() const;
    virtual bool isWaiting() const;
    virtual bool canSkid(float steer_fraction)           { return false; }
    virtual bool forceBraking() OVERRIDE         { return m_force_brake; }
    virtual bool directDrive() OVERRIDE
    {
        return m_avoid_eating_banana || m_overtake_ball || m_steer_with_ball;
    }

public:
                 SoccerAI(AbstractKart *kart);
                ~SoccerAI();
    virtual void update      (float delta);
    virtual void reset       ();
};

#endif
