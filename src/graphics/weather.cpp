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

#include "audio/sfx_base.hpp"
#include "audio/sfx_manager.hpp"
#include "graphics/weather.hpp"
#include "modes/world.hpp"
#include "tracks/track.hpp"
#include "utils/random_generator.hpp"


/**  The weather manager stores information about the weather.
 */
Weather::Weather()
{
    m_thunder_sound = NULL;
    m_weather_sound = NULL;
    m_lightning = 0.0f;
    
    if (Track::getCurrentTrack()->getWeatherLightning())
    {
        m_thunder_sound = SFXManager::get()->createSoundSource("thunder");
    }

    const std::string &sound = Track::getCurrentTrack()->getWeatherSound();
    if (!sound.empty())
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
    if (!Track::getCurrentTrack()->getWeatherLightning())
        return;
        
    if (World::getWorld()->getRaceGUI() == NULL)
        return;
        
    m_next_lightning -= dt;

    if (m_next_lightning < 0.0f)
    {
        startLightning();

        if (m_thunder_sound &&
            World::getWorld()->getPhase() != WorldStatus::IN_GAME_MENU_PHASE)
        {
            m_thunder_sound->play();
        }

        RandomGenerator g;
        m_next_lightning = 35 + (float)g.get(35);
    }
    
    if (m_lightning > 0.0f)
    {
        m_lightning -= dt;
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

irr::core::vector3df Weather::getIntensity()
{
    irr::core::vector3df value = {0.7f * m_lightning, 
                                  0.7f * m_lightning, 
                                  0.7f * std::min(1.0f, m_lightning * 1.5f)};
                                 
    return value;
}
