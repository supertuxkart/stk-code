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

#include "world_timer.hpp"

/** Constructs the world timer.*/
WorldTimer::WorldTimer()
{
	IrrlichtDevice* device = irr_driver->getDevice();
	m_freeze = device->getTimer()->isStopped();
}	//WorldTimer

/** Destructor the world timer.*/
WorldTimer::~WorldTimer()
{
	//unfreeze timer before left
	unfreezeTimer();
}	//~WorldTimer

/** Stop timer of the game*/
void WorldTimer::freezeTimer()
{
	if (!m_freeze)
	{
		IrrlichtDevice* device = irr_driver->getDevice();
		device->getTimer()->stop();
		m_freeze = device->getTimer()->isStopped();
	}
}	//freezeTimer

/** Restart timer of the game*/
void WorldTimer::unfreezeTimer()
{
	if (m_freeze)
	{
		IrrlichtDevice* device = irr_driver->getDevice();
		device->getTimer()->start();
		m_freeze = device->getTimer()->isStopped();
	}
}	//unfreezeTimer