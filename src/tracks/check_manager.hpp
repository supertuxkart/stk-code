//  $Id$
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

#ifndef HEADER_CHECK_MANAGER_HPP
#define HEADER_CHECK_MANAGER_HPP

#include <vector>
#include <string>

class XMLNode;
class CheckStructure;
class Track;

/**
  * \brief Controls all checks structures of a track.
  * \ingroup tracks
  */
class CheckManager
{
private:
    std::vector<CheckStructure*> m_all_checks;
public:
         CheckManager(const XMLNode &node, Track *track);
        ~CheckManager();
    void update(float dt);
    void reset(const Track &track);
    
    int getCheckStructureCount() const { return m_all_checks.size(); }
    
    /** Returns the nth. check structure. */
    CheckStructure *getCheckStructure(unsigned int n) 
    {
        if (n >= m_all_checks.size()) return NULL;
        return m_all_checks[n];
    }
};   // CheckManager

#endif

