//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 SuperTuxKart-Team
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

#ifndef HEADER_WORLD_TIMER_HPP
#define HEADER_WORLD_TIMER_HPP

#include "graphics/irr_driver.hpp"

/** 
 * When pause the game, something must be 'freeze' such as particle (nitro, terrain smoke...) and terrain animation(water...).
 * WorldTimer will stop the timer of the game when gamer pause game, so that all thing bellow will not animate,
 * when gamer resume, this class will start the timer, too.
 */
class WorldTimer
{
private:
	bool m_freeze;
public:
	WorldTimer();
	~WorldTimer();

	/** Stop timer of the game*/
	void freezeTimer();
	/** Restart timer of the game*/
	void unfreezeTimer();
};

#endif