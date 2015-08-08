//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 SuperTuxKart-Team
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

#ifndef GET_PUBLIC_ADDRESS_HPP
#define GET_PUBLIC_ADDRESS_HPP

#include "network/protocol.hpp"

#include <string>

class GetPublicAddress : public Protocol
{
    public:
        GetPublicAddress(CallbackObject* callback_object);
        virtual ~GetPublicAddress();

        virtual bool notifyEvent(Event* event) { return true; }
        virtual bool notifyEventAsynchronous(Event* event) { return true; }
        virtual void setup();
        virtual void update() {}
        virtual void asynchronousUpdate();

    protected:
        enum STATE
        {
            NOTHING_DONE,
            TEST_SENT,
            ADDRESS_KNOWN,
            EXITING
        };
        STATE m_state;
        uint32_t m_stun_tansaction_id[3];
        static const uint32_t m_stun_magic_cookie = 0x2112A442;
        uint32_t m_stun_server_ip;
        STKHost* m_transaction_host;

    private:
        std::string parseResponse();
};

#endif // GET_PUBLIC_ADDRESS_HPP
