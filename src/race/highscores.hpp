//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2015 Joerg Henrichs
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

#ifndef HEADER_HIGHSCORES_HPP
#define HEADER_HIGHSCORES_HPP
#include <array>
#include <string>
#include <vector>
#include <map>

#include "race/race_manager.hpp"

#include "irrString.h"

using namespace irr::core;

class XMLNode;
class UTFWriter;

/**
 *  Represents one highscore entry, i.e. the (atm up to five) highscores
 *  for a particular setting (track, #karts, difficulty etc).
 * \ingroup race
 */
class Highscores
{
public:
    /** Order of sort for Highscores */
    enum SortOrder
    {
        SO_DEFAULT,
        SO_TRACK = SO_DEFAULT,  // Sorted by internal track name
        SO_KART_NUM,            // Sorted by amount of karts used
        SO_DIFF,                // Sorted by difficulty level
        SO_LAPS,                // Sorted by number of laps
        SO_REV                  // Sorted by if using reverse mode
    };

    typedef std::string HighscoreType;

    enum {HIGHSCORE_LEN = 5};       // It's a top 5 list
    std::string         m_track;
    HighscoreType       m_highscore_type;
    int                 m_number_of_karts;
    int                 m_difficulty;
    int                 m_number_of_laps;
    bool                m_reverse;
    int                 m_gp_reverse_type;
    int                 m_gp_minor_mode;

private:
    std::array<std::string, HIGHSCORE_LEN> m_kart_name;
    std::array<stringw, HIGHSCORE_LEN>     m_name;
    std::array<float, HIGHSCORE_LEN>       m_time;

    static SortOrder m_sort_order;

    int findHighscorePosition(const std::string& kart_name, 
                              const core::stringw& name, const float time);

public:
    bool operator < (const Highscores& hi) const;

    static bool compare(const std::unique_ptr<Highscores>& a, const std::unique_ptr<Highscores>& b) { return (*a < *b); }
    /** Creates a new entry
      */
    Highscores (const Highscores::HighscoreType &highscore_type,
                int num_karts, const RaceManager::Difficulty &difficulty,
                const std::string &trackName, const int number_of_laps,
                const bool reverse);
    /** Constructor for grandprix highscores */
    Highscores (int num_karts, const RaceManager::Difficulty &difficulty,
                const std::string &trackName, const int target,
                const GrandPrixData::GPReverseType reverse_type, RaceManager::MinorRaceModeType minor_mode);
    /** Creates an entry from a file
     */
    Highscores (const XMLNode &node);
    // ------------------------------------------------------------------------
    void readEntry (const XMLNode &node);
    // ------------------------------------------------------------------------
    void writeEntry(UTFWriter &writer);
    // ------------------------------------------------------------------------
    int  matches   (const HighscoreType &highscore_type, int num_karts,
                    const RaceManager::Difficulty &difficulty,
                    const std::string &track, const int number_of_laps,
                    const bool reverse);
    // ------------------------------------------------------------------------
    /** matches method for grandprix highscores */
    int matches(int num_karts,
                const RaceManager::Difficulty &difficulty,
                const std::string &track, const int target,
                const GrandPrixData::GPReverseType reverse_type, RaceManager::MinorRaceModeType minor_mode);
    // ------------------------------------------------------------------------
    int  addData   (const std::string& kart_name,
                    const irr::core::stringw& name, const float time);
    int addGPData  (const std::string& kart_name,
                    const irr::core::stringw& name, std::string gp_name, const float time);
    // ------------------------------------------------------------------------
    int  getNumberEntries() const;
    // ------------------------------------------------------------------------
    void getEntry  (int number, std::string &kart_name,
                    irr::core::stringw &name, float *const time) const;
    // ------------------------------------------------------------------------
    static void setSortOrder(SortOrder so)  { m_sort_order = so; }
};  // Highscores

#endif
