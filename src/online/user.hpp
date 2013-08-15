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

#ifndef HEADER_ONLINE_USER_HPP
#define HEADER_ONLINE_USER_HPP

#include <irrString.h>
#include "utils/types.hpp"
#include "utils/synchronised.hpp"
#include "io/xml_node.hpp"

namespace Online{

    /**
      * \brief Class that represents an online registered user
      * \ingroup online
      */
    class User
    {
        private:
            Synchronised<irr::core::stringw>    m_name;
            Synchronised<uint32_t>              m_id;

        protected:
            void        setUserName (const irr::core::stringw & name)       { m_name.setAtomic(name); }
            void        setUserID   (const uint32_t & id)                   { m_id.setAtomic(id); }

        public:
            User(       const irr::core::stringw & username,
                        const uint32_t           & userid
                );
            User(       const XMLNode * xml);
            virtual ~User() {};

            virtual const irr::core::stringw    getUserName()       const   { return m_name.getAtomic();  }
            const uint32_t                      getUserID()         const   { return m_id.getAtomic();    }
            User(uint32_t id);


    };   // class User
} // namespace Online
#endif

/*EOF*/
