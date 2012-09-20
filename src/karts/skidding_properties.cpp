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

#include "karts/skidding_properties.hpp"

#include "io/xml_node.hpp"

#include <string.h>

float SkiddingProperties::UNDEFINED = -99.9f;

SkiddingProperties::SkiddingProperties()
{
    m_skid_increase           = UNDEFINED;
    m_skid_decrease           = UNDEFINED;
    m_skid_max                = UNDEFINED;
    m_time_till_max_skid      = UNDEFINED;
    m_skid_visual             = UNDEFINED;
    m_skid_visual_time        = UNDEFINED;
    m_skid_revert_visual_time = UNDEFINED;
    m_post_skid_rotate_factor = UNDEFINED;
    m_skid_reduce_turn_min    = UNDEFINED;
    m_skid_reduce_turn_max    = UNDEFINED;
    m_physical_jump_time      = UNDEFINED;
    m_graphical_jump_time     = UNDEFINED;
    m_has_skidmarks           = true;

    m_skid_bonus_time.clear();
    m_skid_bonus_speed.clear();
    m_skid_time_till_bonus.clear();
    m_skid_bonus_force.clear();
}   // SkiddingProperties

// ----------------------------------------------------------------------------
void SkiddingProperties::load(const XMLNode *skid_node)
{
    skid_node->get("increase",               &m_skid_increase          );
    skid_node->get("decrease",               &m_skid_decrease          );
    skid_node->get("max",                    &m_skid_max               );
    skid_node->get("time-till-max",          &m_time_till_max_skid     );
    skid_node->get("visual",                 &m_skid_visual            );
    skid_node->get("visual-time",            &m_skid_visual_time       );
    skid_node->get("revert-visual-time",     &m_skid_revert_visual_time);
    skid_node->get("post-skid-rotate-factor",&m_post_skid_rotate_factor);
    skid_node->get("reduce-turn-min",        &m_skid_reduce_turn_min   );
    skid_node->get("reduce-turn-max",        &m_skid_reduce_turn_max   );
    skid_node->get("enable",                 &m_has_skidmarks          );
    skid_node->get("bonus-time",             &m_skid_bonus_time        );
    skid_node->get("bonus-speed",            &m_skid_bonus_speed       );
    skid_node->get("time-till-bonus",        &m_skid_time_till_bonus   );
    skid_node->get("bonus-force",            &m_skid_bonus_force       );
    skid_node->get("physical-jump-time",     &m_physical_jump_time     );
    skid_node->get("graphical-jump-time",    &m_graphical_jump_time    );
}   // load

// ----------------------------------------------------------------------------
void SkiddingProperties::checkAllSet(const std::string &filename) const
{
#define CHECK_NEG(  a,strA) if(a<=UNDEFINED) {                         \
        fprintf(stderr,"Missing default value for '%s' in '%s'.\n",    \
                strA,filename.c_str());exit(-1);                       \
    }
    CHECK_NEG(m_skid_increase,           "skid increase"                 );
    CHECK_NEG(m_skid_decrease,           "skid decrease"                 );
    CHECK_NEG(m_skid_max,                "skid max"                      );
    CHECK_NEG(m_time_till_max_skid,      "skid time-till-max"            );
    CHECK_NEG(m_skid_visual,             "skid visual"                   );
    CHECK_NEG(m_skid_visual_time,        "skid visual-time"              );
    CHECK_NEG(m_skid_revert_visual_time, "skid revert-visual-time"       );
    CHECK_NEG(m_post_skid_rotate_factor, "skid post-skid-rotate-factor"  );
    CHECK_NEG(m_skid_reduce_turn_min,    "skid reduce-turn-min"          );
    CHECK_NEG(m_skid_reduce_turn_max,    "skid reduce-turn-max"          );
    CHECK_NEG(m_physical_jump_time,      "skid physical-jump-time"       );
    CHECK_NEG(m_graphical_jump_time,     "skid graphical-jump-time"      );

    if(m_skid_time_till_bonus.size()==0)
        fprintf(stderr, "Warning: no skid time declared, can be ignored.\n");
    if(m_skid_time_till_bonus.size()!=m_skid_bonus_speed.size())
    {
        fprintf(stderr, "Warning: skid time-till-bonus and bonus-speed\n");
        fprintf(stderr, "         must have same number of elements.\n");
        exit(-1);
    }
    if(m_skid_time_till_bonus.size()!=m_skid_bonus_time.size())
    {
        fprintf(stderr, "Warning: skid time-till-bonus and bonus-time must\n");
        fprintf(stderr, "         have same number of elements.\n");
        exit(-1);
    }
    if(m_skid_time_till_bonus.size()!=m_skid_bonus_force.size())
    {
        fprintf(stderr, "Warning: skid time-till-bonus and bonus-force must\n");
        fprintf(stderr, "         have same number of elements.\n");
        exit(-1);
    }
    for(unsigned int i=0; i<m_skid_time_till_bonus.size()-1; i++)
    {
        if(m_skid_time_till_bonus[i]>=m_skid_time_till_bonus[i+1])
        {
            fprintf(stderr, "Warning: skid time-till-bonus not sorted.\n");
            exit(-1);
        }
    }   // for i

}   // check

// ----------------------------------------------------------------------------
void SkiddingProperties::copyFrom(const SkiddingProperties *destination)
{
    *this = *destination;
}   // copyFrom

// ----------------------------------------------------------------------------


/* EOF */
