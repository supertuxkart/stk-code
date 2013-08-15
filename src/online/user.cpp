//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 Glenn De Jonghe
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


#include "online/user.hpp"

#include <sstream>
#include <stdlib.h>

using namespace Online;

namespace Online{
    // ============================================================================
    User::User  (   const irr::core::stringw & username,
                    const uint32_t           & userid
                )
    {
        setUserName(username);
        setUserID(userid);
    }
    // ============================================================================
    User::User(uint32_t id)
    {
        setUserName("");
        setUserID(id);
    }

    // ============================================================================
    User::User  (   const XMLNode * xml)
    {
        irr::core::stringw username("");
        xml->get("user_name", &username);
        setUserName(username);
        uint32_t id;
        xml->get("id", &id);
        setUserID(id);
    }
} // namespace Online
