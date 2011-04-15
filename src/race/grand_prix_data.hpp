//  $Id$
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
    irr::core::stringw m_name;         //!< The name of the grand prix - might be translated!

    //irr::core::stringw m_description;  //!< Description for this GP
    
    std::string m_id;                  //!< Internal name of the grand prix, not translated
    std::string m_filename;            //!< Original filename, only for error handling needed
    
    /**
      * The ident of the tracks in this grand prix in their right order, ident
      * means the filename of the .track file without .track extension (ie. 'volcano')
      */
    std::vector<std::string> m_tracks;
    
    /** The number of laps that each track should be raced, in the right order */
    std::vector<int> m_laps;

public:

    /** Load the GrandPrixData from the given filename */
#if defined(WIN32) || defined(_WIN32)
#pragma warning(disable:4290)
#endif
                       GrandPrixData  (const std::string filename) throw(std::logic_error);
                       GrandPrixData  ()       {}; // empty for initialising
    
    /** @return the (potentially translated) user-visible name of the Grand Prix */
    const irr::core::stringw getName ()         const { return translations->w_gettext(m_name.c_str()); }
    
    /** @return the (potentially translated) user-visible description of the Grand Prix */
    //const irr::core::stringw& getDescription () const { return m_description;   }

    /** @return the internale name identifier of the Grand Prix (not translated) */
    const std::string& getId          ()        const { return m_id;            }
    
    const std::string& getFilename    ()        const { return m_filename;      }
    const std::string& getTrack(size_t track_index) const { assert(track_index >= 0); assert(track_index < m_tracks.size()); 
                                                       return m_tracks[track_index]; }
    const std::vector<std::string>& getTracks()  const {return m_tracks;        }
    const std::vector<int>&         getLaps()    const {return m_laps;          }
    size_t             getTrackCount()           const {return m_tracks.size(); }
    const int&         getLaps(size_t lap_index) const {assert(lap_index < m_tracks.size()); 
                                                        return m_laps[lap_index];}
    bool               checkConsistency(bool chatty=true) const;
}
;   // GrandPrixData

#endif

/* EOF */
