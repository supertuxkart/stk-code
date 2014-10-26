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

#ifndef HEADER_DUMMY_SFX_HPP
#define HEADER_DUMMY_SFX_HPP

#include "audio/sfx_base.hpp"


/**
 * \brief Dummy sound when ogg or openal aren't available
 * \ingroup audio
 */
class DummySFX : public SFXBase
{
public:
                       DummySFX(SFXBuffer* buffer, bool positional,
                                float gain) {}
    virtual           ~DummySFX()                     {}

    /** Late creation, if SFX was initially disabled */
    virtual bool       init()                           { return true;  }
    virtual bool       isLooped()                       { return false; }
    virtual void       updatePlayingSFX(float dt)       {}
    virtual void       setLoop(bool status)             {}
    virtual void       reallySetLoop(bool status)       {}
    virtual void       setPosition(const Vec3 &p)       {}
    virtual void       reallySetPosition(const Vec3 &p) {}
    virtual void       play()                           {}
    virtual void       reallyPlayNow()                  {}
    virtual void       stop()                           {}
    virtual void       reallyStopNow()                  {}
    virtual void       pause()                          {}
    virtual void       reallyPauseNow()                 {}
    virtual void       resume()                         {}
    virtual void       reallyResumeNow()                {}
    virtual void       deleteSFX()                      { delete this; }
    virtual void       setSpeed(float factor)           {}
    virtual void       reallySetSpeed(float factor)     {}
    virtual void       setVolume(float gain)            {}
    virtual void       reallySetVolume(float gain)      {}
    virtual void       setMasterVolume(float gain)      {}
    virtual void       reallySetMasterVolumeNow(float gain) {}
    virtual SFXStatus  getStatus()                      { return SFX_STOPPED; }
    virtual void       onSoundEnabledBack()             {}
    virtual void       setRolloff(float rolloff)        {}
    virtual const SFXBuffer* getBuffer() const          { return NULL; }

};   // DummySFX


#endif // HEADER_SFX_HPP

