//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 Glenn De Jonghe
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

#include "config/player_profile.hpp"
#include "utils/synchronised.hpp"
#include "utils/types.hpp"

#include <irrString.h>

#include <string>
#include <assert.h>

class PlayerManager;

namespace Online
{
    class OnlineProfile;

    /**
      * \brief Class that represents an online registered user
      * \ingroup online
      */
    class OnlinePlayerProfile : public PlayerProfile
    {
    private:
        std::string                 m_token;
        OnlineProfile              *m_profile;

        /** The state of the player (logged in, logging in, ...) */
        PlayerProfile::OnlineState  m_online_state;

        virtual void signIn(bool success, const XMLNode * input);
        virtual void signOut(bool success, const XMLNode * input,
                            const irr::core::stringw &info);
        virtual uint32_t getOnlineId() const;
        virtual void setUserDetails(std::shared_ptr<HTTPRequest> request,
                                    const std::string &action,
                                    const std::string &url_path = "") const;

        virtual void requestPoll() const;

        // ----------------------------------------------------------------
        /** Returns if this user is logged in. */
        virtual bool isLoggedIn() const
        {
            return m_online_state == PlayerProfile::OS_SIGNED_IN;
        }   // isLoggedIn

        // ----------------------------------------------------------------
        /** The online state of the player (i.e. logged out, logging in,
         *  logged in, ...). */
        PlayerProfile::OnlineState getOnlineState() const
        {
            return m_online_state;
        }   // getOnlineState

        // ----------------------------------------------------------------
        /** Returns the session token of the signed in user. */
        const std::string& getToken() const { return m_token; }
        virtual void requestSavedSession();
        virtual void requestSignOut();
        virtual void requestSignIn(const irr::core::stringw &username,
                                   const irr::core::stringw &password);

    public:
        OnlinePlayerProfile(const XMLNode *player);
        OnlinePlayerProfile(const core::stringw &name, bool is_guest = false);
        virtual ~OnlinePlayerProfile() {}
        // ----------------------------------------------------------------
        /** Returns a pointer to the profile associated with the current user. */
        OnlineProfile* getProfile() const { return m_profile; }
        // ----------------------------------------------------------------
    }; // class OnlinePlayerProfile
} // namespace Online
#endif // HEADER_CURRENT_ONLINE_USER_HPP
