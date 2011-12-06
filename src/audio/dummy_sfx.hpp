//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Patrick Ammann <pammann@aro.ch>
//  Copyright (C) 2008 Joerg Henrichs, Patrick Ammann
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
    virtual           ~DummySFX()                     {}
    
    /** Late creation, if SFX was initially disabled */
    virtual bool       init() { return true; }
    
    virtual void       position(const Vec3 &position) {}
    virtual void       setLoop(bool status)           {}
    virtual void       play()                         {}
    virtual void       stop()                         {}
    virtual void       pause()                        {}
    virtual void       resume()                       {}
    virtual void       speed(float factor)            {}
    virtual void       volume(float gain)             {}
    virtual SFXManager::SFXStatus  getStatus()        { return SFXManager::SFX_STOPPED; }
    virtual void       onSoundEnabledBack()           {}
    virtual void       setRolloff(float rolloff)      {}
    
    virtual const SFXBuffer* getBuffer()              { return NULL; }
    
};   // DummySFX


#endif // HEADER_SFX_HPP

