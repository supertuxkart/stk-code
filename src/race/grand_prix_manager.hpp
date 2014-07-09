//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008-2013 Joerg Henrichs
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

#ifndef HEADER_GRAND_PRIX_MANAGER_HPP
#define HEADER_GRAND_PRIX_MANAGER_HPP

#include <vector>
#include <string>

#include "race/grand_prix_data.hpp"

/**
  * \ingroup race
  */
class GrandPrixManager
{
private:
    static const char* SUFFIX;

    std::vector<GrandPrixData*> m_gp_data;

    /** Load all the grands prix from the 3 directories known */
    void loadFiles();
    /** Load all the grands prix in one directory */
    void loadDir(const std::string& dir);
    /** Load a grand prix and add it to m_gp_data*/
    void load(const std::string &filename);

    /** Generates a new unique indentifier for a user defined grand prix */
    std::string generateId();

    bool existsName(const irr::core::stringw& name) const;

public:
    /** saved here by a random GP dialog to avoid dangling pinters or
     * memory leaks */
    GrandPrixData* m_random_gp;

                   GrandPrixManager();
                  ~GrandPrixManager();
    void           reload();
    GrandPrixData* getGrandPrix(const int i) const { return m_gp_data[i];     }
    GrandPrixData* getGrandPrix(const std::string& s) const;
    unsigned int   getNumberOfGrandPrix()    const { return m_gp_data.size(); }
    void           checkConsistency();

    // Methods for the gp editor
    GrandPrixData* editGrandPrix(const std::string& s) const;
    GrandPrixData* createNewGP(const irr::core::stringw& newName);
    GrandPrixData* copy(const std::string& id,
                        const irr::core::stringw& newName);
    void           remove(const std::string& id);
};   // GrandPrixManager

extern GrandPrixManager *grand_prix_manager;

#endif
