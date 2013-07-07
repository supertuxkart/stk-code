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

#include "network/protocol.hpp"

Protocol::Protocol(CallbackObject* callback_object, PROTOCOL_TYPE type)
{
    m_callback_object = callback_object;
    m_type = type;
}

Protocol::~Protocol()
{
}

void Protocol::pause()
{
    m_listener->requestPause(this);
}
void Protocol::unpause()
{
    m_listener->requestUnpause(this);
}

void Protocol::kill()
{
}

void Protocol::setListener(ProtocolManager* listener)
{
    m_listener = listener; 
}

PROTOCOL_TYPE Protocol::getProtocolType()
{
    return m_type;
}
