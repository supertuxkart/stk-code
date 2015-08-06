//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008-2015 Joerg Henrichs
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

#include "race/grand_prix_data.hpp"

#include <vector>
#include <string>
#include "utils/ptr_vector.hpp"

#include "irrlicht.h"
class GrandPrixData;

/**
  * \ingroup race
  */
class GrandPrixManager
{
private:
    static const char* SUFFIX;

    PtrVector<GrandPrixData> m_gp_data;

    /** Load all the grands prix from the 3 directories known */
    void loadFiles();
    /** Load all the grands prix in one directory */
    void loadDir(const std::string& dir, enum GrandPrixData::GPGroupType group);
    /** Load a grand prix and add it to m_gp_data*/
    void load(const std::string &filename, enum GrandPrixData::GPGroupType group);

    /** Generates a new unique indentifier for a user defined grand prix */
    std::string generateId();

public:
                   GrandPrixManager();
                  ~GrandPrixManager();
    void           reload();
    bool existsName(const irr::core::stringw& name) const;
    void           checkConsistency();

    // Methods for the gp editor
    GrandPrixData* editGrandPrix(const std::string& s);
    GrandPrixData* createNewGP(const irr::core::stringw& newName);
    GrandPrixData* copy(const std::string& id,
                        const irr::core::stringw& newName);
    void           remove(const std::string& id);
    // ------------------------------------------------------------------------
    /** Returns a pointer to the data for the specified GP.
    *  \param i Index of the GP. */
    const GrandPrixData* getGrandPrix(const std::string& s) const;
    // ------------------------------------------------------------------------
    /** Returns a pointer to the data for the specified GP.
     *  \param i Index of the GP. */
    const GrandPrixData* getGrandPrix(const int i) const { return m_gp_data.get(i);     }
    // ------------------------------------------------------------------------
    /** Returns the number of GPs. */
    unsigned int   getNumberOfGrandPrix()    const { return (int)m_gp_data.size(); }
};   // GrandPrixManager

extern GrandPrixManager *grand_prix_manager;

#endif
