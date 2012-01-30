//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009  Joerg Henrichs
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

#include "tracks/check_manager.hpp"

#include <string>

#include "io/xml_node.hpp"
#include "tracks/ambient_light_sphere.hpp"
#include "tracks/check_lap.hpp"
#include "tracks/check_line.hpp"
#include "tracks/check_structure.hpp"
#include "tracks/track.hpp"

CheckManager::CheckManager(const XMLNode &node, Track *track)
{
    for(unsigned int i=0; i<node.getNumNodes(); i++)
    {
        const XMLNode *check_node = node.getNode(i);
        const std::string &type = check_node->getName();
        if(type=="check-line")
        {
            CheckLine *cl = new CheckLine(this, *check_node, i);
            m_all_checks.push_back(cl);
        }   // checkline
        else if(type=="check-lap")
        {
            m_all_checks.push_back(new CheckLap(this, *check_node, i));
        }
        else if(type=="check-sphere")
        {
            AmbientLightSphere *cs = new AmbientLightSphere(this, *check_node,
                                                            i);
            m_all_checks.push_back(cs);
        }   // checksphere
        else
            printf("Unknown check structure '%s' - ignored.\n", type.c_str());
    }   // for i<node.getNumNodes
}   // CheckManager

// ----------------------------------------------------------------------------
CheckManager::~CheckManager()
{
    for(unsigned int i=0; i<m_all_checks.size(); i++)
    {
        delete m_all_checks[i];
    }
}   // ~CheckManager

// ----------------------------------------------------------------------------

/** Reverse checks order. */
void CheckManager::reverse()
{
  // FIXME
}   // reverse

// ----------------------------------------------------------------------------

/** Resets all checks. */
void CheckManager::reset(const Track &track)
{
    std::vector<CheckStructure*>::iterator i;
    for(i=m_all_checks.begin(); i!=m_all_checks.end(); i++)
        (*i)->reset(track);
}   // reset

// ----------------------------------------------------------------------------
/** Updates all animations. Called one per time step.
 *  \param dt Time since last call.
 */
void CheckManager::update(float dt)
{
    std::vector<CheckStructure*>::iterator i;
    for(i=m_all_checks.begin(); i!=m_all_checks.end(); i++)
        (*i)->update(dt);
}   // update

