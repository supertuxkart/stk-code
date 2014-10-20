//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2013 Patrick Ammann <pammann@aro.ch>
//  Copyright (C) 2008-2013 Joerg Henrichs, Patrick Ammann
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

#ifndef HEADER_SFX_HPP
#define HEADER_SFX_HPP

#include "audio/sfx_manager.hpp"
#include "utils/no_copy.hpp"

/**
 * \defgroup audio
 * This module handles audio (sound effects and music).
 */

class Vec3;

/**
 * \brief The base class for sound effects.
 *  It gets a sound buffer from the sound
 *  manager, which is shared between all instances. Do create a new sound
 *  effect object, use sfx_manager->getSFX(...); do not create an instance
 *  with new, since SFXManager makes sure to stop/restart all SFX (esp.
 *  looping sfx like engine sounds) when necessary.
 * \ingroup audio
 */
class SFXBase : public NoCopy
{
public:
    /** Status of a sound effect. */
    enum SFXStatus
    {
        SFX_UNKNOWN = -1, SFX_STOPPED = 0, SFX_PAUSED = 1, SFX_PLAYING = 2,
        SFX_NOT_INITIALISED = 3
    };

    virtual           ~SFXBase()  {}

    /** Late creation, if SFX was initially disabled */
    virtual bool       init()                               = 0;
    virtual bool       isLooped()                           = 0;
    virtual void       updatePlayingSFX(float dt)           = 0;
    virtual void       setPosition(const Vec3 &p)           = 0;
    virtual void       reallySetPosition(const Vec3 &p)     = 0;
    virtual void       setLoop(bool status)                 = 0;
    virtual void       reallySetLoop(bool status)           = 0;
    virtual void       play()                               = 0;
    virtual void       reallyPlayNow()                      = 0;
    virtual void       stop()                               = 0;
    virtual void       reallyStopNow()                      = 0;
    virtual void       pause()                              = 0;
    virtual void       reallyPauseNow()                     = 0;
    virtual void       resume()                             = 0;
    virtual void       reallyResumeNow()                    = 0;
    virtual void       deleteSFX()                          = 0;
    virtual void       setSpeed(float factor)               = 0;
    virtual void       reallySetSpeed(float factor)         = 0;
    virtual void       setVolume(float gain)                = 0;
    virtual void       reallySetVolume(float gain)          = 0;
    virtual void       setMasterVolume(float gain)          = 0;
    virtual void       reallySetMasterVolumeNow(float gain) = 0;
    virtual void       onSoundEnabledBack()                 = 0;
    virtual void       setRolloff(float rolloff)            = 0;
    virtual const SFXBuffer* getBuffer() const              = 0;
    virtual SFXStatus  getStatus()                          = 0;

};   // SFXBase


#endif // HEADER_SFX_HPP

