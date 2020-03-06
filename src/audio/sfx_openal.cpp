//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 Joerg Henrichs
//  Copyright (C) 2006-2015 Patrick Ammann <pammann@aro.ch>
//  Copyright (C) 2009-2015 Marianne Gagnon
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

#ifdef ENABLE_SOUND

#include "audio/sfx_openal.hpp"

#include "audio/sfx_buffer.hpp"
#include "config/user_config.hpp"
#include "modes/world.hpp"
#include "utils/vs.hpp"

#include <assert.h>
#include <cmath>
#include <stdio.h>
#include <string>

SFXOpenAL::SFXOpenAL(SFXBuffer* buffer, bool positional, float volume, 
                     bool owns_buffer) 
         : SFXBase()
{
    m_sound_buffer = buffer;
    m_sound_source = 0;
    m_status       = SFX_NOT_INITIALISED;
    m_positional   = positional;
    m_default_gain = volume;
    m_loop         = false;
    m_gain         = -1.0f;
    m_master_gain  = 1.0f;
    m_owns_buffer  = owns_buffer;
    m_play_time    = 0.0f;

    // Don't initialise anything else if the sfx manager was not correctly
    // initialised. First of all the initialisation will not work, and it
    // will not be used anyway.
    if (SFXManager::get()->sfxAllowed())
    {
        SFXManager::get()->queue(SFXManager::SFX_CREATE_SOURCE, this);
    }
}   // SFXOpenAL

//-----------------------------------------------------------------------------
/** Deletes the sfx source, and if it owns the buffer, also deletes the sound
 *  buffer. */
SFXOpenAL::~SFXOpenAL()
{
    if (m_status!=SFX_UNKNOWN && m_status!=SFX_NOT_INITIALISED)
    {
        alDeleteSources(1, &m_sound_source);
        SFXManager::checkError("deleting a source");
    }

    if (m_owns_buffer && m_sound_buffer)
    {
        m_sound_buffer->unload();
        delete m_sound_buffer;
    }
}   // ~SFXOpenAL

//-----------------------------------------------------------------------------
/** Initialises the sfx. 
 */
bool SFXOpenAL::init()
{
    m_status = SFX_UNKNOWN;

    alGenSources(1, &m_sound_source );
    if (!SFXManager::checkError("generating a source"))
        return false;

    assert( alIsBuffer(m_sound_buffer->getBufferID()) );
    assert( alIsSource(m_sound_source) );

    alSourcei (m_sound_source, AL_BUFFER, m_sound_buffer->getBufferID());

    if (!SFXManager::checkError("attaching the buffer to the source"))
        return false;

    alSource3f(m_sound_source, AL_POSITION,       0.0, 0.0, 0.0);
    alSource3f(m_sound_source, AL_VELOCITY,       0.0, 0.0, 0.0);
    alSource3f(m_sound_source, AL_DIRECTION,      0.0, 0.0, 0.0);

    alSourcef (m_sound_source, AL_ROLLOFF_FACTOR, m_sound_buffer->getRolloff());
    alSourcef (m_sound_source, AL_MAX_DISTANCE,   m_sound_buffer->getMaxDist());

    if (m_gain < 0.0f)
    {
        alSourcef (m_sound_source, AL_GAIN, m_default_gain * m_master_gain);
    }
    else
    {
        alSourcef (m_sound_source, AL_GAIN, m_gain * m_master_gain);
    }

    if (m_positional) alSourcei (m_sound_source, AL_SOURCE_RELATIVE, AL_FALSE);
    else              alSourcei (m_sound_source, AL_SOURCE_RELATIVE, AL_TRUE);

    alSourcei(m_sound_source, AL_LOOPING, m_loop ? AL_TRUE : AL_FALSE);

    if(!SFXManager::checkError("setting up the source"))
        return false;

    m_status = SFX_STOPPED;
    return true;
}   // init

// ------------------------------------------------------------------------
/** Updates the status of a playing sfx. If the sound has been played long
 *  enough, mark it to be finished. This avoid (a potentially costly)
 *  call to openal.
 *  \param dt Time step size.
 */
void SFXOpenAL::updatePlayingSFX(float dt)
{
    assert(m_status==SFX_PLAYING);
    m_play_time += dt;
    if(!m_loop && m_play_time > m_sound_buffer->getDuration())
        m_status = SFX_STOPPED;
}   // updatePlayingSFX

//-----------------------------------------------------------------------------
/** Queues up a change of the pitch of a sound effect to the sfx manager.
 *  \param factor Speedup/slowdown between 0.5 and 2.0
 */
void SFXOpenAL::setSpeed(float factor)
{
    //if(m_status!=SFX_PLAYING || !SFXManager::get()->sfxAllowed()) return;
    assert(!std::isnan(factor));
    SFXManager::get()->queue(SFXManager::SFX_SPEED, this, factor);
}   // setSpeed

//-----------------------------------------------------------------------------
/** Changes the pitch of a sound effect. Executed from the sfx manager thread.
 *  \param factor Speedup/slowdown between 0.5 and 2.0
 */
void SFXOpenAL::reallySetSpeed(float factor)
{
    if (m_status != SFX_PLAYING || !SFXManager::get()->sfxAllowed()) return;

    if(m_status==SFX_NOT_INITIALISED)
    {
        init();
        if(m_status==SFX_UNKNOWN)
            return;
    }

    //OpenAL only accepts pitches in the range of 0.5 to 2.0
    if(factor > 2.0f)
    {
        factor = 2.0f;
    }
    if(factor < 0.5f)
    {
        factor = 0.5f;
    }
    alSourcef(m_sound_source,AL_PITCH,factor);
    SFXManager::checkError("setting speed");
}   // reallySetSpeed

//-----------------------------------------------------------------------------
/** Changes the volume of a sound effect.
 *  \param volume Volume adjustment between 0.0 (mute) and 1.0 (full volume).
 */
void SFXOpenAL::setVolume(float volume)
{
    if(m_status==SFX_UNKNOWN || !SFXManager::get()->sfxAllowed()) return;
    assert(!std::isnan(volume)) ;
    SFXManager::get()->queue(SFXManager::SFX_VOLUME, this, volume);
}   // setVolume

//-----------------------------------------------------------------------------
/** Changes the volume of a sound effect.
 *  \param volume Volume adjustment between 0.0 (mute) and 1.0 (full volume).
 */
void SFXOpenAL::reallySetVolume(float volume)
{
    m_gain = m_default_gain * volume;

    if(m_status==SFX_UNKNOWN) return;

    if(m_status==SFX_NOT_INITIALISED)
    {
        init();
        if(m_status==SFX_UNKNOWN)
            return;
    }

    alSourcef(m_sound_source, AL_GAIN, m_gain * m_master_gain);
}   // reallySetVolume

//-----------------------------------------------------------------------------
/** Schedules setting of the master volume.
 *  \param volume Volume value.
 */
void SFXOpenAL::setMasterVolume(float volume)
{
    // This needs to be called even if sfx are disabled atm, so only exit
    // in case that the sfx could not be loaded in the first place.
    if(m_status==SFX_UNKNOWN) return;
    SFXManager::get()->queue(SFXManager::SFX_MASTER_VOLUME, this, volume);
}   // setMasterVolume

//-----------------------------------------------------------------------------
/** Sets the master volume.
 *  \param volume Master volume.
 */
void SFXOpenAL::reallySetMasterVolumeNow(float volume)
{
    m_master_gain = volume;
    
    if(m_status==SFX_UNKNOWN || m_status == SFX_NOT_INITIALISED) return;

    alSourcef(m_sound_source, AL_GAIN, 
               (m_gain < 0.0f ? m_default_gain : m_gain) * m_master_gain);
    SFXManager::checkError("setting volume");
}   // reallySetMasterVolumeNow

//-----------------------------------------------------------------------------
/** Loops this sound effect.
 */
void SFXOpenAL::setLoop(bool status)
{
    // Set the flag (even if sfx are disabled), so that the right settings
    // are available on restart.
    m_loop = status;
    if (m_status == SFX_UNKNOWN || !SFXManager::get()->sfxAllowed()) return;
    SFXManager::get()->queue(SFXManager::SFX_LOOP, this, status ? 1.0f : 0.0f);
}   // setLoop

//-----------------------------------------------------------------------------
/** Loops this sound effect.
 */
void SFXOpenAL::reallySetLoop(bool status)
{
    if(m_status==SFX_NOT_INITIALISED)
    {
        init();
        if(m_status==SFX_UNKNOWN)
            return;
    }

    alSourcei(m_sound_source, AL_LOOPING, status ? AL_TRUE : AL_FALSE);
    SFXManager::checkError("looping");
}   // reallySetLoop

//-----------------------------------------------------------------------------
/** Queues a stop for this effect to the sound manager.
 */
void SFXOpenAL::stop()
{
    if (m_status == SFX_UNKNOWN || !SFXManager::get()->sfxAllowed()) return;
    SFXManager::get()->queue(SFXManager::SFX_STOP, this);
}   // stop

//-----------------------------------------------------------------------------
/** The sfx manager thread executes a stop for this sfx.
 */
void SFXOpenAL::reallyStopNow()
{
    if(m_status==SFX_PLAYING || m_status==SFX_PAUSED)
    {
        m_status = SFX_STOPPED;
        m_loop = false;
        alSourcei(m_sound_source, AL_LOOPING, AL_FALSE);
        alSourceStop(m_sound_source);
        SFXManager::checkError("stopping");
    }
}   // reallyStopNow

//-----------------------------------------------------------------------------
/** Queues up a pause command for this sfx.
 */
void SFXOpenAL::pause()
{
    SFXManager::get()->queue(SFXManager::SFX_PAUSE, this);
}   // pause

//-----------------------------------------------------------------------------
/** Pauses a SFX that's currently played. Nothing happens it the effect is
 *  currently not being played.
 */
void SFXOpenAL::reallyPauseNow()
{
    // Need to be tested again here, since this function can be called
    // from pauseAll, and we have to make sure to only pause playing sfx.
    if (m_status != SFX_PLAYING || !SFXManager::get()->sfxAllowed()) return;
    m_status = SFX_PAUSED;
    alSourcePause(m_sound_source);
    SFXManager::checkError("pausing");
}   // reallyPauseNow

//-----------------------------------------------------------------------------
/** Queues up a resume command for this sound effect.
 */
void SFXOpenAL::resume()
{
    //if (m_status != SFX_PLAYING || !SFXManager::get()->sfxAllowed()) return;
    if (!SFXManager::get()->sfxAllowed()) return;
    SFXManager::get()->queue(SFXManager::SFX_RESUME, this);
}   // resume

//-----------------------------------------------------------------------------
/** Resumes a sound effect.
 */
void SFXOpenAL::reallyResumeNow()
{
    if(m_status==SFX_NOT_INITIALISED)
    {
        init();
        if(m_status==SFX_UNKNOWN)
            return;
    }

    if(m_status==SFX_PAUSED)
    {
        alSourcePlay(m_sound_source);
        SFXManager::checkError("resuming");
        m_status = SFX_PLAYING;
    }
}   // reallyResumeNow

//-----------------------------------------------------------------------------
/** This actually queues up the sfx in the sfx manager. It will be started
 *  from a separate thread later (in this frame).
 */
void SFXOpenAL::play()
{
    if (m_status == SFX_UNKNOWN || !SFXManager::get()->sfxAllowed()) return;

    if(m_status==SFX_STOPPED || m_status==SFX_NOT_INITIALISED)
        m_play_time = 0.0f;

    // Technically the sfx is only playing after the sfx thread starts it,
    // but it is important to set this here since stk might decide to
    // delete a sfx if it has finished playing (i.e. is in stopped state)
    // - which can happen if the sfx thread had no time to actually start
    // it yet.
    m_status = SFX_PLAYING;
    SFXManager::get()->queue(SFXManager::SFX_PLAY, this);
}   // play

//-----------------------------------------------------------------------------
/** Plays this sound effect.
 */
void SFXOpenAL::reallyPlayNow(SFXBuffer* buffer)
{
    if (!SFXManager::get()->sfxAllowed()) return;
    if (m_status==SFX_NOT_INITIALISED)
    {
        // lazily create OpenAL source when needed
        init();

        // creation of OpenAL source failed, giving up
        if (m_status==SFX_UNKNOWN) return;
    }

    if (buffer != NULL)
    {
        if (m_status == SFX_PLAYING || m_status == SFX_PAUSED)
            reallyStopNow();

        m_sound_buffer = buffer;
        alSourcei(m_sound_source, AL_BUFFER, m_sound_buffer->getBufferID());

        if (!SFXManager::checkError("attaching the buffer to the source"))
            return;
    }

    alSourcePlay(m_sound_source);
    SFXManager::checkError("playing");
    // Esp. with terrain sounds it can (very likely) happen that the status
    // got overwritten: a sound is created and an init event is queued. Then
    // a play event is queued, and the status is immediately changed to
    // playing. But when now the init event is executed, the status is set
    // to stopped again. So for this case we have to set the status to
    // playing again.
    m_status = SFX_PLAYING;
}   // reallyPlayNow

//-----------------------------------------------------------------------------
/** This actually queues up the sfx in the sfx manager. It will be started
 *  from a separate thread later (in this frame).
 */
void SFXOpenAL::play(const Vec3 &position, SFXBuffer* buffer)
{
    if (m_owns_buffer && buffer != NULL)
        assert(false); // sources that own a buffer cannot play any other buffer

    if (m_status == SFX_UNKNOWN || !SFXManager::get()->sfxAllowed()) return;

    if(m_status==SFX_STOPPED || m_status==SFX_NOT_INITIALISED)
        m_play_time = 0.0f;

    // Technically the sfx is only playing after the sfx thread starts it,
    // but it is important to set this here since stk might decide to
    // delete a sfx if it has finished playing (i.e. is in stopped state)
    // - which can happen if the sfx thread had no time to actually start
    // it yet.
    m_status = SFX_PLAYING;
    SFXManager::get()->queue(SFXManager::SFX_PLAY_POSITION, this, position, buffer);
}   // play(Vec3)
//-----------------------------------------------------------------------------
/** Plays this sound effect.
 */
void SFXOpenAL::reallyPlayNow(const Vec3 &position, SFXBuffer* buffer)
{
    reallySetPosition(position);
    reallyPlayNow(buffer);
}   // reallyPlayNow(Vec3)

//-----------------------------------------------------------------------------
/** Sets the position where this sound effects is played.
 *  \param position Position of the sound effect.
 */
void SFXOpenAL::setPosition(const Vec3 &position)
{
    // Don't send a position command to the thread if the sound is not playing
    // (or sfx disabled or the sound was not initialised correctly)
    if (!SFXManager::get()->sfxAllowed()) return;
    SFXManager::get()->queue(SFXManager::SFX_POSITION, this, position);

}   // setPosition

//-----------------------------------------------------------------------------
/** Sets the position where this sound effects is played.
 *  \param position Position of the sound effect.
 */
void SFXOpenAL::reallySetPosition(const Vec3 &position)
{
    if(m_status==SFX_NOT_INITIALISED)
    {
        init();
        if(m_status==SFX_UNKNOWN)
            return;
    }

    if (!m_positional)
    {
        // in multiplayer, all sounds are positional, so in this case don't
        // bug users with an error message if (note that 0 players is also
        // possible, in cutscenes)
        if (RaceManager::get()->getNumLocalPlayers() < 2)
        {
            Log::warn("SFX", "Position called on non-positional SFX");
        }
        return;
    }

    alSource3f(m_sound_source, AL_POSITION, position.getX(),
               position.getY(), -position.getZ());

    if (SFXManager::get()->getListenerPos().distance(position)
        > m_sound_buffer->getMaxDist())
    {
        alSourcef(m_sound_source, AL_GAIN, 0);
    }
    else
    {
        alSourcef(m_sound_source, AL_GAIN, 
                  (m_gain < 0.0f ? m_default_gain : m_gain) * m_master_gain);
    }

    SFXManager::checkError("positioning");
}   // reallySetPosition

// ----------------------------------------------------------------------------
/** Shortcut that plays at a specified position. */
void SFXOpenAL::setSpeedPosition(float factor, const Vec3 &position)
{
    //if (m_status != SFX_PLAYING || !SFXManager::get()->sfxAllowed()) return;
    if (!SFXManager::get()->sfxAllowed()) return;

    SFXManager::get()->queue(SFXManager::SFX_SPEED_POSITION, this,
                             factor, position);
}   // play(vec3)
//-----------------------------------------------------------------------------
/** Set speed and position of a sound effect.
 *  \param f Speed.
 *  \param position Position of the sfx.
 */
void SFXOpenAL::reallySetSpeedPosition(float f, const Vec3 &position)
{
    if (m_status != SFX_PLAYING || !SFXManager::get()->sfxAllowed()) return;
    reallySetSpeed(f);
    reallySetPosition(position);
}   // reallySetSpeedPosition
//-----------------------------------------------------------------------------
/** Queues up a delete request for this object. This is necessary to avoid
 *  a crash if the sfx manager thread might be delayed and access this object
 *  after it was deleted.
 */
void SFXOpenAL::deleteSFX()
{
    SFXManager::get()->queue(SFXManager::SFX_DELETE, this);
}   // deleteSFX

//-----------------------------------------------------------------------------

void SFXOpenAL::onSoundEnabledBack()
{
    if (m_loop)
    {
        if (m_status==SFX_NOT_INITIALISED) init();
        if (m_status!=SFX_UNKNOWN)
        {
            alSourcef(m_sound_source, AL_GAIN, 0);
            play();
            pause();
            alSourcef(m_sound_source, AL_GAIN,
                     (m_gain < 0.0f ? m_default_gain : m_gain) * m_master_gain);
        }
    }
}   // onSoundEnabledBack

//-----------------------------------------------------------------------------

void SFXOpenAL::setRolloff(float rolloff)
{
    alSourcef (m_sound_source, AL_ROLLOFF_FACTOR,  rolloff);
}

//-----------------------------------------------------------------------------

#endif //ifdef ENABLE_SOUND
