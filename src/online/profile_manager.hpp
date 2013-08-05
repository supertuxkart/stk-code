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

#ifndef HEADER_ONLINE_PROFILE_MANAGER_HPP
#define HEADER_ONLINE_PROFILE_MANAGER_HPP

#include "http_manager.hpp"
#include "online/request.hpp"
#include "online/user.hpp"
#include "utils/types.hpp"


#include <irrString.h>

#include <string>

namespace Online{

    // ============================================================================

    /**
      * \brief Class that takes care of online profiles
      * \ingroup online
      */
    class ProfileManager
    {
        public:
            enum State
            {
                S_FETCHING = 1,
                S_READY
            };

            class ProfileInfoRequest : public XMLRequest
            {
                virtual void callback ();
            public:
                ProfileInfoRequest() : XMLRequest() {}
            };

        private:
            Synchronised<State>     m_state;
            bool                    m_is_current_user;
            bool                    m_has_fetched_overview;

            ProfileManager();

            void setState           (State state) { m_state.setAtomic(state); }

        public:
            /**Singleton */
            static ProfileManager *         get();
            static void                     deallocate();

            void set();
            void set(Online::User *);
            const State                     getState()          const { return m_state.getAtomic(); }

    };   // class CurrentUser

} // namespace Online

#endif

/*EOF*/
