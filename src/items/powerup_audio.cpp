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

	for (int i=0; i<POWERUP_SOUND_SOURCES;i++)
	{
		m_powerup_sound[i] = NULL;
	}
	m_curr_sound = 0;
} // PowerupAudio

//-----------------------------------------------------------------------------
/** Does the sound configuration.
 */
void PowerupAudio::adjustSound(Kart* kart)
{
	if (m_powerup_sound[m_curr_sound] == NULL)
		return;

	m_powerup_sound[m_curr_sound]->setPosition(kart->getXYZ());
	// in multiplayer mode, sounds are NOT positional (because we have multiple listeners)
	// so the sounds of all AIs are constantly heard. So reduce volume of sounds.
	if (RaceManager::get()->getNumLocalPlayers() > 1)
	{
		// player karts played at full volume; AI karts much dimmer

		if (kart->getController()->isLocalPlayerController())
		{
			m_powerup_sound[m_curr_sound]->setVolume( 1.0f );
		}
		else
		{
			m_powerup_sound[m_curr_sound]->setVolume( 
					 std::min(0.5f, 1.0f / RaceManager::get()->getNumberOfKarts()) );
		}
	}
}   // adjustSound


//-----------------------------------------------------------------------------
/** Apply the on-usage sound effects of powerups
 */
void PowerupAudio::onUseAudio(Kart* kart, PowerupManager::PowerupType type, int sound_type, Kart* player_kart, PowerupManager::MiniState mini_state)
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

	prepareSoundSource();

	// Play custom kart sound when collectible is used //TODO: what about the bubble gum?
	if (type != PowerupManager::POWERUP_NOTHING &&
		type != PowerupManager::POWERUP_SWATTER &&
		type != PowerupManager::POWERUP_ZIPPER)
		kart->playCustomSFX(SFXManager::CUSTOM_SHOOT);

	switch (type)
	{
	case PowerupManager::POWERUP_ZIPPER:
		// TODO
		break;
	case PowerupManager::POWERUP_SWITCH:
		m_powerup_sound[m_curr_sound] = SFXManager::get()->createSoundSource("swap");
		m_powerup_sound[m_curr_sound]->play();
		break;
	case PowerupManager::POWERUP_CAKE:	     // Fall-through
	case PowerupManager::POWERUP_RUBBERBALL: // Fall-through
	case PowerupManager::POWERUP_PLUNGER:
		playShootSound(kart);
		break;
	case PowerupManager::POWERUP_BOWLING:
		m_powerup_sound[m_curr_sound] = SFXManager::get()->createSoundSource("bowling_shoot");
		adjustSound(kart);
		m_powerup_sound[m_curr_sound]->play();
		break;
	case PowerupManager::POWERUP_SWATTER:
		break;
	case PowerupManager::POWERUP_SUDO:
		// If a local player benefits from the "nitro-hack", play a good sound
		if (sound_type % 2 == 1)
			m_sudo_good->play();
		// Play a bad sound if there is an affected local player
		if (sound_type >= 2)
			m_sudo_bad->play();
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
				playShootSound(kart);
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
		m_powerup_sound[m_curr_sound] = SFXManager::get()->createSoundSource("anvil");
		// Not worth the effort
		break;

	case PowerupManager::POWERUP_PARACHUTE:
		m_powerup_sound[m_curr_sound] = SFXManager::get()->createSoundSource("parachute");

		// We play the parachute sound if the kart using it is a local player
		// or if one of the karts affected by the parachute is a local player.
		//
		// Although if the player kart is not affected by a nearby AI kart is,
		// it's a bit strange to not have the sound... However, creating as many
		// sound sources as there is karts affected is not desirable.
		//
		// TODO: support parachute sound in normal race replays (no local player involved)
		//       once normal race replays are implemented
		if (kart->getController()->isLocalPlayerController())
		{
			m_powerup_sound[m_curr_sound]->setPosition(kart->getXYZ());
			m_powerup_sound[m_curr_sound]->play();
		}
		else if (player_kart != NULL)
		{
			m_powerup_sound[m_curr_sound]->setPosition(player_kart->getXYZ());
			m_powerup_sound[m_curr_sound]->play();
		}
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
		if (sound_type == 1) // gum successfully dropped on the ground
			m_powerup_sound[m_curr_sound] = SFXManager::get()->createSoundSource("goo");
		else if (sound_type == 2) // gum shield inflating
			m_powerup_sound[m_curr_sound] = SFXManager::get()->createSoundSource("inflate");
		adjustSound(kart);
		m_powerup_sound[m_curr_sound]->play();
	}
} // playGumSound

//-----------------------------------------------------------------------------
/* Used by cakes, mini-cakes, basket-balls and plungers */
void PowerupAudio::playShootSound(Kart* kart)
{
    m_powerup_sound[m_curr_sound] = SFXManager::get()->createSoundSource("shoot");
    adjustSound(kart);
    m_powerup_sound[m_curr_sound]->play();
} // playShootSound

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
/** This function is used to pick which of the powerup sound sources will be
 *  used and ensures we don't leak sound sources. */
void PowerupAudio::prepareSoundSource()
{
// Select a sound source that's not already in use, if there is one
	for (int i=0 ; i<POWERUP_SOUND_SOURCES; i++)
	{
		if (m_powerup_sound[i] == NULL ||
			m_powerup_sound[i]->getStatus() != SFXBase::SFX_PLAYING)
		{
			m_curr_sound = i;
			break;
		}
	}

	// Avoid leaking sound sources
	if (m_powerup_sound[m_curr_sound] != NULL)
	{
		m_powerup_sound[m_curr_sound]->deleteSFX();
		m_powerup_sound[m_curr_sound] = NULL;
	}
} // prepareSoundSource