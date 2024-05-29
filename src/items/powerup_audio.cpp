//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2024 Alayan
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

#include "items/powerup_audio.hpp"

#include "audio/sfx_base.hpp"
#include "audio/sfx_manager.hpp"
#include "items/powerup_manager.hpp"
#include "karts/controller/controller.hpp"
#include "karts/kart.hpp"
#include "modes/world.hpp"

//-----------------------------------------------------------------------------
PowerupAudio* PowerupAudio::getInstance()
{
	static PowerupAudio instance;
	return &instance;
} // getInstance

//-----------------------------------------------------------------------------
/** Constructor */
PowerupAudio::PowerupAudio()
{
    m_sudo_good = SFXManager::get()->createSoundSource("sudo_good");
    m_sudo_bad  = SFXManager::get()->createSoundSource("sudo_bad");

    m_powerup_sound = NULL;
} // PowerupAudio

//-----------------------------------------------------------------------------
/** Frees the memory for the sound effects.
 */
PowerupAudio::~PowerupAudio()
{
    m_sudo_good->deleteSFX();
    m_sudo_bad->deleteSFX();
	if(m_powerup_sound) m_powerup_sound->deleteSFX();
} // ~PowerupAudio

//-----------------------------------------------------------------------------
/** Does the sound configuration.
 */
void PowerupAudio::adjustSound(Kart* kart)
{
    if (m_powerup_sound == NULL)
        return;

    m_powerup_sound->setPosition(kart->getXYZ());
    // in multiplayer mode, sounds are NOT positional (because we have multiple listeners)
    // so the sounds of all AIs are constantly heard. So reduce volume of sounds.
    if (RaceManager::get()->getNumLocalPlayers() > 1)
    {
        // player karts played at full volume; AI karts much dimmer

        if (kart->getController()->isLocalPlayerController())
        {
            m_powerup_sound->setVolume( 1.0f );
        }
        else
        {
            m_powerup_sound->setVolume( 
                     std::min(0.5f, 1.0f / RaceManager::get()->getNumberOfKarts()) );
        }
    }
}   // adjustSound


//-----------------------------------------------------------------------------
/** Apply the on-usage sound effects of powerups
 */
void PowerupAudio::onUseAudio(Kart* kart, PowerupManager::PowerupType type, PowerupManager::MiniState mini_state, int sound_type)
{
    const int ticks = World::getWorld()->getTicksSinceStart();
    bool has_played_sound = false;
    auto it = m_played_sound_ticks.find(ticks);
    if (it != m_played_sound_ticks.end())
        has_played_sound = true;
    else
        m_played_sound_ticks.insert(ticks);

    if (has_played_sound)
    	return;

    resetSoundSource();

    // Play custom kart sound when collectible is used //TODO: what about the bubble gum?
    if (type != PowerupManager::POWERUP_NOTHING &&
        type != PowerupManager::POWERUP_SWATTER &&
        type != PowerupManager::POWERUP_ZIPPER)
        kart->playCustomSFX(SFXManager::CUSTOM_SHOOT);

    // FIXME - for some collectibles, set() is never called
    if (m_powerup_sound == NULL)
        m_powerup_sound = SFXManager::get()->createSoundSource("shoot");

    switch (type)
    {
    case PowerupManager::POWERUP_ZIPPER:
		// TODO
        break;
    case PowerupManager::POWERUP_SWITCH:
    	m_powerup_sound = SFXManager::get()->createSoundSource("swap");
        m_powerup_sound->play();
        break;
    case PowerupManager::POWERUP_CAKE:       // Fall-through
    case PowerupManager::POWERUP_PLUNGER:
    	m_powerup_sound = SFXManager::get()->createSoundSource("shoot");
        adjustSound(kart);
        m_powerup_sound->play();
        break;
    case PowerupManager::POWERUP_RUBBERBALL: // Fall-through
        adjustSound(kart);
        m_powerup_sound->play();
        break;
    case PowerupManager::POWERUP_BOWLING:    // Fall-through
    	m_powerup_sound = SFXManager::get()->createSoundSource("bowling_shoot");
        adjustSound(kart);
        m_powerup_sound->play();
        break;
    case PowerupManager::POWERUP_SWATTER:
    	break;
    case PowerupManager::POWERUP_SUDO:
        // Play a good sound for the kart that benefits from the "nitro-hack",
        // if it's a local player
        if (kart->getController()->isLocalPlayerController())
            PowerupAudio::getInstance()->playSudoGoodSFX();
        // Play a bad sound if there is an affected local player
        // FIXME if (player_kart != NULL)
            // FIXME PowerupAudio::getInstance()->playSudoBadSFX();

        break;
    case PowerupManager::POWERUP_ELECTRO:
    	break;
    case PowerupManager::POWERUP_MINI:
        switch (mini_state)
        {
            case PowerupManager::NOT_MINI:
            case PowerupManager::MINI_SELECT:
            	break;
            case PowerupManager::MINI_CAKE:
                resetSoundSource();
                m_powerup_sound = SFXManager::get()->createSoundSource("shoot");
                adjustSound(kart);
                m_powerup_sound->play();
                break;
            // Mini-zipper case
            case PowerupManager::MINI_ZIPPER:
            		// TODO
                    break;
            // Mini-gum case
            case PowerupManager::MINI_GUM:
                playGumSound(kart, sound_type);
                break;
        } // Switch mini-state
        break;
    case PowerupManager::POWERUP_BUBBLEGUM:
        playGumSound(kart, sound_type);
        break;

    case PowerupManager::POWERUP_ANVIL:
    	m_powerup_sound = SFXManager::get()->createSoundSource("anvil");
        // Not worth the effort
        break;

    case PowerupManager::POWERUP_PARACHUTE:
    	m_powerup_sound = SFXManager::get()->createSoundSource("parachute");
        // should we position the sound at the kart that is hit,
        // or the kart "throwing" the anvil? Ideally it should be both.
        // Meanwhile, don't play it near AI karts since they obviously
        // don't hear anything
        if(kart->getController()->isLocalPlayerController())
            m_powerup_sound->setPosition(kart->getXYZ());
        // FIXME else if(player_kart)
            //FIXME m_powerup_sound->setPosition(player_kart->getXYZ());
        m_powerup_sound->play();
        break;

    case PowerupManager::POWERUP_NOTHING:
        if(!kart->getKartAnimation())
            kart->beep();
        break;
    default : break;
    }
} // onUseAudio

//-----------------------------------------------------------------------------
void PowerupAudio::playGumSound(Kart* kart, int sound_type)
{
	if (sound_type >= 1) // A sound type of 0 indicates nothing to be played
	{
        resetSoundSource();
        //Extraordinary. Usually sounds are set in Powerup::set()
		//In this case this is a workaround, since the bubblegum item has two different sounds.
        if (sound_type == 1) // gum successfully dropped on the ground
	        m_powerup_sound = SFXManager::get()->createSoundSource("goo");
	    else if (sound_type == 2) // gum shield inflating
	        m_powerup_sound = SFXManager::get()->createSoundSource("inflate");
        adjustSound(kart);
        m_powerup_sound->play();
    }
}

//-----------------------------------------------------------------------------
void PowerupAudio::update(Kart* kart, int ticks)
{
	/*
    // Remove any sound ticks that should have played
    const int remove_ticks = World::getWorld()->getTicksSinceStart() - 1000;
    for (auto it = m_played_sound_ticks.begin();
         it != m_played_sound_ticks.end();)
    {
        if (*it < remove_ticks)
        {
            it = m_played_sound_ticks.erase(it);
            continue;
        }
        break;
    }
    */
}   // update

//-----------------------------------------------------------------------------
/** This function ensure we don't leak sound sources */
void PowerupAudio::resetSoundSource()
{
    if (m_powerup_sound != NULL)
    {
        m_powerup_sound->deleteSFX();
        m_powerup_sound = NULL;
    }
} // resetSoundSource