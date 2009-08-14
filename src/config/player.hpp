// $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 SuperTuxKart-Team
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

#ifndef HEADER_PLAYER_HPP
#define HEADER_PLAYER_HPP

#include <string>
#include "config/user_config.hpp"
#include "input/input_device.hpp"

class InputDevice;
class PlayerKart;

/**
  * class for managing player profiles (name, control configuration, etc.)
  * A list of all possible players is stored as PlayerProfiles in the user config.
  * A list of currently playing players will be stored somewhere else (FIXME : complete comment)
  */
class PlayerProfile
{
private:
    
    // for saving to config file
    GroupUserConfigParam  m_player_group;
    
    StringUserConfigParam m_name;
    
    // int m_last_kart_id;

public:
    
    PlayerProfile(const char* name) : m_player_group("Player", "Represents one human player"),
                                      m_name(name, "name", &m_player_group) //, m_last_kart_id(-1)
    {
    }
    
    
    void setName(const std::string &name_){ m_name = name_;}

    const char* getName() { return m_name.c_str(); }

    //int getLastKartId(){ return m_last_kart_id; }
    //void setLastKartId(int newLastKartId){ m_last_kart_id = newLastKartId; }
};

/**
  * Represents a player that is currently playing. It helps manage associating with a
  * player profile and an input device (we're very flexible on this; ActivePlayer #1
  * can choose to e.g. use profile #5 and device #2)
  */
class ActivePlayer
{
    PlayerProfile* m_player;
    InputDevice* m_device;
public:
    
    ActivePlayer(PlayerProfile* player, InputDevice* device);
    ~ActivePlayer();
    
    PlayerProfile* getProfile();
    void setPlayerProfile(PlayerProfile* player);
    
    InputDevice* getDevice() const;
    void setDevice(InputDevice* device);
    
    PlayerKart* getKart();
};

#endif

/*EOF*/
