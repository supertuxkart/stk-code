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

#ifndef HEADER_AI_PROPERTIES_HPP
#define HEADER_AI_PROPERTIES_HPP

#include <string>

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
    friend class SkiddingAI;

    /** Used to check that all values are defined in the xml file. */
    static float UNDEFINED;

    /** Maximum direction change when trying to collect an item. Items that
     *  are more than this away, will not even be considered. */
    float m_max_item_angle;

    /** Maximum direction change when trying to collect an item while being on
     *  high-speed (i.e. skidding bonus, nitro, ...). Items that
     *  are more than this away, will not even be considered. */
    float m_max_item_angle_high_speed;

    /** Time for  AI karts to reach full steer angle (used to reduce shaking
     *   of karts). */
    float m_time_full_steer;

public:

         AIProperties();
    void load(const XMLNode *skid_node);
    void copyFrom(const AIProperties *destination);
    void checkAllSet(const std::string &filename) const;
};   // AIProperties


#endif

/* EOF */

