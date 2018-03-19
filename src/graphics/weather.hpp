//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011-2015  Joerg Henrichs, Marianne Gagnon
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

#ifndef HEADER_WEATHER_HPP
#define HEADER_WEATHER_HPP

#include "utils/singleton.hpp"
#include <vector3d.h>

class SFXBase;

class Weather : public AbstractSingleton<Weather>
{
    /** Counts ticks till the next lighting appears. */
    int m_next_lightning;

    /** Counts the ticks for displaying the current lighting. */
    int m_lightning;

    SFXBase* m_thunder_sound;
    SFXBase* m_weather_sound;

public:
             Weather();
    virtual ~Weather();

    void update(int ticks);
    void playSound();
    void startLightning();
    irr::core::vector3df getIntensity();

    bool shouldLightning() { return m_lightning > 0; }
    
};   // class Weather

#endif
