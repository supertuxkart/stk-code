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

#if (HAVE_OPENAL && (HAVE_MIKMOD || HAVE_OGGVORBIS))

#include <assert.h>
#include <stdio.h>
#include <string>

#ifdef __APPLE__
#  include <OpenAL/al.h>
#else
#  include <AL/al.h>
#endif
#include <AL/alut.h>

#include "sfx_openal.hpp"
#include "loader.hpp"



SFXImpl::SFXImpl(const char* filename)
{
    m_soundBuffer= 0;
    m_soundSource= 0;
    assert(load(filename));
}

//-----------------------------------------------------------------------------
SFXImpl::~SFXImpl()
{
    alDeleteBuffers(1, &m_soundBuffer);
    alDeleteSources(1, &m_soundSource);
}

//-----------------------------------------------------------------------------
void SFXImpl::play()
{
    alSourcePlay(m_soundSource);
}

//-----------------------------------------------------------------------------
bool SFXImpl::load(const char* filename)
{
    std::string path=  loader->getPath(filename);

    alGenBuffers(1, &m_soundBuffer);
    if (alGetError() != AL_NO_ERROR) 
    {
        fprintf(stderr, "Loading '%s' failed\n",filename); 
        return false; 
    }
    ALenum format= 0;
    ALsizei size= 0, freq= 0;
    ALvoid* data= NULL;
    ALboolean loop= AL_FALSE;

#ifdef __APPLE__
    alutLoadWAVFile((ALbyte*)path.c_str(), &format, &data, &size, &freq);
#else
    alutLoadWAVFile((ALbyte*)path.c_str(), &format, &data, &size, &freq, &loop);
#endif

    if (data == NULL)
    {
        fprintf(stderr, "Error 1 loading SFX: %s failed\n", path.c_str());
        return false;
    }

    alBufferData(m_soundBuffer, format, data, size, freq);
    if (alGetError() != AL_NO_ERROR)
    {
        fprintf(stderr, "Error 2 loading SFX: %s failed\n", path.c_str());
        return false;
    }

    alutUnloadWAV(format, data, size, freq);
    if (alGetError() != AL_NO_ERROR)
    {
        fprintf(stderr, "Error 3 loading SFX: %s failed\n", path.c_str());
        return false;
    }

    // Bind buffer with a source.
    alGenSources(1, &m_soundSource);
    if (alGetError() != AL_NO_ERROR)
    {
        fprintf(stderr, "Error 4 loading SFX: %s failed\n", path.c_str());
        return false;
    }

    // not 3D yet
    alSourcei (m_soundSource, AL_BUFFER,          m_soundBuffer);
    alSource3f(m_soundSource, AL_POSITION,        0.0, 0.0, 0.0);
    alSource3f(m_soundSource, AL_VELOCITY,        0.0, 0.0, 0.0);
    alSource3f(m_soundSource, AL_DIRECTION,       0.0, 0.0, 0.0);
    alSourcef (m_soundSource, AL_ROLLOFF_FACTOR,  0.0          );
    alSourcei (m_soundSource, AL_SOURCE_RELATIVE, AL_TRUE      );

    return true;
}

#endif //if (HAVE_OPENAL && (HAVE_MIKMOD || HAVE_OGGVORBIS))

