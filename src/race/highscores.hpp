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
#include <string>
#include <vector>
#include <map>

#include "race/race_manager.hpp"

#include <irrString.h>

class XMLNode;
class UTFWriter;

/**
 *  Represents one highscore entry, i.e. the (atm up to three) highscores
 *  for a particular setting (track, #karts, difficulty etc).
 * \ingroup race
 */
class Highscores
{
public:
    typedef std::string HighscoreType;

private:
    enum {HIGHSCORE_LEN = 3};       // It's a top 3 list
    std::string         m_track;
    HighscoreType       m_highscore_type;
    int                 m_number_of_karts;
    int                 m_difficulty;
    int                 m_number_of_laps;
    bool                m_reverse;
    std::string         m_kart_name[HIGHSCORE_LEN];
    irr::core::stringw  m_name[HIGHSCORE_LEN];
    float               m_time[HIGHSCORE_LEN];
public:
    /** Creates a new entry
      */
    Highscores (const Highscores::HighscoreType &highscore_type,
                int num_karts, const RaceManager::Difficulty &difficulty,
                const std::string &trackName, const int number_of_laps,
                const bool reverse);
    /** Creates an entry from a file
     */
    Highscores (const XMLNode &node);

    void readEntry (const XMLNode &node);
    void writeEntry(UTFWriter &writer);
    int  matches   (const HighscoreType &highscore_type, int num_karts,
                    const RaceManager::Difficulty &difficulty,
                    const std::string &track, const int number_of_laps,
                    const bool reverse);
    int  addData   (const std::string& kart_name,
                    const irr::core::stringw& name, const float time);
    int  getNumberEntries() const;
    void getEntry  (int number, std::string &kart_name,
                    irr::core::stringw &name, float *const time) const;
};  // Highscores

#endif
