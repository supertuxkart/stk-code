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

#if (HAVE_OPENAL && HAVE_OGGVORBIS)

#include <assert.h>
#include <stdio.h>
#include <string>

#ifdef __APPLE__
#  include <OpenAL/al.h>
#else
#  include <AL/al.h>
#endif

#include <SDL/SDL.h>
#include <SDL/SDL_endian.h>

#include "sfx_openal.hpp"
#include "file_manager.hpp"
#include "user_config.hpp"

SFXImpl::SFXImpl(const char* filename)
{
    m_soundBuffer= 0;
    m_soundSource= 0;
    const bool LOADED = load(filename);
    assert( LOADED );
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
    alSourcef(m_soundSource,AL_GAIN,user_config->m_sfx_volume);
    alSourcePlay(m_soundSource);

    // Check (and clear) the error flag
    int error = alGetError();

    if(error != AL_NO_ERROR)
    {
        fprintf(stderr, "SFX OpenAL error: %d\n", error);
    }

}

//-----------------------------------------------------------------------------
bool SFXImpl::load(const char* filename)
{
    std::string path = file_manager->getSFXFile(filename);

    alGenBuffers(1, &m_soundBuffer);
    if (alGetError() != AL_NO_ERROR)
    {
        fprintf(stderr, "Loading '%s' failed\n",filename);
        return false;
    }
    ALenum format = 0;
    Uint32 size = 0;
    Uint8* data = NULL;
    SDL_AudioSpec spec;

    if( SDL_LoadWAV( path.c_str(), &spec, &data, &size ) == NULL)
    {
        fprintf(stderr, "Error 1 loading SFX: with file %s, SDL_LoadWAV() failed\n", path.c_str());
        return false;
    }

    switch( spec.format )
    {
        case AUDIO_U8:
        case AUDIO_S8:
            if( spec.channels == 2 ) format = AL_FORMAT_STEREO8;
            else format = AL_FORMAT_MONO8;
            break;
        case AUDIO_U16LSB:
        case AUDIO_S16LSB:
        case AUDIO_U16MSB:
        case AUDIO_S16MSB:
            if( spec.channels == 2 ) format = AL_FORMAT_STEREO16;
            else format = AL_FORMAT_MONO16;
            
            #ifdef WORDS_BIGENDIAN
            // swap bytes around for big-endian systems
            for(unsigned int n=0; n<size-1; n+=2)
            {
                Uint8 temp = data[n+1];
                data[n+1] = data[n];
                data[n] = temp;
            }
            #endif
            
            break;
    }

    alBufferData(m_soundBuffer, format, data, size, spec.freq);
    if (alGetError() != AL_NO_ERROR)
    {
        fprintf(stderr, "Error 2 loading SFX: %s failed\n", path.c_str());
        return false;
    }

    SDL_FreeWAV(data);

    alGenSources(1, &m_soundSource );
    if (alGetError() != AL_NO_ERROR)
    {
        fprintf(stderr, "Error 3 loading SFX: %s failed\n", path.c_str());
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

#endif //if (HAVE_OPENAL && HAVE_OGGVORBIS)

