
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006-2007 Eduardo Hernandez Munoz
//  Copyright (C) 2010      Joerg Henrichs
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

#ifndef HEADER_BATTLE_AI_HPP
#define HEADER_BATTLE_AI_HPP

#include "karts/controller/ai_base_controller.hpp"
#include "race/race_manager.hpp"
#include "tracks/battle_graph.hpp"
#include "utils/random_generator.hpp"

class AIProperties;
class ThreeStrikesBattle;
class BattleGraph;
class Track;
class Vec3;
class Item;

namespace irr
{
    namespace scene { class ISceneNode; }
    namespace video { class ITexture; }
}

class BattleAI : public AIBaseController
{

private:

    /** Holds the position info of targets. */
    struct posData {bool behind; float angle; float distance;};

    /** Holds the current position of the AI on the battle graph. Sets to
     *  BattleGraph::UNKNOWN_POLY if the location is unknown. This variable is
     *  updated in ThreeStrikesBattle::updateKartNodes(). */
    int m_current_node;

    int m_closest_kart_node;
    Vec3 m_closest_kart_point;

    posData m_closest_kart_pos_data;
    posData m_cur_kart_pos_data;

   /** Indicates that the kart is currently stuck, and m_time_since_stuck is
     * counting down. */
    bool m_is_stuck;

    /** Indicates that the kart need a uturn to reach a node behind, and
     *  m_time_since_uturn is counting down. */
    bool m_is_uturn;

    const Item *m_item_to_collect;

    /** Holds the unique node ai has walked through, useful to tell if AI is
     *  stuck by determine the size of this set. */
    std::set <int> m_on_node;

    /** Holds the corner points computed using the funnel algorithm that the AI
     *  will eventaully move through. See stringPull(). */
    std::vector<Vec3> m_path_corners;

    /** Holds the set of portals that the kart will cross when moving through
     *  polygon channel. See findPortals(). */
    std::vector<std::pair<Vec3,Vec3> > m_portals;

    /** The node(poly) at which the target point lies in. */
    int m_target_node;

    /** The target point. */
    Vec3 m_target_point;

    /** Time an item has been collected and not used. */
    float m_time_since_last_shot;

    /** This is a timer that counts down when the kart is reversing to get unstuck. */
    float m_time_since_reversing;

    /** This is a timer that counts down when the kart is starting to get stuck. */
    float m_time_since_stuck;

    /** This is a timer that counts down when the kart is doing u-turn. */
    float m_time_since_uturn;

    void  checkIfStuck(const float dt);
    void  checkPosition(const Vec3 &, posData*);
    float determineTurnRadius(std::vector<Vec3>& points);
    void  findClosestKart();
    void  findPortals(int start, int end);
    void  findTarget();
    void  handleAcceleration(const float dt);
    void  handleBraking();
    void  handleItems(const float dt);
    void  handleItemCollection(Vec3*, int*);
    void  handleSteering(const float dt);
    void  handleUTurn(const float dt);
    void  stringPull(const Vec3&, const Vec3&);

protected:

    /** Keep a pointer to world. */
    ThreeStrikesBattle *m_world;

//#ifdef AI_DEBUG
    /** For debugging purpose: a sphere indicating where the AI
     *  is targeting at. */
    irr::scene::ISceneNode *m_debug_sphere;
//#endif

public:
                 BattleAI(AbstractKart *kart,
                          StateManager::ActivePlayer *player=NULL);
                ~BattleAI();
    unsigned int getCurrentNode() const { return m_current_node; }
    void         setCurrentNode(int i)  { m_current_node = i;    }
    virtual void update      (float delta);
    virtual void reset       ();

    virtual void crashed(const AbstractKart *k) {};
    virtual void handleZipper(bool play_sound) {};
    virtual void finishedRace(float time) {};
    virtual void collectedItem(const Item &item, int add_info=-1,
                               float previous_energy=0) {};
    virtual void setPosition(int p) {};
    virtual bool isNetworkController() const { return false;     }
    virtual bool isPlayerController() const { return false;      }
    virtual void action(PlayerAction action, int value) {};
    virtual void skidBonusTriggered() {};
    virtual bool disableSlipstreamBonus() const {return 0;       }
    virtual void newLap(int lap) {};
};

#endif
