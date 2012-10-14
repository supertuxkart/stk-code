//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012  Joerg Henrichs
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
#include "utils/string_utils.hpp"

float AIProperties::UNDEFINED = -99.9f;

/** Constructor. Sets all properties to the special UNDEFINED value.
 */
AIProperties::AIProperties()
{
    m_max_item_angle             = UNDEFINED;
    m_max_item_angle_high_speed  = UNDEFINED;
    m_time_full_steer            = UNDEFINED;
    m_bad_item_closeness_2       = UNDEFINED;
    m_straight_length_for_zipper = UNDEFINED;
}   // AIProperties

// ----------------------------------------------------------------------------
/** Loads the AI properties from an XML file.
 *  \param ai_node The XML node containing all AI properties.
 */
void AIProperties::load(const XMLNode *ai_node)
{
    ai_node->get("max-item-angle",            &m_max_item_angle            );
    ai_node->get("max-item-angle-high-speed", &m_max_item_angle_high_speed );
    ai_node->get("time-full-steer",           &m_time_full_steer           );
    ai_node->get("bad-item-closeness",        &m_bad_item_closeness_2      );
    ai_node->get("straight-length-for-zipper",&m_straight_length_for_zipper);

    std::string s;
    ai_node->get("rb-skid-probability", &s);
    std::vector<std::string> pairs = StringUtils::split(s, ' ');
    for(unsigned int i=0; i<pairs.size(); i++)
    {
        std::vector<std::string> pair = StringUtils::split(pairs[i],':');
        if(pair.size()!=2)
        {
            printf("Incorrect pair '%s' in rd-skid-probability.\n",
                   pairs[i].c_str());
            printf("Must be distance:probability.\n");
            exit(-1);
        }
        float distance;
        if(!StringUtils::fromString(pair[0], distance))
        {
            printf("Incorrect distance in pair '%s'.\n", pairs[i].c_str());
            exit(-1);
        }
        float p;
        if(!StringUtils::fromString(pair[1], p))
        {
            printf(
                "Incorrect probability in pair '%s' in rb-skid-probability.\n",
                  pair[1].c_str());
            exit(-1);
        }
        m_skid_distances.push_back(distance);
        m_skid_probabilities.push_back(p);
    }   // for i

    if(m_skid_distances.size()==0)
    {
        printf("No skid probability defined.\n");
        exit(-1);
    }
    for(unsigned int i=0; i<m_skid_distances.size()-1; i++)
    {
        if(m_skid_distances[i]>=m_skid_distances[i+1])
        {
            printf("Skid distances must be sorted.\n");
            exit(-1);
        }
    }
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
#define CHECK_NEG(  a,str_a) if(a<=UNDEFINED) {                        \
        fprintf(stderr,"Missing default value for '%s' in '%s'.\n",    \
                str_a,filename.c_str());exit(-1);                      \
    }
    CHECK_NEG(m_max_item_angle,            "max-item-angle"            );
    CHECK_NEG(m_max_item_angle_high_speed, "max-item-angle-high-speed" );
    CHECK_NEG(m_time_full_steer,           "time-full-steer"           );
    CHECK_NEG(m_bad_item_closeness_2,      "bad-item-closeness"        );
    CHECK_NEG(m_straight_length_for_zipper,"straight-length-for-zipper");

}   // checkAllSet

// ----------------------------------------------------------------------------
void AIProperties::copyFrom(const AIProperties *destination)
{
    *this = *destination;
}   // copyFrom

// ----------------------------------------------------------------------------
float AIProperties::getSkiddingProbability(float distance) const
{
    if(m_skid_distances.size()==1)
        return m_skid_probabilities[0];

    if(distance<m_skid_distances[0])
        return m_skid_probabilities[0];
    if(distance>m_skid_distances[m_skid_distances.size()-1])
        return m_skid_probabilities[m_skid_probabilities.size()-1];

    // Now distance must be between two of the distances in the
    // sorted m_skid_distances array
    for(unsigned int i=1; i<m_skid_distances.size(); i++)
    {
        if(distance >m_skid_distances[i]) continue;
        float f = m_skid_probabilities[i-1] + 
                  (m_skid_probabilities[i]-m_skid_probabilities[i-1]) *
                  (distance - m_skid_distances[i-1])/
                  (m_skid_distances[i]-m_skid_distances[i-1]);
        return f;
    }
    assert(false);
    return 0.0f;
}   // getSkiddingProbability

/* EOF */
