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

#ifndef HEADER_HIGHSCORES_H
#define HEADER_HIGHSCORES_H
#include <string>
#include <vector>
#include <map>

#include "lisp/lisp.hpp"
#include "lisp/writer.hpp"
#include "race_manager.hpp"

class Highscores
{
public:
    enum HighscoreType {HST_UNDEFINED,
                        HST_TIMETRIAL_FASTEST_LAP,
                        HST_TIMETRIAL_OVERALL_TIME,
                        HST_RACE_FASTEST_LAP,
                        HST_RACE_OVERALL_TIME};
private:
    enum {HIGHSCORE_LEN = 3};       // It's a top 3 list
    std::string         m_track;
    HighscoreType       m_highscore_type;
    int                 m_number_of_karts;
    int                 m_difficulty;
    int                 m_number_of_laps;
    std::string         m_kart_name[HIGHSCORE_LEN];
    std::string         m_name[HIGHSCORE_LEN];
    float               m_time[HIGHSCORE_LEN];
public:
         Highscores();
    void Read      (const lisp::Lisp* const node);
    void Write     (lisp::Writer* writer);
    int  matches   (HighscoreType highscore_type, int num_karts,
                    const RaceManager::Difficulty difficulty, 
                    const std::string &track, const int number_of_laps);
    int  addData   (const HighscoreType highscore_type, const std::string kart_name,
                    const std::string name, const float time);
    int  getNumberEntries() const;
    void getEntry  (int number, std::string &kart_name,
                    std::string &name, float *const time) const;
};  // Highscores

#endif
