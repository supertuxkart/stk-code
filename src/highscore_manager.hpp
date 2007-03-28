//  $Id: highscores.hpp 921 2007-02-28 05:43:34Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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

#ifndef HEADER_HIGHSCORE_MANAGER_H
#define HEADER_HIGHSCORE_MANAGER_H

#include <string>
#include <vector>
#include <map>

#include "highscores.hpp"
#include "lisp/lisp.hpp"
#include "race_setup.hpp"
class HighscoreManager
{
public:
private:
    static const int HIGHSCORE_LEN = 3;       // It's a top 3 list

    typedef std::vector<Highscores*> type_all_scores;
    type_all_scores m_allScores;
    
    std::string filename;
private:
    void Load();
    void Save();
    void SetFilename();

public:
    HighscoreManager();
    ~HighscoreManager();
    Highscores *addResult(const Highscores::HighscoreType highscore_type, 
                          const int num_karts, const RaceDifficulty difficulty, 
                          const std::string track, const std::string kart_name, 
                          const std::string name, const float time);
};   // HighscoreManager

extern HighscoreManager* highscore_manager;
#endif
