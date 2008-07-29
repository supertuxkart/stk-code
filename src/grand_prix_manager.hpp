//  $Id: grand_prix_manager.hpp 2173 2008-07-21 01:55:41Z auria $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008 Joerg Henrichs
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

#ifndef HEADER_GRAND_PRIX_MANAGER_H
#define HEADER_GRAND_PRIX_MANAGER_H

#include <vector>
#include <string>
#include "cup_data.hpp"

class GrandPrixManager
{
private:
    std::vector<CupData*> m_cup_data;
public:
                   GrandPrixManager();
                  ~GrandPrixManager();
    void           load(const std::string &filename);
    const CupData* getCup(int i)          const { return m_cup_data[i];     }
    const CupData* getCup(const std::string& s) const;
    unsigned int   getNumberOfGrandPrix() const { return m_cup_data.size(); }
    void           checkConsistency();

};   // GrandPrixManager

extern GrandPrixManager *grand_prix_manager;
#endif
