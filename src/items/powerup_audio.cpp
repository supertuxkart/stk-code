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

//-----------------------------------------------------------------------------
PowerupAudio* PowerupAudio::getInstance()
{
	static PowerupAudio instance;
	return &instance;
}

//-----------------------------------------------------------------------------
/** Constructor */
PowerupAudio::PowerupAudio()
{
	printf("Constructor called\n");
    m_sudo_good = SFXManager::get()->createSoundSource("sudo_good");
    m_sudo_bad  = SFXManager::get()->createSoundSource("sudo_bad");
}

//-----------------------------------------------------------------------------
/** Frees the memory for the sound effects.
 */
PowerupAudio::~PowerupAudio()
{
	printf("Destructor called\n");
    m_sudo_good->deleteSFX();
    m_sudo_bad->deleteSFX();
}   // ~Powerup
