//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011-2013 Joerg Henrichs
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

#include "graphics/hit_sfx.hpp"

#include "audio/sfx_base.hpp"
#include "audio/sfx_manager.hpp"
#include "race/race_manager.hpp"

/** Creates a sound effect when something was hit. */
HitSFX::HitSFX(const Vec3& coord, const char* explosion_sound)
             : HitEffect()
{
    m_sfx = SFXManager::get()->createSoundSource( explosion_sound );
    m_sfx->setPosition(coord);

    // in multiplayer mode, sounds are NOT positional (because we have
    // multiple listeners) so the sounds of all AIs are constantly heard.
    // Therefore reduce volume of sounds.
    float vol = race_manager->getNumLocalPlayers() > 1 ? 0.5f : 1.0f;
    m_sfx->setVolume(vol);
    m_sfx->play();
}   // HitSFX

//-----------------------------------------------------------------------------
/** Destructor stops the explosion sfx from being played and frees its memory.
 */
HitSFX::~HitSFX()
{
    m_sfx->deleteSFX();
}   // ~HitEffect

//-----------------------------------------------------------------------------
/** Called if this hit effect is for a player kart (in which case it might be
 *  played louder than for a non-player kart if split screen is used).
 *  If this sfx is for a player kart in split screen, make it louder again.
 */
void HitSFX::setPlayerKartHit()
{
    if(race_manager->getNumLocalPlayers())
        m_sfx->setVolume(1.0f);
}   // setPlayerKartHit

//-----------------------------------------------------------------------------
/** Updates the hit sfx, called one per time step. If this function returns
 *  true, the effect will be deleted.
 *  \param dt Time step size.
 *  \return true If the explosion is finished.
 */
bool HitSFX::updateAndDelete(float dt)
{
    SFXBase::SFXStatus status = m_sfx->getStatus();
    return status!= SFXBase::SFX_PLAYING;
}   // updateAndDelete
