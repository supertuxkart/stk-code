//  $Id: profile_world.hpp 3849 2009-08-13 11:12:26Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Joerg Henrichs
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

#ifndef HEADER_PROFILE_WORLD_HPP
#define HEADER_PROFILE_WORLD_HPP

#include "modes/standard_race.hpp"

class Kart;

class ProfileWorld : public StandardRace
{
private:
    /** Profiling modes. */
    enum        ProfileType {PROFILE_NONE, PROFILE_TIME, PROFILE_LAPS};
    /** If profiling is done, and if so, which mode. */
    static ProfileType m_profile_mode;
    /** In laps based profiling: number of laps to run. */
    static int   m_num_laps;
    /** In time based profiling only: time to run. */
    static float m_time;
    /** Return value of real time at start of race. */
    unsigned int m_start_time;
    /** Number of frames. For statistics only. */
    int          m_frame_count;

protected:

    virtual Kart *createKart(const std::string &kart_ident, int index, 
                             int local_player_id, int global_player_id,
                             const btTransform &init_pos);

public:
                          ProfileWorld();
    virtual              ~ProfileWorld() {};
    /** Returns identifier for this world. */
    virtual  std::string getInternalCode() const {return "PROFILE"; }
    virtual  void        update(float dt);
    virtual  bool        isRaceOver();
    virtual  void        enterRaceOverState(const bool delay=false);

    static   void setProfileModeTime(float time);
    static   void setProfileModeLaps(int laps);
    /** Returns true if profile mode was selected. */
    static   bool isProfileMode() {return m_profile_mode!=PROFILE_NONE; }
};

#endif
