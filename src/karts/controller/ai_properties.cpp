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

float AIProperties::UNDEFINED = -99.9f;

/** Constructor. Sets all properties to the special UNDEFINED value.
 */
AIProperties::AIProperties()
{
    m_max_item_angle            = UNDEFINED;
    m_max_item_angle_high_speed = UNDEFINED;
    m_time_full_steer           = UNDEFINED;
}   // AIProperties

// ----------------------------------------------------------------------------
/** Loads the AI properties from an XML file.
 *  \param ai_node The XML node containing all AI properties.
 */
void AIProperties::load(const XMLNode *ai_node)
{
    ai_node->get("max-item-angle",            &m_max_item_angle           );
    ai_node->get("max-item-angle-high-speed", &m_max_item_angle_high_speed);
    ai_node->get("time-full-steer",           &m_time_full_steer          );
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
    CHECK_NEG(m_max_item_angle,            "max-item-angle"           );
    CHECK_NEG(m_max_item_angle_high_speed, "max-item-angle-high-speed");
    CHECK_NEG(m_time_full_steer,           "time-full-steer"          );

}   // checkAllSet

// ----------------------------------------------------------------------------
void AIProperties::copyFrom(const AIProperties *destination)
{
    *this = *destination;
}   // copyFrom

// ----------------------------------------------------------------------------


/* EOF */
