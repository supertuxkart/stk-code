//  $Id: challenge_data.hpp 2173 2008-07-21 01:55:41Z auria $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008 Joerg Henrichs
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

#ifndef HEADER_CHALLENGE_DATA_H
#define HEADER_CHALLENGE_DATA_H

#include <string>
#include <vector>
#include <stdio.h>

#include "challenges/challenge.hpp"
#include "race_manager.hpp"

class ChallengeData : public Challenge 
{
private:
    RaceManager::MajorRaceModeType m_major;
    RaceManager::MinorRaceModeType m_minor;
    RaceManager::Difficulty        m_difficulty;
    int                            m_num_laps;
    int                            m_position;
    int                            m_num_karts;
    float                          m_time;
    std::string                    m_gp_id;
    std::string                    m_track_name;
    int                            m_energy;
    std::vector<std::string>       m_depends_on;
    std::vector<UnlockableFeature> m_unlock;

    std::string                    m_filename;
    void getUnlocks(const lisp::Lisp *lisp, const char* type, REWARD_TYPE reward);
    void error(const char *id);

public:
                 ChallengeData(const std::string& filename);
    void         setRace() const;
    virtual bool raceFinished();
    virtual bool grandPrixFinished();
};   // ChallengeData

#endif
