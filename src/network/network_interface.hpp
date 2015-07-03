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

/*! \file network_interface.hpp
 *  \brief Defines an interface to network middle-level functions.
 */

#ifndef NETWORK_INTERFACE_H
#define NETWORK_INTERFACE_H

#include "network/types.hpp"
#include "network/network_manager.hpp"
#include "utils/singleton.hpp"

#include <pthread.h>
#include <string>

/** \class NetworkInterface
  * \ingroup network
  */
class NetworkInterface : public AbstractSingleton<NetworkInterface>
{
    friend class AbstractSingleton<NetworkInterface>;
    public:

        /*! \brief Used to init the network.
         *  \param server : True if we're a server.
         */
        void initNetwork(bool server);

    protected:
        // protected functions
        NetworkInterface();
        virtual ~NetworkInterface();

};

#endif // NETWORK_INTERFACE_H
