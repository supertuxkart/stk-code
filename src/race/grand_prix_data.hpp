//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2015  Ingo Ruhnke <grumbel@gmx.de>
//  Copyright (C) 2006-2015  Joerg Henrichs
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

#include <irrString.h>
#include <string>
#include <vector>

#include "utils/translation.hpp"

using irr::core::stringw;

class Track;

/** Simple class that hold the data relevant to a 'grand_prix', aka. a number
  * of races that has to be completed one after the other
  * \ingroup race
  */
class GrandPrixData
{
public:
    /** Used to classify GPs into groups */
    enum GPGroupType
    {
        GP_NONE = 0,      ///< No group
        GP_STANDARD,      ///< Standard GP, included with the game
        GP_USER_DEFINED,  ///< Created by the user
        GP_ADDONS,        ///< Add-on GP
        GP_GROUP_COUNT    ///< Number of groups
    };   // GPGroupType

private:
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

    /** The number of laps that each track will be raced, in the right order */
    std::vector<int> m_laps;

    /** Whether the track in question should be done in reverse mode */
    std::vector<bool> m_reversed;

    /** Wether the user can edit this grand prix or not */
    bool m_editable;

    /** Group to which this GP belongs. */
    enum GPGroupType m_group;

    /** In the last GP Fort Magma can not be used untill the final challenge.
     *  In order to provide still 5 tracks/GP, the last GP is only using 4
     *  tracks in story mode, but once nolok is unlocked Fort Magma becomes
     *  available (i.e. if nolok is unlocked, Fort Magma is available, otherwise
     *  not).
     */
    bool isTrackAvailable(const std::string &id, bool includeLocked) const;

public:
    /** Used to define the reverse setting when creating a random GP:
     *  No reverse, all reverse (if available on the track), random
     *  selection. */
    enum GPReverseType
    {
        GP_NO_REVERSE = 0,
        GP_ALL_REVERSE = 1,
        GP_RANDOM_REVERSE = 2,
        GP_DEFAULT_REVERSE = 3
    };   // GPReverseType

private:
    GPReverseType m_reverse_type;

public:
#if (defined(WIN32) || defined(_WIN32)) && !defined(__MINGW32__)
#  pragma warning(disable:4290)
#endif
    /** Load the GrandPrixData from the given filename */
    GrandPrixData(const std::string& filename, enum GPGroupType group);

    /** Needed for simple creation of an instance of GrandPrixData */
    GrandPrixData() {};

    void changeTrackNumber(const unsigned int number_of_tracks,
                           const std::string& track_group);
    void changeReverse(const GPReverseType use_reverse);

    void createRandomGP(const unsigned int number_of_tracks,
                        const std::string& track_group,
                        const GPReverseType use_reverse,
                        bool new_tracks=false);

    // Methods for the GP editor
    void setId(const std::string& id);
    void setName(const irr::core::stringw& name);
    void setFilename(const std::string& filename);
    void setEditable(const bool editable);
    void setGroup(const enum GPGroupType group);
    /** Load the grand prix from the file set by the constructor or the grand
     * prix editor */
    void reload();
    bool writeToFile();

    bool                     checkConsistency(bool log_error=true) const;
    std::vector<std::string> getTrackNames(const bool includeLocked=false) const;
    std::vector<int>         getLaps(const bool includeLocked=false) const;
    std::vector<bool>        getReverse(const bool includeLocked=false) const;
    bool                     isEditable() const;
    unsigned int             getNumberOfTracks(const bool includeLocked=false) const;
    const std::string&       getTrackId(const unsigned int track) const;
    irr::core::stringw       getTrackName(const unsigned int track) const;
    unsigned int             getLaps(const unsigned int track) const;
    bool                     getReverse(const unsigned int track) const;
    void                     moveUp(const unsigned int track);
    void                     moveDown(const unsigned int track);
    void                     addTrack(Track* track, unsigned int laps,
                                      bool reverse, int position=-1);
    void                     editTrack(unsigned int index, Track* track,
                                       unsigned int laps, bool reverse);
    void                     remove(const unsigned int track);

    // -------------------------------------------------------------------------
    /** @return the (potentially translated) user-visible name of the Grand
     *  Prix (apply fribidi as needed) */
    irr::core::stringw getName()      const { return m_editable ? m_name.c_str() : _LTR(m_name.c_str());   }

    // -------------------------------------------------------------------------
    /** @return the internal indentifier of the Grand Prix (not translated) */
    const std::string& getId()        const { return m_id;                     }

    // -------------------------------------------------------------------------
    /** Returns true if this GP is a random GP. */
    bool isRandomGP()                 const { return m_id==getRandomGPID();    }
    // -------------------------------------------------------------------------
    /** Returns the filename of the grand prix xml file. */
    const std::string& getFilename()  const { return m_filename;               }

    // ------------------------------------------------------------------------
    enum GPGroupType getGroup()       const { return m_group;                  }

    // -------------------------------------------------------------------------
    enum GPReverseType getReverseType()
                                      const { return m_reverse_type;           }
    static const char*        getRandomGPID()   { return "random";             }
    static irr::core::stringw getRandomGPName() { return _LTR("Random Grand Prix"); }
};   // GrandPrixData

#endif

/* EOF */
