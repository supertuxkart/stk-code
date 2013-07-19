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

namespace Online
{
    namespace Messages
    {
        irr::core::stringw loadingDots(float timer, bool spaces, float interval, int max_dots)
        {
            int nr_dots = int(floor(timer * (1 / interval))) % (max_dots+1);
            return irr::core::stringw((std::string(nr_dots,'.') + std::string(max_dots-nr_dots,' ')).c_str());
        }
        // ------------------------------------------------------------------------

        irr::core::stringw signingIn(float timer)
        {
            return irr::core::stringw(_("Signing in")) + loadingDots(timer);
        }
        // ------------------------------------------------------------------------

        irr::core::stringw signingOut(float timer)
        {
            return irr::core::stringw(_("Signing out")) + loadingDots(timer);
        }
        // ------------------------------------------------------------------------

        irr::core::stringw signedInAs(const irr::core::stringw & name)
        {
            return irr::core::stringw(_("Signed in as : ")) + name + ".";
        }
    } // namespace messages
} // namespace Online


/* EOF */
