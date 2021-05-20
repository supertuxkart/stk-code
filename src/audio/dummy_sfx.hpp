//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2015 Patrick Ammann <pammann@aro.ch>
//  Copyright (C) 2008-2015 Joerg Henrichs, Patrick Ammann
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
#include "utils/cpp2011.hpp"

/**
 * \brief Dummy sound when ogg or openal aren't available
 * \ingroup audio
 */
class DummySFX : public SFXBase
{
public:
                       DummySFX(SFXBuffer* buffer, bool positional,
                                float gain) {}
    virtual           ~DummySFX() {}

    /** Late creation, if SFX was initially disabled */
    virtual bool       init() OVERRIDE { return true;  }
    virtual bool       isLooped() OVERRIDE { return false; }
    virtual void       updatePlayingSFX(float dt) OVERRIDE {}
    virtual void       setLoop(bool status) OVERRIDE {}
    virtual void       reallySetLoop(bool status) OVERRIDE {}
    virtual void       setPosition(const Vec3 &p) OVERRIDE {}
    virtual void       reallySetPosition(const Vec3 &p) OVERRIDE {}
    virtual void       setSpeedPosition(float factor,
                                        const Vec3 &p) OVERRIDE {}
    virtual void       reallySetSpeedPosition(float f,
                                         const Vec3 &p) OVERRIDE {}
    virtual void       play() OVERRIDE {}
    virtual void       reallyPlayNow(SFXBuffer* buffer = NULL) OVERRIDE {}
    virtual void       play(const Vec3 &xyz, SFXBuffer* buffer = NULL) OVERRIDE {}
    virtual void       reallyPlayNow(const Vec3 &xyz, SFXBuffer* buffer = NULL) OVERRIDE {}
    virtual void       stop() OVERRIDE {}
    virtual void       reallyStopNow() OVERRIDE {}
    virtual void       pause() OVERRIDE {}
    virtual void       reallyPauseNow() OVERRIDE {}
    virtual void       resume() OVERRIDE {}
    virtual void       reallyResumeNow() OVERRIDE {}
    virtual void       deleteSFX() OVERRIDE {}
    virtual void       setSpeed(float factor) OVERRIDE {}
    virtual void       reallySetSpeed(float factor) OVERRIDE {}
    virtual void       setVolume(float gain) OVERRIDE {}
    virtual void       reallySetVolume(float gain) OVERRIDE {}
    virtual void       setMasterVolume(float gain) OVERRIDE {}
    virtual void       reallySetMasterVolumeNow(float gain) OVERRIDE {}
    virtual SFXStatus  getStatus() OVERRIDE { return SFX_STOPPED; }
    virtual void       onSoundEnabledBack() OVERRIDE {}
    virtual void       setRolloff(float rolloff) OVERRIDE {}
    virtual SFXBuffer* getBuffer() const OVERRIDE { return NULL; }

};   // DummySFX


#endif // HEADER_SFX_HPP

