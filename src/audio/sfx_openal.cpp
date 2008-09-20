//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Patrick Ammann <pammann@aro.ch>
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

#if HAVE_OGGVORBIS

#include "audio/sfx_openal.hpp"

#include <assert.h>
#include <stdio.h>
#include <string>

#ifdef __APPLE__
#  include <OpenAL/al.h>
#else
#  include <AL/al.h>
#endif

#include <SDL/SDL.h>
#include <SDL/SDL_endian.h>

#include "file_manager.hpp"
#include "user_config.hpp"

SFXOpenAL::SFXOpenAL(ALuint buffer) : SFXBase()
{
    m_soundBuffer = buffer;
    m_soundSource = 0;
    m_ok          = false;

    alGenSources(1, &m_soundSource );
    if(!SFXManager::checkError("generating a source")) return;

    // not 3D yet
    alSourcei (m_soundSource, AL_BUFFER,          m_soundBuffer);
    alSource3f(m_soundSource, AL_POSITION,        0.0, 0.0, 5.0);
    alSource3f(m_soundSource, AL_VELOCITY,        0.0, 0.0, 0.0);
    alSource3f(m_soundSource, AL_DIRECTION,       0.0, 0.0, 0.0);
    alSourcef (m_soundSource, AL_ROLLOFF_FACTOR,  0.2f         );
    alSourcei (m_soundSource, AL_SOURCE_RELATIVE, AL_TRUE      );

    m_ok = SFXManager::checkError("setting up the source");
}   // SFXOpenAL

//-----------------------------------------------------------------------------
SFXOpenAL::~SFXOpenAL()
{
    alDeleteSources(1, &m_soundSource);
}   // ~SFXOpenAL

//-----------------------------------------------------------------------------
/** Changes the pitch of a sound effect.
 *  \param factor Speedup/slowdown between 0.5 and 2.0
 */
void SFXOpenAL::speed(float factor)
{
    if(!sfx_manager->sfxAllowed()||!m_ok) return;

    //OpenAL only accepts pitches in the range of 0.5 to 2.0
    if(factor > 2.0f)
    {
        factor = 2.0f;
    }
    if(factor < 0.5f)
    {
        factor = 0.5f;
    }
    alSourcef(m_soundSource,AL_PITCH,factor);
    SFXManager::checkError("changing the speed");
}   // speed

//-----------------------------------------------------------------------------
/** Loops this sound effect.
 */
void SFXOpenAL::loop()
{
    if(!m_ok) return;

    alSourcei(m_soundSource, AL_LOOPING, AL_TRUE);
    SFXManager::checkError("looping");
}   // loop

//-----------------------------------------------------------------------------
/** Stops playing this sound effect.
 */
void SFXOpenAL::stop()
{
    if(!sfx_manager->sfxAllowed()||!m_ok) return;

    alSourcei(m_soundSource, AL_LOOPING, AL_FALSE);
    alSourceStop(m_soundSource);
    SFXManager::checkError("stoping");
}   // stop

//-----------------------------------------------------------------------------
/** Pauses a SFX that's currently played. Nothing happens it the effect is
 *  currently not being played.
 */
void SFXOpenAL::pause()
{
    alSourcePause(m_soundSource);
    SFXManager::checkError("pausing");
}   // pause

//-----------------------------------------------------------------------------
/** Resumes a sound effect.
 */
void SFXOpenAL::resume()
{
    if(!sfx_manager->sfxAllowed()||!m_ok) return;

    alSourcef(m_soundSource,AL_GAIN,user_config->m_sfx_volume);
    alSourcePlay(m_soundSource);
    SFXManager::checkError("resuming");
}   // resume

//-----------------------------------------------------------------------------
/** Plays this sound effect.
 */
void SFXOpenAL::play()
{
    if(!sfx_manager->sfxAllowed()||!m_ok) return;

    alSourcef(m_soundSource,AL_GAIN,user_config->m_sfx_volume);
    alSourcePlay(m_soundSource);
    SFXManager::checkError("playing");
}   // play

//-----------------------------------------------------------------------------
/** Sets the position where this sound effects is played.
 *  \param position Position of the sound effect.
 */
void SFXOpenAL::position(Vec3 position)
{
    if(!sfx_manager->sfxAllowed()||!m_ok) return;

    alSource3f(m_soundSource, AL_POSITION,
               position.getX(), position.getY(), position.getZ());
    SFXManager::checkError("positioning");
}   // position

//-----------------------------------------------------------------------------
/** Returns the status of this sound effect.
 */
SFXManager::SFXStatus SFXOpenAL::getStatus()
{
    if(!sfx_manager->sfxAllowed()||!m_ok) return SFXManager::SFX_UNKNOWN;

	int state = 0;
    alGetSourcei(m_soundSource, AL_SOURCE_STATE, &state);
    switch(state)
    {
    case AL_STOPPED: return SFXManager::SFX_STOPED;
    case AL_PLAYING: return SFXManager::SFX_PLAYING;
    case AL_PAUSED:  return SFXManager::SFX_PAUSED;
    case AL_INITIAL: return SFXManager::SFX_INITIAL;
    default:         return SFXManager::SFX_UNKNOWN;
    }
}   // getStatus

#endif //if HAVE_OGGVORBIS
