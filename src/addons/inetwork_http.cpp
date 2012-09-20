//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012 Joerg Henrichs
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

#include "addons/dummy_network_http.hpp"
#include "addons/inetwork_http.hpp"
#include "addons/network_http.hpp"

#include <assert.h>

INetworkHttp *INetworkHttp::m_network_http = NULL;

/** Creates the network_http instance (depending on compile time options).
 */
void INetworkHttp::create()
{
    assert(m_network_http == NULL);
#ifdef NO_CURL
    m_network_http = new DummyNetworkHttp();
#else
    m_network_http = new NetworkHttp();
#endif
}   // create

// ----------------------------------------------------------------------------
/** Destroys the network_http instance.
 */
void INetworkHttp::destroy()
{
    if(m_network_http)
    {
        delete m_network_http;
        m_network_http = NULL;
    }
}   // destroy
