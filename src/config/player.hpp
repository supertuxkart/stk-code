//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2013 SuperTuxKart-Team
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

#include "config/user_config.hpp"
#include "utils/no_copy.hpp"
#include "utils/types.hpp"

#include <irrString.h>
using namespace irr;

#include <string>

class UTFWriter;

/**
  * \brief Class for managing player profiles (name, control configuration, etc.)
  * A list of all possible players is stored as PlayerProfiles in the user config.
  * A list of currently playing players will be stored somewhere else (FIXME : complete comment)
  * \ingroup config
  */
class PlayerProfile : public NoCopy
{
protected:

    /** The name of the player (wide string, so it can be in native 
     *  language). */
    core::stringw m_name;

    /** True if this account is a guest account. */
    bool m_is_guest_account;

#ifdef DEBUG
    unsigned int m_magic_number;
#endif

    /** Counts how often this player was used. */
    unsigned int m_use_frequency;

    /** A unique number for this player, used to link it to challenges etc. */
    unsigned int m_unique_id;

public:

    PlayerProfile(const core::stringw& name, bool is_guest = false);

    PlayerProfile(const XMLNode* node);

    void save(UTFWriter &out);
    void incrementUseFrequency();

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



    // ------------------------------------------------------------------------
    /** Returns the unique id of this player. */
    unsigned int getUniqueID() const { return m_unique_id; }

};   // class PlayerProfile

#endif

/*EOF*/
