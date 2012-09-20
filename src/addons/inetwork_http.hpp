//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 Lucas Baudin
//                2011 Lucas Baudin, Joerg Henrichs
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

#ifndef HEADER_INETWORK_HTTP_HPP
#define HEADER_INETWORK_HTTP_HPP


#include "addons/request.hpp"

class XMLNode;

/**
 * \ingroup addonsgroup
 * Abstract base interface for the network manager
 */
class INetworkHttp
{

private:
    /** The one instance of this object. */
    static INetworkHttp *m_network_http;

public:
    /** If stk has permission to access the internet (for news
     *  server etc).
     *  IPERM_NOT_ASKED: The user needs to be asked if he wants to 
     *                   grant permission
     *  IPERM_ALLOWED:   STK is allowed to access server.
     *  IPERM_NOT_ALLOWED: STK must not access external servers. */
    enum InternetPermission {IPERM_NOT_ASKED  =0,
        IPERM_ALLOWED    =1,
        IPERM_NOT_ALLOWED=2 };
    
public:
    virtual ~INetworkHttp() {}
    virtual void          startNetworkThread() = 0;
    virtual void          stopNetworkThread() = 0;
    virtual void          insertReInit() = 0;
    virtual Request      *downloadFileAsynchron(const std::string &url, 
                                                const std::string &save = "",
                                                int   priority = 1,
                                                bool  manage_memory=true) = 0;
    virtual void          cancelAllDownloads() = 0;
    static void           create();
    static INetworkHttp  *get() { return m_network_http; }
    static void           destroy();
    
};   // NetworkHttp


#endif

