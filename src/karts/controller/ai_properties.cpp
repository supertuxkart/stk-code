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

#include "karts/controller/ai_properties.hpp"

#include "io/xml_node.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"

float AIProperties::UNDEFINED = -99.9f;

/** Constructor. Sets all properties to the special UNDEFINED value.
 */
AIProperties::AIProperties(RaceManager::Difficulty difficulty)
{
    m_ident = RaceManager::get()->getDifficultyAsString(difficulty);

    m_max_item_angle             = UNDEFINED;
    m_max_item_angle_high_speed  = UNDEFINED;
    m_time_full_steer            = UNDEFINED;
    m_bad_item_closeness_2       = UNDEFINED;
    m_straight_length_for_zipper = UNDEFINED;
    m_skidding_threshold         = UNDEFINED;
    m_min_start_delay            = UNDEFINED;
    m_max_start_delay            = UNDEFINED;
    m_shield_incoming_radius     = UNDEFINED;
    m_false_start_probability    = UNDEFINED;
    m_make_use_of_slipstream     = false;
    m_collect_avoid_items        = false;
    m_handle_bomb                = false;
    m_item_usage_skill           = 0;
    m_disable_slipstream_usage   = false;
    m_nitro_usage                = 0;

}   // AIProperties

// ----------------------------------------------------------------------------
/** Loads the AI properties from an XML file.
 *  \param ai_node The XML node containing all AI properties.
 */
void AIProperties::load(const XMLNode *ai_node)
{
    ai_node->get("use-slipstream",            &m_make_use_of_slipstream    );
    ai_node->get("disable-slipstream-usage",  &m_disable_slipstream_usage  );
    ai_node->get("max-item-angle",            &m_max_item_angle            );
    ai_node->get("max-item-angle-high-speed", &m_max_item_angle_high_speed );
    ai_node->get("time-full-steer",           &m_time_full_steer           );
    ai_node->get("bad-item-closeness",        &m_bad_item_closeness_2      );
    ai_node->get("collect-item-probability",  &m_collect_item_probability  );
    ai_node->get("straight-length-for-zipper",&m_straight_length_for_zipper);
    ai_node->get("rb-skid-probability",       &m_skid_probability          );
    ai_node->get("first-speed-cap",           &m_first_speed_cap           );
    ai_node->get("last-speed-cap",            &m_last_speed_cap            );
    ai_node->get("item-skill",                &m_item_usage_skill          );
    ai_node->get("collect-avoid-items",       &m_collect_avoid_items       );
    ai_node->get("handle-bomb",               &m_handle_bomb               );
    ai_node->get("skidding-threshold",        &m_skidding_threshold        );
    ai_node->get("shield-incoming-radius",    &m_shield_incoming_radius    );
    ai_node->get("false-start-probability",   &m_false_start_probability   );
    ai_node->get("min-start-delay",           &m_min_start_delay           );
    ai_node->get("max-start-delay",           &m_max_start_delay           );
    ai_node->get("nitro-usage",               &m_nitro_usage               );

    // We actually need the square of the distance later
    m_bad_item_closeness_2 *= m_bad_item_closeness_2;

}   // load

// ----------------------------------------------------------------------------
/** Check if all AI properties are defined, and aborts if some are missing.
 *  \param filename Name of the file from which the properties are read.
 *         Only used for error messages.
 */
void AIProperties::checkAllSet(const std::string &filename) const
{
#define CHECK_NEG(  a,str_a) if(a<=UNDEFINED) {                     \
        Log::fatal("AIProperties","Missing default value for"       \
                    " '%s' in '%s' 'for AI '%s'.",                \
                    str_a, filename.c_str(), m_ident.c_str());      \
    }
    CHECK_NEG(m_max_item_angle,            "max-item-angle"            );
    CHECK_NEG(m_max_item_angle_high_speed, "max-item-angle-high-speed" );
    CHECK_NEG(m_time_full_steer,           "time-full-steer"           );
    CHECK_NEG(m_bad_item_closeness_2,      "bad-item-closeness"        );
    CHECK_NEG(m_straight_length_for_zipper,"straight-length-for-zipper");
    CHECK_NEG(m_skidding_threshold,        "skidding-threshold"        );
    CHECK_NEG(m_shield_incoming_radius,    "shield-incoming-radius"    );
    CHECK_NEG(m_false_start_probability,   "false-start-probability"   );
    CHECK_NEG(m_min_start_delay,           "min-start-delay"           );
    CHECK_NEG(m_max_start_delay,           "max-start-delay"           );

    if(m_skid_probability.size()==0)
    {
        Log::fatal("AIProperties", "No skid probability defined.");
    }

    if(m_first_speed_cap.size()==0 || m_last_speed_cap.size()==0)
    {
        Log::fatal("AIProperties", "No speed cap defined.");
    }

    if(m_collect_item_probability.size()==0)
    {
        Log::fatal("AIProperties", "No collect-item-probability defined.");
    }

}   // checkAllSet


// ------------------------------------------------------------------------
/** Returns the fraction of maximum speed the AI should drive at, depending
 *  on the distance from the player. */
float AIProperties::getSpeedCap(float distance, int ai_position, int num_ai) const
{
    float wfirst = 1.0f;
    if (num_ai >= 2)
        wfirst = 1.0f - ( (float) (ai_position-1) / (float) (num_ai-1) );

    return wfirst      * m_first_speed_cap.get(distance)
           +(1-wfirst) * m_last_speed_cap.get(distance);
}   // getSpeedCap

/* EOF */
