//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2019 dumaosen
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

#ifndef HEADER_STR_TO_SCREEN_HPP
#define HEADER_STR_TO_SCREEN_HPP

#include <string>

/** This is a class function, which runs a screen from a string.
 * Only those general screens are added in order to make everything simple.
 * \ingroup states_screens
 */
class StrToScreen
{

private:
    /** The string of this strtoscreen. */
    std::string m_screen;

public:

    StrToScreen(std::string screen);

    /** Running the screen the string points to. */
    void runScreen();
    
};   // class StrToScreen

#endif
 