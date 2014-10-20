//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2013 Patrick Ammann <pammann@aro.ch>
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

#ifndef HEADER_SFX_OPENAL_HPP
#define HEADER_SFX_OPENAL_HPP

#if HAVE_OGGVORBIS

#include <assert.h>
#ifdef __APPLE__
#  include <OpenAL/al.h>
#else
#  include <AL/al.h>
#endif
#include "audio/sfx_base.hpp"
#include "utils/leak_check.hpp"

/**
  * \brief OpenAL implementation of the abstract SFXBase interface
  * \ingroup audio
  */
class SFXOpenAL : public SFXBase
{
private:
    LEAK_CHECK()

    /** Buffers hold sound data. */
    SFXBuffer*   m_sound_buffer;

    /** Sources are points emitting sound. */
    ALuint       m_sound_source;

    /** The status of this SFX. */
    SFXStatus    m_status;

    /** If the sfx is positional. */
    bool         m_positional;

    /** Default gain value. */
    float        m_default_gain;

    /** The OpenAL source contains this info, but if audio is disabled initially then
        the sound source won't be created and we'll be left with no clue when enabling
        sounds later */
    bool m_loop;

    /** Contains a volume if set through the "volume" method, or a negative number if
     this method was not called.
     The OpenAL source contains this info, but if audio is disabled initially then
     the sound source won't be created and we'll be left with no clue when enabling
     sounds later. */
    float m_gain;
    
    /** The master gain set in user preferences */
    float m_master_gain;

    /** If this sfx should also free the sound buffer. */
    bool m_owns_buffer;

    /** How long the sfx has been playing. */
    float m_play_time;

public:
              SFXOpenAL(SFXBuffer* buffer, bool positional, float gain,
                        bool owns_buffer = false);
    virtual  ~SFXOpenAL();

    virtual void      updatePlayingSFX(float dt);
    virtual bool      init();
    virtual void      play();
    virtual void      reallyPlayNow();
    virtual void      setLoop(bool status);
    virtual void      reallySetLoop(bool status);
    virtual void      stop();
    virtual void      reallyStopNow();
    virtual void      pause();
    virtual void      reallyPauseNow();
    virtual void      resume();
    virtual void      reallyResumeNow();
    virtual void      deleteSFX();
    virtual void      setSpeed(float factor);
    virtual void      reallySetSpeed(float factor);
    virtual void      setPosition(const Vec3 &position);
    virtual void      reallySetPosition(const Vec3 &p);
    virtual void      setVolume(float gain);
    virtual void      reallySetVolume(float gain);
    virtual void      setMasterVolume(float gain);
    virtual void      reallySetMasterVolumeNow(float gain);
    virtual void      onSoundEnabledBack();
    virtual void      setRolloff(float rolloff);
    // ------------------------------------------------------------------------
    /** Returns if this sfx is looped or not. */
    virtual bool      isLooped() { return m_loop; }
    // ------------------------------------------------------------------------
    /** Returns the status of this sfx. */
    virtual SFXStatus getStatus() { return m_status; }

    // ------------------------------------------------------------------------
    /** Returns the buffer associated with this sfx. */
    virtual const SFXBuffer* getBuffer() const { return m_sound_buffer; }

};   // SFXOpenAL

#endif
#endif // HEADER_SFX_OPENAL_HPP

