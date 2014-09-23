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

#ifndef HEADER_TYPES_HPP
#define HEADER_TYPES_HPP

    #if defined(_MSC_VER) && _MSC_VER < 1700
        typedef unsigned char    uint8_t;
        typedef unsigned short   uint16_t;
        typedef __int32          int32_t;
        typedef unsigned __int32 uint32_t;
        typedef __int64          int64_t;
        typedef unsigned __int64 uint64_t;
    #elif defined(_MSC_VER) && _MSC_VER >= 1700 	
        #include <stdint.h>
        // We can't use the ifndef SOCKET_ERROR below in this case, since
        // SOCKET_ERROR will be defined again in winsock2.h later 
        // - without the #ifdef guard then causing a compiler error.
    #else
        #include <stdint.h>
        #ifndef SOCKET_ERROR
            #define SOCKET_ERROR -1
        #endif
    #endif

#endif
