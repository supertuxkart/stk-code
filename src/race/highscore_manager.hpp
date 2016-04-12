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

#ifndef HEADER_HIGHSCORE_MANAGER_HPP
#define HEADER_HIGHSCORE_MANAGER_HPP

#include <string>
#include <vector>
#include <map>

#include "race/highscores.hpp"

/**
  * This class reads and writes the 'highscores.xml' file, and also takes
  * care of dealing with new records. One 'HighscoreEntry' object is created
  * for each highscore entry.
  * \ingroup race
  */
class HighscoreManager
{
public:
private:
    static const unsigned int CURRENT_HSCORE_FILE_VERSION;
    typedef std::vector<Highscores*> type_all_scores;
    type_all_scores m_all_scores;

    std::string m_filename;
    bool        m_can_write;

    void loadHighscores();
    void setFilename();

public:
                HighscoreManager();
               ~HighscoreManager();
    void        saveHighscores();
    Highscores *getHighscores(const Highscores::HighscoreType &highscore_type,
                              int num_karts,
                              const RaceManager::Difficulty difficulty,
                              const std::string &trackName,
                              const int number_of_laps,
                              const bool reverse);
};   // HighscoreManager

extern HighscoreManager* highscore_manager;
#endif
