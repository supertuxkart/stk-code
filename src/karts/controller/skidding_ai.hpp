
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

#ifndef HEADER_SKIDDING_AI_HPP
#define HEADER_SKIDDING_AI__HPP

#include "karts/controller/ai_base_controller.hpp"
#include "race/race_manager.hpp"
#include "tracks/graph_node.hpp"
#include "utils/random_generator.hpp"

class LinearWorld;
class QuadGraph;
class ShowCurve;
class Track;

namespace irr
{
    namespace scene
    {
        class ISceneNode;
    }
}

/**
  * \ingroup controller
  */
class SkiddingAI : public AIBaseController
{
private:

    class CrashTypes
    {
        public:

        bool m_road; //true if we are going to 'crash' with the bounds of the road
        int m_kart; //-1 if no crash, pos numbers are the kart it crashes with
        CrashTypes() : m_road(false), m_kart(-1) {};
        void clear() {m_road = false; m_kart = -1;}
    } m_crashes;

    RaceManager::AISuperPower m_superpower;    

    /*General purpose variables*/

    /** Pointer to the closest kart ahead of this kart. NULL if this
     *  kart is first. */
    AbstractKart *m_kart_ahead;

    /** Distance to the kart ahead. */
    float m_distance_ahead;

    /** Pointer to the closest kart behind this kart. NULL if this kart
     *  is last. */
    AbstractKart *m_kart_behind;

    /** Distance to the kard behind. */
    float m_distance_behind;

    /** The actual start delay used. */
    float m_start_delay; 
  
    /** Time an item has been collected and not used. */
    float m_time_since_last_shot;
  
    float m_time_since_stuck;

    /** Direction of crash: -1 = left, 1 = right, 0 = no crash. */
    int m_start_kart_crash_direction; 

    /** The direction of the track where the kart is on atm. */
    GraphNode::DirectionType m_current_track_direction;

    /** The radius of the curve the kart is currently driving. Undefined
     *  when being on a straigt section. */
    float m_current_curve_radius;

    /** Stores the center of the curve (if the kart is in a curve, 
     *  otherwise undefined). */
    Vec3  m_curve_center;

    /** The index of the last node with the same direction as the current
     *  node the kart is on (i.e. if kart is in a left turn, this will be
     *  the last node that is still turning left). */
    unsigned int m_last_direction_node;

    /** If set an item that the AI should aim for. */
    const Item *m_item_to_collect;

    /** True if items to avoid are close by. Used to avoid using zippers
     *  (which would make it more difficult to avoid items). */
    bool m_avoid_item_close;

    /** Distance to the player, used for rubber-banding. */
    float m_distance_to_player;

    /** A random number generator to decide if the AI should skid or not. */
    RandomGenerator m_random_skid;

    /** This implements a simple finite state machine: it starts in
     *  NOT_YET. The first time the AI decides to skid, the state is changed
     *  randomly (dependeng on skid probability) to N_SKID or SKID.
     *  As long as the AI keeps on deciding the skid, the state remains
     *  unchanged (so no new random decision is made) till it decides
     *  not to skid. In which case the state is set to NOT_YET again.
     *  This guarantees that for each 'skidable' section of the track
     *  the random decision is only done once. */
    enum {SKID_PROBAB_NOT_YET, SKID_PROBAB_NO_SKID, SKID_PROBAB_SKID}
          m_skid_probability_state;

    /** Which of the three Point Selection Algorithms (i.e.
     *  findNoNCrashingPoint* functions) to use:
     *  the default (which is actually slightly buggy, but so far best one 
     *              (after handling of 90 degree turns was added)
     *  the fixed one (which fixes one problem of the buggy algorithm),
     *  the new algorithm (see findNonCrashingPoint* for details).
     *  So far the default one has by far the best performance. */
    enum {PSA_DEFAULT, PSA_FIXED, PSA_NEW}
          m_point_selection_algorithm;

#ifdef DEBUG
    /** For skidding debugging: shows the estimated turn shape. */
    ShowCurve **m_curve;

    /** For debugging purpose: a sphere indicating where the AI 
     *  is targeting at. */
    irr::scene::ISceneNode *m_debug_sphere[4];

    /** For item debugging: set to the item that is selected to 
     *  be collected. */
    irr::scene::ISceneNode *m_item_sphere;
#endif


    /*Functions called directly from update(). They all represent an action
     *that can be done, and end up setting their respective m_controls
     *variable, except handle_race_start() that isn't associated with any
     *specific action (more like, associated with inaction).
     */
    void  handleRaceStart();
    void  handleAcceleration(const float dt);
    void  handleSteering(float dt);
    void  handleItems(const float dt);
    void  handleRescue(const float dt);
    void  handleBraking();
    void  handleNitroAndZipper();
    void  computeNearestKarts();
    void  handleItemCollectionAndAvoidance(Vec3 *aim_point, 
                                           int last_node);
    bool  handleSelectedItem(float kart_aim_angle, Vec3 *aim_point);
    bool  steerToAvoid(const std::vector<const Item *> &items_to_avoid,
                       const core::line2df &line_to_target,
                       Vec3 *aim_point);
    bool  hitBadItemWhenAimAt(const Item *item, 
                              const std::vector<const Item *> &items_to_avoid);
    void  evaluateItems(const Item *item, float kart_aim_angle, 
                        std::vector<const Item *> *items_to_avoid,
                        std::vector<const Item *> *items_to_collect);

    void  checkCrashes(const Vec3& pos);
    void  findNonCrashingPointFixed(Vec3 *result, int *last_node);
    void  findNonCrashingPointNew(Vec3 *result, int *last_node);
    void  findNonCrashingPointDefault(Vec3 *result, int *last_node);

    void  determineTrackDirection();
    void  determineTurnRadius(const Vec3 &start,
                              const Vec3 &start_direction,
                              const Vec3 &end,
                              Vec3 *center,
                              float *radius);
    virtual bool doSkid(float steer_fraction);
    virtual void setSteering(float angle, float dt);
    void handleCurve();

protected:
    virtual unsigned int getNextSector(unsigned int index);

public:
                 SkiddingAI(AbstractKart *kart);
                ~SkiddingAI();
    virtual void update      (float delta) ;
    virtual void reset       ();
    virtual const irr::core::stringw& getNamePostfix() const;
};

#endif

/* EOF */
