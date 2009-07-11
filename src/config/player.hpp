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

/**
  * class for managing player name and control configuration.
  * A list of all possible players is stored in the user config.
  * A list of currently playing players will be stored somewhere else (FIXME : complete comment)
  */
class Player
{
private:
    
    // for saving to config file
    GroupUserConfigParam  m_player_group;
    
    StringUserConfigParam m_name;
    
    int m_last_kart_id;

public:
    
    Player(const char* name) : m_player_group("Player", "Represents one human player"),
                                      m_name(name, "name", &m_player_group),
                                      m_last_kart_id(-1)
    {
    }
    
    
    void setName(const std::string &name_){m_name = name_;}

    const char* getName() { return m_name.c_str(); }

    int getLastKartId(){ return m_last_kart_id; }
    void setLastKartId(int newLastKartId){ m_last_kart_id = newLastKartId; }
};

/**
  * Represents a player that is currently playing.
  */
class ActivePlayer
{
    Player* m_player;
    InputDevice* m_device;
public:
    
    ActivePlayer(Player* player)
    {
        m_player = player;
        m_device = NULL;
    }
    ~ActivePlayer()
    {
        setDevice(NULL);
    }
    
    Player* getPlayer()
    {
        return m_player;
    }
    void setPlayer(Player* player)
    {
        m_player = player;
    }
    
    InputDevice* getDevice() const { return m_device; }
    void setDevice(InputDevice* device)
    {
        if (m_device != NULL) m_device->setPlayer(NULL);
        
        m_device = device;
        
        if(device != NULL)
            device->setPlayer(this);
    }
};

#endif

/*EOF*/
