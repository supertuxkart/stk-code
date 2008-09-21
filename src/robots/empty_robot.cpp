/* This is a template to avoid writing boring stuff again and again when
 * creating a new robot.
 *
 * Don't forget to replace the copyright in the fourth line of the header in
 * this file and the header file, with the current year and your name, the
 * #include to <name of your robot>_robot.hpp, the class name references in
 * this file and in empty_robot.hpp to <robot name>Robot and the #ifdef at
 * beginning of the header file.
 *
 * You should also delete these intructions. Oh, and if you use this template
 * as a robot, it does nothing.
 */

//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006-2007 Eduardo Hernandez Munoz
//  Copyright (C) <insert year here> <insert name here>
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

#include "modes/world.hpp"

#include "empty_robot.hpp"

EmptyRobot::EmptyRobot(const KartProperties *kart_properties, int position,
                   sgCoord init_pos) :
    AutoKart(kart_properties, position, init_pos)
{
    //This is called just once per *competition*

    reset();
}
//-----------------------------------------------------------------------------
void EmptyRobot::update (float delta)
{
    /*General kart stuff*/
    AutoKart::update(delta);
}

//-----------------------------------------------------------------------------
void EmptyRobot::reset()
{
    //This function is called at the beginning of *each race*

    m_controls.lr      = 0.0;
    m_controls.accel   = false;
    m_controls.brake   = false;
    m_controls.brake   = false;
    m_controls.wheelie = false;
    m_controls.jump    = false;
    m_controls.rescue  = false;
    m_controls.fire    = false;
    /*General kart stuff*/
    AutoKart::reset();
}
