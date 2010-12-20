//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 Alejandro Santiago
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

#ifndef HEADER_TUTORIAL_DATA_HPP
#define HEADER_TUTORIAL_DATA_HPP

#include <string>
#include <vector>
#include <stdio.h>
#include <stdexcept>

#include "tutorial/tutorial.hpp"
#include "race/race_manager.hpp"

/**
  * \ingroup tutorial
  */
class TutorialData : public Tutorial
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
    std::string                    m_filename;

    void getUnlocks(const XMLNode *root, const std:: string type, REWARD_TYPE reward);
    void error(const char *id) const;

public:
#ifdef WIN32
                 TutorialData(const std::string& filename);
#else
                 TutorialData(const std::string& filename) throw(std::runtime_error);
#endif
    
    /** sets the right parameters in RaceManager to try this tutorial */
    void    setRace() const;

    void setEnergy(int m_energy);
    void setTime(float m_time);
    void setPosition(int position);
    void setDifficulty(std::string difficulty);
    void setTrack(std::string track_name);
    void setLaps(int num_laps);
    void setMajor(std::string major);
    void setMinor(std::string minor);
    void setNumKarts(int num_karts);

    virtual void check() const;
    virtual bool raceFinished();
    virtual bool grandPrixFinished();
    
};   // TutorialData

#endif   // HEADER_TUTORIAL_DATA_HPP
