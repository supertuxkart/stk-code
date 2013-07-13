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

#ifndef HEADER_CURRENT_ONLINE_USER_HPP
#define HEADER_CURRENT_ONLINE_USER_HPP

#include "online/online_user.hpp"
#include <string>
#include <irrString.h>
#include "utils/types.hpp"
#include "online/server.hpp"



// ============================================================================

/**
  * \brief Class that represents an online registered user
  * \ingroup online
  */
class CurrentOnlineUser : public OnlineUser
{
    private:

    protected:
        std::string m_token;
        bool m_is_signed_in;
        bool m_is_guest;
        CurrentOnlineUser();

    public:
        // singleton
        static CurrentOnlineUser* get();
        static void deallocate();

        bool trySavedSession();
        // Login
        bool signIn(    const irr::core::stringw &username,
                        const irr::core::stringw &password,
                        bool save_session,
                        irr::core::stringw &info);
        // Register
        bool signUp(    const irr::core::stringw &username,
                        const irr::core::stringw &password,
                        const irr::core::stringw &password_ver,
                        const irr::core::stringw &email,
                        bool terms,
                        irr::core::stringw &info);
        // Logout - Best to be followed by CurrentOnlineUser::deallocate
        bool signOut(   irr::core::stringw &info);

        bool createServer(  const irr::core::stringw &name,
                            int max_players,
                            irr::core::stringw &info);

        bool requestJoin(   uint32_t server_id,
                            irr::core::stringw &info);

        /** Returns the username if signed in. */
        irr::core::stringw getUserName() const;
        bool isSignedIn(){ return m_is_signed_in; }
        bool isGuest(){ return m_is_guest; }

};   // class CurrentOnlineUser

#endif

/*EOF*/
