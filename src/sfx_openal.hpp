//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Patrick Ammann <pammann@aro.ch>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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

#ifndef HEADER_SFX_OPENAL_H
#define HEADER_SFX_OPENAL_H

#include <assert.h>
#include <AL/al.h>

#include "sfx.hpp"


class SFXImpl : public SFX
{
public:
    SFXImpl(const char* filename);
    virtual ~SFXImpl();

    virtual void play();

private:
    bool load(const char* filename);

    ALuint m_soundBuffer;   // Buffers hold sound data.
    ALuint m_soundSource;   // Sources are points emitting sound.
};

#endif // HEADER_SFX_OPENAL_H

