//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014 Joerg Henrichs
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

#ifndef HEADER_GRAPHICS_RESTrICTIONS_HPP
#define HEADER_GRAPHICS_RESTrICTIONS_HPP

/**
 * \defgroup graphics
 */

#include <string>
#include <vector>

namespace GraphicsRestrictions
{
    /** Which graphical restrictions can be defined. Note that
     *  the variable m_names_of_restrictions in the cpp file contains the
     *  string representation used in the XML files. Any change to this
     *  type declaration needs a change in that variable as well. */
    enum GraphicsRestrictionsType 
    {
        GR_BUFFER_STORAGE,
        GR_GLOBAL_ILLUMINATION,
        GR_COUNT  /** MUST be last entry. */
    } ;

    void init(const std::string &driver_version,
              const std::string &card_name       );
    bool isDisabled(GraphicsRestrictionsType type);

    void unitTesting();
};   // HardwareStats

#endif
