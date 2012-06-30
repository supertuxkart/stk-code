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
#include "audio/sfx_manager.hpp"

/**
  * \brief OpenAL implementation of the abstract SFXBase interface
  * \ingroup audio
  */
class SFXOpenAL : public SFXBase
{
private:
    SFXBuffer*   m_soundBuffer;   //!< Buffers hold sound data.
    ALuint       m_soundSource;   //!< Sources are points emitting sound.
    bool         m_ok;
    bool         m_positional;
    float        m_defaultGain;
    
    int          m_rolloffType;
    
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
    
public:
                                  SFXOpenAL(SFXBuffer* buffer, bool positional, float gain);
    virtual                      ~SFXOpenAL();
    
    /** Late creation, if SFX was initially disabled */
    virtual bool                  init();
    
    virtual void                  play();
    virtual void                  setLoop(bool status);
    virtual void                  stop();
    virtual void                  pause();
    virtual void                  resume();
    virtual void                  speed(float factor);
    virtual void                  position(const Vec3 &position);
    virtual void                  volume(float gain);
    virtual SFXManager::SFXStatus getStatus();
    virtual void                  onSoundEnabledBack();
    virtual void                  setRolloff(float rolloff);

    virtual const SFXBuffer* getBuffer() const { return m_soundBuffer; }
    
};   // SFXOpenAL

#endif
#endif // HEADER_SFX_OPENAL_HPP

