//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2015 Patrick Ammann <pammann@aro.ch>
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

#ifdef ENABLE_SOUND

#include <assert.h>
#include <atomic>
#include <AL/al.h>
#include "audio/sfx_base.hpp"
#include "utils/leak_check.hpp"
#include "utils/cpp2011.hpp"

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
    std::atomic<SFXStatus> m_status;

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
              SFXOpenAL(SFXBuffer* buffer, bool positional, float volume,
                        bool owns_buffer = false);
    virtual  ~SFXOpenAL();

    virtual void      updatePlayingSFX(float dt) OVERRIDE;
    virtual bool      init() OVERRIDE;
    virtual void      play() OVERRIDE;
    virtual void      reallyPlayNow(SFXBuffer* buffer = NULL) OVERRIDE;
    virtual void      play(const Vec3 &xyz, SFXBuffer* buffer = NULL) OVERRIDE;
    virtual void      reallyPlayNow(const Vec3 &xyz, SFXBuffer* buffer = NULL) OVERRIDE;
    virtual void      setLoop(bool status) OVERRIDE;
    virtual void      reallySetLoop(bool status) OVERRIDE;
    virtual void      stop() OVERRIDE;
    virtual void      reallyStopNow() OVERRIDE;
    virtual void      pause() OVERRIDE;
    virtual void      reallyPauseNow() OVERRIDE;
    virtual void      resume() OVERRIDE;
    virtual void      reallyResumeNow() OVERRIDE;
    virtual void      deleteSFX() OVERRIDE;
    virtual void      setSpeed(float factor) OVERRIDE;
    virtual void      reallySetSpeed(float factor) OVERRIDE;
    virtual void      setPosition(const Vec3 &position) OVERRIDE;
    virtual void      reallySetPosition(const Vec3 &p) OVERRIDE;
    virtual void      setSpeedPosition(float factor, const Vec3 &p) OVERRIDE;
    virtual void      reallySetSpeedPosition(float f,const Vec3 &p) OVERRIDE;
    virtual void      setVolume(float volume) OVERRIDE;
    virtual void      reallySetVolume(float volume) OVERRIDE;
    virtual void      setMasterVolume(float volume) OVERRIDE;
    virtual void      reallySetMasterVolumeNow(float volue) OVERRIDE;
    virtual void      onSoundEnabledBack() OVERRIDE;
    virtual void      setRolloff(float rolloff) OVERRIDE;
    // ------------------------------------------------------------------------
    /** Returns if this sfx is looped or not. */
    virtual bool      isLooped()  OVERRIDE { return m_loop; }
    // ------------------------------------------------------------------------
    /** Returns the status of this sfx. */
    virtual SFXStatus getStatus()  OVERRIDE { return m_status; }

    // ------------------------------------------------------------------------
    /** Returns the buffer associated with this sfx. */
    virtual SFXBuffer* getBuffer() const OVERRIDE
                                                     { return m_sound_buffer; }

};   // SFXOpenAL

#endif
#endif // HEADER_SFX_OPENAL_HPP

