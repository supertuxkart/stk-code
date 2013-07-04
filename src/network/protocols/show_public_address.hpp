//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 SuperTuxKart-Team
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

#ifndef SHOW_PUBLIC_ADDRESS_HPP
#define SHOW_PUBLIC_ADDRESS_HPP

#include "network/protocol.hpp"
#include <string>

class ShowPublicAddress : public Protocol
{
    public:
        ShowPublicAddress(CallbackObject* callback_object);
        virtual ~ShowPublicAddress();
        
        virtual void notifyEvent(Event* event);
        virtual void setup();
        virtual void update();
        
        virtual void setUsername(std::string username);
        virtual void setPassword(std::string password);
        virtual void setPublicAddress(uint32_t ip, uint16_t port);
        
    protected:
        std::string m_username;
        std::string m_password;
        uint32_t m_public_ip;
        uint16_t m_public_port;
        
        enum STATE
        {
            NONE,
            DONE
        };
        STATE m_state;
};

#endif // HIDE_PUBLIC_ADDRESS_HPP
