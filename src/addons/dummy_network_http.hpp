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

#ifndef HEADER_DUMMY_NETWORK_HTTP_HPP
#define HEADER_DUMMY_NETWORK_HTTP_HPP


#include "addons/request.hpp"

class XMLNode;

/**
 * \ingroup addonsgroup
 * Dummy implementation used when curl is not available
 */
class DummyNetworkHttp
{
    
public:
    virtual ~DummyNetworkHttp() {}
    virtual void          startNetworkThread() {}
    virtual void          stopNetworkThread() {}
    virtual void          insertReInit() {}
    virtual Request      *downloadFileAsynchron(const std::string &url, 
                                                const std::string &save = "",
                                                int   priority = 1,
                                                bool  manage_memory=true) { return NULL; }
    virtual void          cancelAllDownloads() {}
};   // NetworkHttp


#endif

