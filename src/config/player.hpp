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
#include "utils/no_copy.hpp"

/**
  * \brief Class for managing player profiles (name, control configuration, etc.)
  * A list of all possible players is stored as PlayerProfiles in the user config.
  * A list of currently playing players will be stored somewhere else (FIXME : complete comment)
  * \ingroup config
  */
class PlayerProfile : public NoCopy
{
protected:
    
    /** For saving to config file. */
    GroupUserConfigParam  m_player_group;
    
    StringUserConfigParam m_name;
    
    BoolUserConfigParam   m_is_guest_account;
        
#ifdef DEBUG
    unsigned int m_magic_number;
#endif
    
public:
    
    IntUserConfigParam    m_use_frequency;

    /**
      * Constructor to create a new player that didn't exist before
      */
    PlayerProfile(const char* name) : m_player_group("Player", "Represents one human player"),
                                      m_name(name, "name", &m_player_group), //, m_last_kart_id(-1)
                                      m_is_guest_account(false, "guest", &m_player_group),
                                      m_use_frequency(0, "use_frequency", &m_player_group)
    {
#ifdef DEBUG
        m_magic_number = 0xABCD1234;
#endif
    }
    
    /**
      * Constructor to deserialize a player that was saved to a XML file
      * (...UserConfigParam classes will automagically take care of serializing all
      * create players to the user's config file)
      */
    PlayerProfile(const XMLNode* node) : m_player_group("Player", "Represents one human player"),
                                         m_name("-", "name", &m_player_group), //, m_last_kart_id(-1)
                                         m_is_guest_account(false, "guest", &m_player_group),
                                         m_use_frequency(0, "use_frequency", &m_player_group)
    {
        //m_player_group.findYourDataInAChildOf(node);
        m_name.findYourDataInAnAttributeOf(node);
        m_is_guest_account.findYourDataInAnAttributeOf(node);
        m_use_frequency.findYourDataInAnAttributeOf(node);
        
#ifdef DEBUG
        m_magic_number = 0xABCD1234;
#endif
    }
    
    
    ~PlayerProfile()
    {
#ifdef DEBUG
        m_magic_number = 0xDEADBEEF;
#endif
    }
    
    void setName(const std::string &name_){ assert(m_magic_number == 0xABCD1234); m_name = name_;  }

    const char* getName() const { assert(m_magic_number == 0xABCD1234); return m_name.c_str();     }

    bool isGuestAccount() const { assert(m_magic_number == 0xABCD1234); return m_is_guest_account; }
    
    //int getLastKartId(){ return m_last_kart_id; }
    //void setLastKartId(int newLastKartId){ m_last_kart_id = newLastKartId; }
};

#endif

/*EOF*/
