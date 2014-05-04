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

#include "online/messages.hpp"
#include "utils/translation.hpp"
#include "utils/time.hpp"

namespace Online
{
    namespace Messages
    {
        irr::core::stringw signingIn()
        {
            return irr::core::stringw(_("Signing in")) + loadingDots();
        }
        // ------------------------------------------------------------------------

        irr::core::stringw signingOut()
        {
            return irr::core::stringw(_("Signing out")) + loadingDots();
        }
        // ------------------------------------------------------------------------
        irr::core::stringw validatingInfo()
        {
            return irr::core::stringw(_("Validating info")) + loadingDots();
        }
        // ------------------------------------------------------------------------
        irr::core::stringw searching()
        {
            return irr::core::stringw(_("Searching")) + loadingDots();
        }
        // ------------------------------------------------------------------------

        irr::core::stringw joiningServer()
        {
            return irr::core::stringw(_("Joining server")) + loadingDots();
        }
        // ------------------------------------------------------------------------

        irr::core::stringw creatingServer()
        {
            return irr::core::stringw(_("Creating server")) + loadingDots();
        }

        // ------------------------------------------------------------------------

        irr::core::stringw fetchingServers()
        {
            return irr::core::stringw(_("Fetching servers")) + loadingDots();
        }

        // ------------------------------------------------------------------------

        irr::core::stringw fetchingFriends()
        {
            return irr::core::stringw(_("Fetching friends")) + loadingDots();
        }

        // ------------------------------------------------------------------------

        irr::core::stringw fetchingAchievements()
        {
            return irr::core::stringw(_("Fetching achievements")) + loadingDots();
        }

        // ------------------------------------------------------------------------

        irr::core::stringw processing()
        {
            return irr::core::stringw(_("Processing")) + loadingDots();
        }

        // ------------------------------------------------------------------------
        /** Convenience function to type less in calls. */
        irr::core::stringw loadingDots(const wchar_t *s)
        {
            return irr::core::stringw(s)+loadingDots();
        }
        // ------------------------------------------------------------------------
        /**
          * Shows a increasing number of dots.
          * \param interval A float representing the time it takes to add a new dot
          * \param max_dots The number of dots used. Defaults to 3.
          */
        irr::core::stringw loadingDots(float interval, int max_dots)
        {
            int nr_dots = int(floor(StkTime::getRealTime() / interval)) % (max_dots+1);
            return irr::core::stringw((std::string(nr_dots,'.') + std::string(max_dots-nr_dots,' ')).c_str());
        }
    } // namespace messages
} // namespace Online


/* EOF */
