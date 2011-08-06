//  $Id$
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

#ifndef HEADER_SFX_HPP
#define HEADER_SFX_HPP

#include "audio/sfx_manager.hpp"
#include "utils/no_copy.hpp"

/**
 * \defgroup audio
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
    virtual           ~SFXBase()                       {}
    
    /** Late creation, if SFX was initially disabled */
    virtual bool       init() = 0;
    
    virtual void       position(const Vec3 &position) = 0;
    virtual void       setLoop(bool status)      = 0;
    virtual void       play()                    = 0;
    virtual void       stop()                    = 0;
    virtual void       pause()                   = 0;
    virtual void       resume()                  = 0;
    virtual void       speed(float factor)       = 0;
    virtual void       volume(float gain)        = 0;
    virtual SFXManager::SFXStatus  
                       getStatus()               = 0;
    virtual void       onSoundEnabledBack()      = 0;
    virtual void       setRolloff(float rolloff) = 0;

    virtual const SFXBuffer* getBuffer() const = 0;

};   // SfxBase


#endif // HEADER_SFX_HPP

