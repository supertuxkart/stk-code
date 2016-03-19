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

#ifndef HIDE_PUBLIC_ADDRESS_HPP
#define HIDE_PUBLIC_ADDRESS_HPP

#include "network/protocol.hpp"
#include "utils/cpp2011.hpp"

#include <string>

namespace Online { class XMLRequest; }

class HidePublicAddress : public Protocol
{
private:
    Online::XMLRequest* m_request;
    enum STATE
    {
        NONE,
        REQUEST_PENDING,
        DONE,
        EXITING
    };
    STATE m_state;

public:
    HidePublicAddress();
    virtual ~HidePublicAddress();

    virtual void asynchronousUpdate() OVERRIDE;
    virtual void setup() OVERRIDE;
    // ------------------------------------------------------------------------
    virtual bool notifyEvent(Event* event) OVERRIDE { return true; }
    // ------------------------------------------------------------------------
    virtual bool notifyEventAsynchronous(Event* event) OVERRIDE { return true; }
    // ------------------------------------------------------------------------
    virtual void update(float dt) OVERRIDE {}
};   // class HidePublicAddress

#endif // HIDE_PUBLIC_ADDRESS_HPP
