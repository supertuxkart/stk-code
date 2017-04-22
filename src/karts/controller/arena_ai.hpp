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
}

/** A base class for AI that use navmesh to work.
 * \ingroup controller
 */
class ArenaAI : public AIBaseController
{
protected:
    /** Pointer to the \ref ArenaGraph. */
    ArenaGraph* m_graph;

    /** Pointer to the closest kart around this kart. */
    AbstractKart *m_closest_kart;

    /** The \ref ArenaNode at which the closest kart located on. */
    int m_closest_kart_node;

    /** The closest kart location. */
    Vec3 m_closest_kart_point;

    /** Holds the current difficulty. */
    RaceManager::Difficulty m_cur_difficulty;

    /** For debugging purpose: a sphere indicating where the AI
     *  is targeting at. */
    irr::scene::ISceneNode *m_debug_sphere;

    /** For debugging purpose: a sphere indicating where the first
     *  turning corner is located. */
    irr::scene::ISceneNode *m_debug_sphere_next;

    /** The \ref ArenaNode at which the target point located on. */
    int m_target_node;

    /** The coordinates of target point. */
    Vec3 m_target_point;

    /** True if AI can skid, currently only do when close to target, see
     *  \ref doSkiddingTest(). */
    bool m_mini_skid;

    // ------------------------------------------------------------------------
    void          tryCollectItem(Vec3* aim_point, int* target_node) const;
    // ------------------------------------------------------------------------
    /** Find the closest kart around this AI, implemented by sub-class.
     *  \param consider_difficulty If take current difficulty into account.
     *  \param find_sta If find \ref SpareTireAI only. */
    virtual void  findClosestKart(bool consider_difficulty, bool find_sta) = 0;

private:
    /** Local coordinates of current target point. */
    Vec3 m_target_point_lc;

    /** Save the last target point before reversing, so AI will end reversing
     *  until facing in front of it. */
    Vec3 m_reverse_point;

    /** Indicates that the kart is currently stuck, and m_time_since_reversing
     *  is counting down. */
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

    /** Used to determine braking and nitro usage. */
    float m_turn_radius;

    /** Used to determine if skidding can be done. */
    float m_steering_angle;

    /** The point in front of the AI which distance is \ref m_kart_length, used
     *  to compensate the time difference between steering when finding next
     *  node. */
    Vec3 m_current_forward_point;

    /** The \ref ArenaNode at which the forward point located on. */
    int m_current_forward_node;

    void          configSpeed();
    // ------------------------------------------------------------------------
    void          configSteering();
    // ------------------------------------------------------------------------
    void          checkIfStuck(const float dt);
    // ------------------------------------------------------------------------
    void          determinePath(int forward, std::vector<int>* path);
    // ------------------------------------------------------------------------
    void          doSkiddingTest();
    // ------------------------------------------------------------------------
    void          doUTurn(const float dt);
    // ------------------------------------------------------------------------
    bool          gettingUnstuck(const float dt);
    // ------------------------------------------------------------------------
    bool          updateAimingPosition(Vec3* target_point);
    // ------------------------------------------------------------------------
    void          useItems(const float dt);
    // ------------------------------------------------------------------------
    virtual bool  canSkid(float steer_fraction) OVERRIDE
                                                        { return m_mini_skid; }
    // ------------------------------------------------------------------------
    /** Find a suitable target for this frame, implemented by sub-class. */
    virtual void  findTarget() = 0;
    // ------------------------------------------------------------------------
    /** If true, AI will always try to brake for this frame. */
    virtual bool  forceBraking()                              { return false; }
    // ------------------------------------------------------------------------
    /** Return the current \ref ArenaNode the AI located on. */
    virtual int   getCurrentNode() const = 0;
    // ------------------------------------------------------------------------
    /** Return the distance based on graph distance matrix to any kart.
     *  \param kart \ref AbstractKart to check. */
    virtual float getKartDistance(const AbstractKart* kart) const = 0;
    // ------------------------------------------------------------------------
    /** If true, AI will drive directly to target without path finding. */
    virtual bool  ignorePathFinding()                         { return false; }
    // ------------------------------------------------------------------------
    /** If true, AI will stop moving. */
    virtual bool  isWaiting() const = 0;
    // ------------------------------------------------------------------------
    /** If true, AI stays on the \ref ArenaNode correctly, otherwise
     *  \ref RescueAnimation will be done after sometime. */
    virtual bool  isKartOnRoad() const = 0;
    // ------------------------------------------------------------------------
    /** Overridden if any action is needed to be done when AI stopped
     *  moving or changed driving direction. */
    virtual void  resetAfterStop() {}

public:
                 ArenaAI(AbstractKart *kart);
    // ------------------------------------------------------------------------
    virtual     ~ArenaAI() {}
    // ------------------------------------------------------------------------
    virtual void update (float delta) OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void reset  () OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void newLap (int lap) OVERRIDE {}

};

#endif
