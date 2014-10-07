//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2013 Patrick Ammann <pammann@aro.ch>
//  Copyright (C) 2009-2013 Marianne Gagnon
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

#include "audio/sfx_buffer.hpp"
#include "config/user_config.hpp"
#include "io/file_manager.hpp"
#include "race/race_manager.hpp"
#include "utils/vs.hpp"

#ifdef __APPLE__
#  include <OpenAL/al.h>
#else
#  include <AL/al.h>
#endif

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string>

SFXOpenAL::SFXOpenAL(SFXBuffer* buffer, bool positional, float gain, bool ownsBuffer) : SFXBase()
{
    m_soundBuffer = buffer;
    m_soundSource = 0;
    m_ok          = false;
    m_positional  = positional;
    m_defaultGain = gain;
    m_loop        = false;
    m_gain        = -1.0f;
    m_master_gain = 1.0f;
    m_owns_buffer = ownsBuffer;

    // Don't initialise anything else if the sfx manager was not correctly
    // initialised. First of all the initialisation will not work, and it
    // will not be used anyway.
    if (SFXManager::get()->sfxAllowed())
    {
        init();
    }
}   // SFXOpenAL

//-----------------------------------------------------------------------------

SFXOpenAL::~SFXOpenAL()
{
    if (m_ok)
    {
        alDeleteSources(1, &m_soundSource);
    }

    if (m_owns_buffer && m_soundBuffer != NULL)
    {
        m_soundBuffer->unload();
        delete m_soundBuffer;
    }
}   // ~SFXOpenAL

//-----------------------------------------------------------------------------

bool SFXOpenAL::init()
{
    alGenSources(1, &m_soundSource );
    if (!SFXManager::checkError("generating a source")) return false;

    assert( alIsBuffer(m_soundBuffer->getBufferID()) );
    assert( alIsSource(m_soundSource) );

    //Log::info("SFXOpenAL", "Setting a source with buffer, %p, rolloff %f, gain = %f, position = %s",
    //    m_soundBuffer, rolloff, m_defaultGain, positional ? "true" : "false");

    alSourcei (m_soundSource, AL_BUFFER, m_soundBuffer->getBufferID());

    if (!SFXManager::checkError("attaching the buffer to the source"))
        return false;

    alSource3f(m_soundSource, AL_POSITION,       0.0, 0.0, 0.0);
    alSource3f(m_soundSource, AL_VELOCITY,       0.0, 0.0, 0.0);
    alSource3f(m_soundSource, AL_DIRECTION,      0.0, 0.0, 0.0);

    alSourcef (m_soundSource, AL_ROLLOFF_FACTOR, m_soundBuffer->getRolloff());
    alSourcef (m_soundSource, AL_MAX_DISTANCE,   m_soundBuffer->getMaxDist());

    if (m_gain < 0.0f)
    {
        alSourcef (m_soundSource, AL_GAIN, m_defaultGain * m_master_gain);
    }
    else
    {
        alSourcef (m_soundSource, AL_GAIN, m_gain * m_master_gain);
    }

    if (m_positional) alSourcei (m_soundSource, AL_SOURCE_RELATIVE, AL_FALSE);
    else              alSourcei (m_soundSource, AL_SOURCE_RELATIVE, AL_TRUE);

    alSourcei(m_soundSource, AL_LOOPING, m_loop ? AL_TRUE : AL_FALSE);

    m_ok = SFXManager::checkError("setting up the source");

    return m_ok;
}   // init

//-----------------------------------------------------------------------------
/** Changes the pitch of a sound effect.
 *  \param factor Speedup/slowdown between 0.5 and 2.0
 */
void SFXOpenAL::speed(float factor)
{
    if(!m_ok || isnan(factor)) return;

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
/** Changes the volume of a sound effect.
 *  \param gain Volume adjustment between 0.0 (mute) and 1.0 (full volume).
 */
void SFXOpenAL::volume(float gain)
{
    m_gain = m_defaultGain * gain;

    if(!m_ok) return;

    alSourcef(m_soundSource, AL_GAIN, m_gain * m_master_gain);
    SFXManager::checkError("setting volume");
}   // volume

//-----------------------------------------------------------------------------

void SFXOpenAL::setMasterVolume(float gain)
{
    m_master_gain = gain;
    
    if(!m_ok) return;

    alSourcef(m_soundSource, AL_GAIN, 
               (m_gain < 0.0f ? m_defaultGain : m_gain) * m_master_gain);
    SFXManager::checkError("setting volume");
}   //setMasterVolume

//-----------------------------------------------------------------------------
/** Loops this sound effect.
 */
void SFXOpenAL::setLoop(bool status)
{
    m_loop = status;

    if(!m_ok) return;

    alSourcei(m_soundSource, AL_LOOPING, status ? AL_TRUE : AL_FALSE);
    SFXManager::checkError("looping");
}   // loop

//-----------------------------------------------------------------------------
/** Stops playing this sound effect.
 */
void SFXOpenAL::stop()
{
    if(!m_ok) return;

    m_loop = false;
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
    if(!m_ok) return;
    alSourcePause(m_soundSource);
    SFXManager::checkError("pausing");
}   // pause

//-----------------------------------------------------------------------------
/** Resumes a sound effect.
 */
void SFXOpenAL::resume()
{
    if (!m_ok)
    {
        // lazily create OpenAL source when needed
        init();

        // creation of OpenAL source failed, giving up
        if (!m_ok) return;
    }

    alSourcePlay(m_soundSource);
    SFXManager::checkError("resuming");
}   // resume

//-----------------------------------------------------------------------------
/** This actually queues up the sfx in the sfx manager. It will be started
 *  from a separate thread later (in this frame).
 */
void SFXOpenAL::play()
{
    SFXManager::get()->queue(this);
}   // play

//-----------------------------------------------------------------------------
/** Plays this sound effect.
 */
void SFXOpenAL::reallyPlayNow()
{
    if (!SFXManager::get()->sfxAllowed()) return;
    if (!m_ok)
    {
        // lazily create OpenAL source when needed
        init();

        // creation of OpenAL source failed, giving up
        if (!m_ok) return;
    }

    alSourcePlay(m_soundSource);
    SFXManager::checkError("playing");
}   // reallyPlayNow

//-----------------------------------------------------------------------------
/** Sets the position where this sound effects is played.
 *  \param position Position of the sound effect.
 */
void SFXOpenAL::position(const Vec3 &position)
{
    if(!UserConfigParams::m_sfx)
        return;
    if (!m_ok)
    {
        Log::warn("SFX", "Position called on non-ok SFX <%s>", m_soundBuffer->getFileName().c_str());
        return;
    }
    if (!m_positional)
    {
        // in multiplayer, all sounds are positional, so in this case don't bug users with
        // an error message if (race_manager->getNumLocalPlayers() > 1)
        // (note that 0 players is also possible, in cutscenes)
        if (race_manager->getNumLocalPlayers() < 2)
        {
            Log::warn("SFX", "Position called on non-positional SFX");
        }
        return;
    }

    alSource3f(m_soundSource, AL_POSITION,
               (float)position.getX(), (float)position.getY(), (float)position.getZ());

    if (SFXManager::get()->getListenerPos().distance(position) > m_soundBuffer->getMaxDist())
    {
        alSourcef(m_soundSource, AL_GAIN, 0);
    }
    else
    {
        alSourcef(m_soundSource, AL_GAIN, (m_gain < 0.0f ? m_defaultGain : m_gain) * m_master_gain);
    }

    SFXManager::checkError("positioning");
}   // position

//-----------------------------------------------------------------------------
/** Returns the status of this sound effect.
 */
SFXManager::SFXStatus SFXOpenAL::getStatus()
{
    if(!m_ok) return SFXManager::SFX_UNKNOWN;

    int state = 0;
    alGetSourcei(m_soundSource, AL_SOURCE_STATE, &state);
    switch(state)
    {
    case AL_STOPPED: return SFXManager::SFX_STOPPED;
    case AL_PLAYING: return SFXManager::SFX_PLAYING;
    case AL_PAUSED:  return SFXManager::SFX_PAUSED;
    case AL_INITIAL: return SFXManager::SFX_INITIAL;
    default:         return SFXManager::SFX_UNKNOWN;
    }
}   // getStatus

//-----------------------------------------------------------------------------

void SFXOpenAL::onSoundEnabledBack()
{
    if (m_loop)
    {
        if (!m_ok) init();
        if (m_ok)
        {
            alSourcef(m_soundSource, AL_GAIN, 0);
            play();
            pause();
            alSourcef(m_soundSource, AL_GAIN, (m_gain < 0.0f ? m_defaultGain : m_gain) * m_master_gain);
        }
    }
}

//-----------------------------------------------------------------------------

void SFXOpenAL::setRolloff(float rolloff)
{
    alSourcef (m_soundSource, AL_ROLLOFF_FACTOR,  rolloff);
}

#endif //if HAVE_OGGVORBIS
