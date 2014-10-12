//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011-2013  Joerg Henrichs, Marianne Gagnon
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

#include "audio/sfx_base.hpp"
#include "audio/sfx_manager.hpp"
#include "graphics/weather.hpp"
#include "modes/world.hpp"
#include "states_screens/race_gui.hpp"
#include "utils/random_generator.hpp"


// The weather manager

Weather::Weather(bool lightning, std::string sound)
{
    m_lightning = lightning;
    m_thunder_sound = NULL;
    m_weather_sound = NULL;
    
    if (m_lightning)
    {
        m_thunder_sound = SFXManager::get()->createSoundSource("thunder");        
    }

    if (sound != "")
    {
        m_weather_sound = SFXManager::get()->createSoundSource(sound);
    }

    RandomGenerator g;
    m_next_lightning = (float)g.get(35);
}   // Weather

// ----------------------------------------------------------------------------

Weather::~Weather()
{
    if (m_thunder_sound != NULL) 
        m_thunder_sound->deleteSFX();
        
    if (m_weather_sound != NULL) 
        m_weather_sound->deleteSFX();
}   // ~Weather

// ----------------------------------------------------------------------------

void Weather::update(float dt)
{
    if (m_lightning)
    {
        m_next_lightning -= dt;
    
        if (m_next_lightning < 0.0f)
        {
            RaceGUIBase* gui_base = World::getWorld()->getRaceGUI();

            if (gui_base != NULL)
            {
                gui_base->doLightning();

                if (m_thunder_sound) 
                {
                    m_thunder_sound->play();
                }
            }
    
            RandomGenerator g;
            m_next_lightning = 35 + (float)g.get(35);
        }
    }
}   // update

// ----------------------------------------------------------------------------

void Weather::playSound()
{
    if (m_weather_sound) 
    {
        m_weather_sound->setLoop(true);
        m_weather_sound->play();
    }   
}
