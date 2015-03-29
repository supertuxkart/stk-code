//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 SuperTuxKart-Team
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

#include "graphics/irr_driver.hpp"
#include "graphics/wind.hpp"
#include "utils/helpers.hpp"

Wind::Wind()
{
    m_seed = (float)((rand() % 1000) - 500);
}

vector3df Wind::getWind() const
{
    return m_wind;
}

void Wind::update()
{
    vector3df dir(0, 0, 1);
    const u32 now = irr_driver->getDevice()->getTimer()->getTime();

    const float rotation = asinf(noise2d(m_seed, now / 100000.0f)) * RAD_TO_DEGREE;
    dir.rotateXZBy(rotation);

    m_wind = dir;
}
