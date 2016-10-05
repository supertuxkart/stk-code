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

#ifndef HEADER_ARENA_AI_HPP
#define HEADER_ARENA_AI_HPP

#include "karts/controller/ai_base_controller.hpp"
#include "race/race_manager.hpp"

#undef AI_DEBUG
#ifdef AI_DEBUG
#include "graphics/irr_driver.hpp"
#endif

class ArenaGraph;

namespace irr
{
    namespace scene { class ISceneNode; }
    namespace video { class ITexture; }
}

/** A base class for AI that use navmesh to work.
 * \ingroup controller
 */
class ArenaAI : public AIBaseController
{
protected:
    ArenaGraph* m_graph;

    /** Pointer to the closest kart around this kart. */
    AbstractKart *m_closest_kart;

    int m_closest_kart_node;
    Vec3 m_closest_kart_point;

    /** Holds the current difficulty. */
    RaceManager::Difficulty m_cur_difficulty;

    /** For debugging purpose: a sphere indicating where the AI
     *  is targeting at. */
    irr::scene::ISceneNode *m_debug_sphere;
    irr::scene::ISceneNode *m_debug_sphere_next;

    /** The node(quad) at which the target point lies in. */
    int m_target_node;

    /** The target point. */
    Vec3 m_target_point;

    bool m_mini_skid;

    void  collectItemInArena(Vec3*, int*) const;
private:
    /** Used by handleArenaUTurn, it tells whether to do left or right
     *  turning when steering is overridden. */
    bool m_adjusting_side;

    Vec3 m_target_point_lc;

   /** Indicates that the kart is currently stuck, and m_time_since_reversing is
     * counting down. */
    bool m_is_stuck;

    /** Indicates that the kart need a uturn to reach a node behind, and
     *  m_time_since_uturn is counting down. */
    bool m_is_uturn;

    /** Holds the unique node ai has driven through, useful to tell if AI is
     *  stuck by determine the size of this set. */
    std::set<int> m_on_node;

    /** Time an item has been collected and not used. */
    float m_time_since_last_shot;

    /** This is a timer that counts down when the kart is reversing to get unstuck. */
    float m_time_since_reversing;

    /** This is a timer that counts down when the kart is starting to drive. */
    float m_time_since_driving;

    /** This is a timer that counts down when the kart is doing u-turn. */
    float m_time_since_uturn;

    /** This is a timer that counts when the kart start going off road. */
    float m_time_since_off_road;

    float m_turn_radius;

    float m_steering_angle;

    Vec3 m_current_forward_point;

    int m_current_forward_node;

    void          configSpeed();
    void          configSteering();
    void          checkIfStuck(const float dt);
    void          determinePath(int forward, std::vector<int>* path);
    void          doSkiddingTest();
    void          doUTurn(const float dt);
    bool          gettingUnstuck(const float dt);
    bool          updateAimingPosition(Vec3* target_point);
    void          useItems(const float dt);
    virtual bool  canSkid(float steer_fraction) OVERRIDE
                                     { return m_mini_skid; }
    virtual void  findClosestKart(bool use_difficulty) = 0;
    virtual void  findTarget() = 0;
    virtual bool  forceBraking() { return false; }
    virtual int   getCurrentNode() const = 0;
    virtual float getKartDistance(const AbstractKart* kart) const = 0;
    virtual bool  ignorePathFinding() { return false; }
    virtual bool  isWaiting() const = 0;
    virtual bool  isKartOnRoad() const = 0;
    virtual void  resetAfterStop() {};
public:
                 ArenaAI(AbstractKart *kart);
    virtual     ~ArenaAI() {};
    virtual void update (float delta) OVERRIDE;
    virtual void reset  () OVERRIDE;
    virtual void newLap (int lap) OVERRIDE {}
};

#endif
