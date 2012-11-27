//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2006 Ingo Ruhnke <grumbel@gmx.de>
//  Copyright (C) 2006      Joerg Henirchs, Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_GRAND_PRIX_DATA_HPP
#define HEADER_GRAND_PRIX_DATA_HPP

#include <string>
#include <vector>
#include <cassert>
#include <irrString.h>
#include <stdexcept>

#include "utils/translation.hpp"

/** Simple class that hold the data relevant to a 'grand_prix', aka. a number
  * of races that has to be completed one after the other
  * \ingroup race
  */
class GrandPrixData
{
    /** The name of the grand prix. */
    irr::core::stringw m_name;

    /** Internal name of the grand prix, not translated. */
    std::string m_id;

    /** Original filename, only for error handling needed. */
    std::string m_filename;
    
    /** The ident of the tracks in this grand prix in their right order, ident
     *  means the filename of the .track file without .track extension 
     *  (ie. 'volcano'). */
    std::vector<std::string> m_tracks;

    /** This is the list of actually available tracks. In the last GP Fort
     *  Magma can not be used untill the final challenge. In order to provide
     *  still 5 tracks/GP, the last GP is only using 4 tracks in story mode,
     *  but (once nolok is unlocked) Fort Magma is added after that. So this
     *  list is always re-evaluated depending on the state of Nolok (i.e. if
     *  nolok is unlocked, Fort Magma is available, otherwise not).
     *  Mark this member mutable so that getTrackNames can be const. */
    mutable std::vector<std::string> m_really_available_tracks;
    
    /** The number of laps that each track should be raced, in the right order */
    std::vector<int> m_laps;

    /** Whether the track in question should be done in reverse mode */
    std::vector<bool> m_reversed;

    bool isTrackAvailable(const std::string &id) const;
public:

    /** Load the GrandPrixData from the given filename */
#if (defined(WIN32) || defined(_WIN32)) && !defined(__MINGW32__)
#pragma warning(disable:4290)
#endif
                       GrandPrixData  (const std::string filename) throw(std::logic_error);
                       GrandPrixData  ()       {}; // empty for initialising
    
    bool checkConsistency(bool chatty=true) const;
    const std::vector<std::string>& getTrackNames() const;
    void getLaps(std::vector<int> *laps) const;
    void getReverse(std::vector<bool> *reverse) const;

    // ------------------------------------------------------------------------
    /** @return the (potentially translated) user-visible name of the Grand 
     *  Prix (apply fribidi as needed) */
    const irr::core::stringw getName() const { return _LTR(m_name.c_str());    }

    // ------------------------------------------------------------------------
    /** @return the internal name identifier of the Grand Prix (not translated) */
    const std::string& getId() const { return m_id;            }
    
    // ------------------------------------------------------------------------
    /** Returns the filename of the grand prix xml file. */
    const std::string& getFilename() const { return m_filename;  }

};   // GrandPrixData

#endif

/* EOF */
