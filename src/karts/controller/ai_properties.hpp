//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012-2015  Joerg Henrichs
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

#ifndef HEADER_AI_PROPERTIES_HPP
#define HEADER_AI_PROPERTIES_HPP

#include "race/race_manager.hpp"
#include "utils/interpolation_array.hpp"

#include <string>
#include <vector>

class XMLNode;


/** A simple class that stores all AI related properties. It acts as
 *  interface between kart_properties and AI (to avoid either passing
 *  very many individual variables, or making KartProperties a dependency
 *  of the AI). The AIs are friends of this class and so have access to
 *  its protected members.
 * \ingroup karts
 */

class AIProperties
{
public:
    //LEAK_CHECK();
protected:
    // Give them access to the members
    friend class AIBaseController;
    friend class AIBaseLapController;
    friend class SkiddingAI;
    friend class ArenaAI;
    friend class TestAI;

    /** Used to check that all values are defined in the xml file. */
    static float UNDEFINED;

    /** Maximum direction change when trying to collect an item. Items that
     *  are more than this away, will not even be considered. */
    float m_max_item_angle;

    /** Maximum direction change when trying to collect an item while being on
     *  high-speed (i.e. skidding bonus, nitro, ...). Items that
     *  are more than this away, will not even be considered. */
    float m_max_item_angle_high_speed;

    /** If a good item and a bad item are closer than this distance, a good
     *  item will be avoided (in order to avoid the bad item). If the items
     *  are further apart, it is assumed that there is enough time to
     *  change steering direction.
     */
    float m_bad_item_closeness_2;

    /** Time for  AI karts to reach full steer angle (used to reduce shaking
     *   of karts). */
    float m_time_full_steer;

    /** Minimum length of a straight in order to activate a zipper. */
    float m_straight_length_for_zipper;

    /** The array of (distance, skid_probability) points. */
    InterpolationArray m_skid_probability;

    /** To cap maximum speed if the kart is ahead of the player. */
    InterpolationArray m_speed_cap;

    /** To determine the probability of selecting an item. */
    InterpolationArray m_collect_item_probability;

    /** Distance at which a detected projectile triggers a shield. */
    float m_shield_incoming_radius;

    /** Probability of a false start. Note that Nolok in boss battle will never
     *  have a false start. */
    float m_false_start_probability;

    /** Minimum start delay. */
    float m_min_start_delay;

    /** Maximum start delay. */
    float m_max_start_delay;

    /** True if the AI should avtively try to make use of slipstream. */
    bool m_make_use_of_slipstream;

    /** Used for low level AI to not give them a slipstream bonus.
     *  Otherwise they tend to build 'trains' (AIs driving close behing
     *  each other and get slipstream bonus). Only for making the easy
     *  AI really easy. */
    bool m_disable_slipstream_usage;

    /** Actively collect and avoid items. */
    bool m_collect_avoid_items;

    /** If the AI should actively try to pass on a bomb. */
    bool m_handle_bomb;

    /** True if items should be used better (i.e. non random). */
    bool m_item_usage_non_random;

    /** How the AI uses nitro. */
    enum {NITRO_NONE, NITRO_SOME, NITRO_ALL} m_nitro_usage;

    /** TODO: ONLY USE FOR OLD SKIDDING! CAN BE REMOVED once the new skidding
     *  works as expected.
     *  The minimum steering angle at which the AI adds skidding. Lower values
     *  tend to improve the line the AI is driving. This is used to adjust for
     *  different AI levels. */
    float    m_skidding_threshold;

    /** An identifier like 'easy', 'medium' or 'hard' for this data set. */
    std::string m_ident;

public:

         AIProperties(RaceManager::Difficulty difficulty);
    void load(const XMLNode *skid_node);
    void checkAllSet(const std::string &filename) const;
    // ------------------------------------------------------------------------
    /** Returns the skidding probability dependent on the specified distance
     *  to the first player kart. */
    float getSkiddingProbability(float distance) const
    {
        return m_skid_probability.get(distance);
    }   // getSkiddingProbability
    // ------------------------------------------------------------------------
    /** Returns the fraction of maximum speed the AI should drive at, depending
     *  on the distance from the player. */
    float getSpeedCap(float distance) const
    {
        return m_speed_cap.get(distance);
    }   // getSpeedCap
    // ------------------------------------------------------------------------
    /** Returns the probability to collect an item depending on the distance
     *  to the first player kart. */
    float getItemCollectProbability(float distance) const
    {
        return m_collect_item_probability.get(distance);
    }   // getItemcollectProbability
    // ------------------------------------------------------------------------
    /** Returns true if this kart should not even get a slipstream bonus. */
    bool disableSlipstreamUsage() const { return m_disable_slipstream_usage; }
};   // AIProperties


#endif

/* EOF */

