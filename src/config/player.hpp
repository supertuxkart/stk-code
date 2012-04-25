//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 SuperTuxKart-Team
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
#include <irrString.h>
using namespace irr;

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
    
    WStringUserConfigParam m_name;
    
    BoolUserConfigParam   m_is_guest_account;
        
#ifdef DEBUG
    unsigned int m_magic_number;
#endif
    
    IntUserConfigParam    m_use_frequency;

    /** Profile names can change, so rather than try to make sure all renames are done everywhere,
     *  assign a unique ID to each profiler. Will save much headaches.
     */
    StringUserConfigParam m_unique_id;
    
    int64_t generateUniqueId(const char* playerName);
    
public:
    
    /**
      * Constructor to create a new player that didn't exist before
      */
    PlayerProfile(const core::stringw& name);
    
    /**
      * Constructor to deserialize a player that was saved to a XML file
      * (...UserConfigParam classes will automagically take care of serializing all
      * create players to the user's config file)
      */
    PlayerProfile(const XMLNode* node);
    
    
    ~PlayerProfile()
    {
        #ifdef DEBUG
        m_magic_number = 0xDEADBEEF;
        #endif
    }
    
    void setName(const core::stringw& name)
    { 
        #ifdef DEBUG
		assert(m_magic_number == 0xABCD1234);
        #endif
		m_name = name;
    }

    core::stringw getName() const
    { 
        #ifdef DEBUG
        assert(m_magic_number == 0xABCD1234);
        #endif
		return m_name.c_str();
    }

    bool isGuestAccount() const
    { 
        #ifdef DEBUG
		assert(m_magic_number == 0xABCD1234); 
        #endif
		return m_is_guest_account;
    }
    
    int getUseFrequency() const
    {
        if (m_is_guest_account) return -1;
        else return m_use_frequency;
    }
    
    
    void incrementUseFrequency();
 
    // please do NOT try to optimise this to return a reference, I don't know why,
    // maybe compiler bug, but hell breaks loose when you do that
    std::string getUniqueID() const
    {
        return m_unique_id;
    }
};

#endif

/*EOF*/
