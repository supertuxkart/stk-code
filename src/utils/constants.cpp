//  SuperTuxKart - a fun racing game with go-kart
//
//  Copyright (C) 2012-2015 SuperTuxKart-Team
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

#include "utils/constants.hpp"

// for code that needs to know about endianness so do a simple test.
// 0 : little endian
// 1 : big endian
// I'm doing it at runtime rather than at compile-time to be friendly with
// cross-compilation (universal binaries on mac, namely)
static const int endianness_test = 0x01000000;
static const char* endianness_test_ptr = (const char*)&endianness_test;

// in little-endian, byte 0 will be 0. in big endian, byte 0 will be 1
const bool IS_LITTLE_ENDIAN = (endianness_test_ptr[0] == 0);

// "SUPERTUXKART_VERSION" is defined from CMakeLists.txt from the project version
const char STK_VERSION[] = SUPERTUXKART_VERSION;
