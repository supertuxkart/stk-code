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

#if HAVE_OPENAL && HAVE_MIKMOD

#ifdef __APPLE__
#  include <OpenAL/al.h>
#else
#  include <AL/al.h>
#endif
#include <AL/alut.h>

#include "music_mikmod.hpp"
#include "loader.hpp"
#include "user_config.hpp"

#define BUFFER_SIZE (1024 * 32)


MusicMikMod::MusicMikMod()
{
    m_modStream= NULL;
    m_soundBuffers[0]= m_soundBuffers[1]= 0;
    m_soundSource= 0;
    m_pausedMusic= true;

    static bool initialized = false;
    if( !initialized )
    {
        initialized = true;

        //Register network drivers
        MikMod_RegisterDriver(&drv_AF);
        MikMod_RegisterDriver(&drv_esd);

        //Register hardware drivers, but disk writer drivers cause problems
        //on computers without proper hardware or network drivers.
        MikMod_RegisterDriver(&drv_aix);
        MikMod_RegisterDriver(&drv_alsa);
        MikMod_RegisterDriver(&drv_hp);
        MikMod_RegisterDriver(&drv_sam9407);
        MikMod_RegisterDriver(&drv_sgi);
        MikMod_RegisterDriver(&drv_sun);
        MikMod_RegisterDriver(&drv_ultra);

        //These two won't link, at least under Gentoo linux.
        //MikMod_RegisterDriver(&drv_dart);
        //MikMod_RegisterDriver(&drv_os2);

        //Register the null driver in case sound is not available.
        //Also, for some reason, the assert at line 76 of this file
        //crashes the program without it.
        MikMod_RegisterDriver(&drv_nos);

        // register loaders (it, s3m, xm y mod)
        MikMod_RegisterAllLoaders();
    }

    if(MikMod_Init(""))
    {
        user_config->setMusic(UserConfig::UC_TEMPORARY_DISABLE);
        fprintf(stderr,"Problems initialising mikmod. Disabling music.\n");
    }
}

//-----------------------------------------------------------------------------
MusicMikMod::~MusicMikMod()
{
    stopMusic();
    if(!release())
    {
        fprintf(stderr,"Problems with mikmod:release.\n");
    }
    MikMod_Exit();
}

//-----------------------------------------------------------------------------
bool MusicMikMod::load(const char* filename)
{
    if(!release())
    {
        user_config->setMusic(UserConfig::UC_TEMPORARY_DISABLE);
        fprintf(stderr,"Problems mikmod:release. Disabling music.\n");
        return false;
    }


    m_fileName =  loader->getPath(filename);
    FILE* modFile = fopen(m_fileName.c_str(), "rb");
    if (modFile == NULL)
    {
        fprintf(stderr, "Loading Music: %s failed\n", m_fileName.c_str());
        return false;
    }

    m_modStream= Player_LoadFP(modFile, 255, 0);

    fclose(modFile);
    if (m_modStream == NULL)
    {
        fprintf(stderr, "Loading Music: %s failed\n", m_fileName.c_str());
        return false;
    }

    Player_Start(m_modStream);

    alGenBuffers(2, m_soundBuffers);
    if (alGetError() != AL_NO_ERROR)
    {
        fprintf(stderr, "Loading Music: %s failed\n", m_fileName.c_str());
        return false;
    }

    alGenSources(1, &m_soundSource);
    if (alGetError() != AL_NO_ERROR)
    {
        fprintf(stderr, "Loading Music: %s failed\n", m_fileName.c_str());
        return false;
    }

    alSource3f(m_soundSource, AL_POSITION,        0.0, 0.0, 0.0);
    alSource3f(m_soundSource, AL_VELOCITY,        0.0, 0.0, 0.0);
    alSource3f(m_soundSource, AL_DIRECTION,       0.0, 0.0, 0.0);
    alSourcef (m_soundSource, AL_ROLLOFF_FACTOR,  0.0          );
    alSourcei (m_soundSource, AL_SOURCE_RELATIVE, AL_TRUE      );

    return true;
}

//-----------------------------------------------------------------------------
bool MusicMikMod::release()
{
    if (m_fileName == "")
    {
        // nothing is loaded
        return true;
    }
    m_fileName= "";

    alSourceStop(m_soundSource);

    // empty
    int queued= 0;
    alGetSourcei(m_soundSource, AL_BUFFERS_QUEUED, &queued);
    while(queued--)
    {
        ALuint buffer= 0;
        alSourceUnqueueBuffers(m_soundSource, 1, &buffer);
        if (alGetError() != AL_NO_ERROR)
        {
            return false;
        }
    }
    alDeleteSources(1, &m_soundSource);
    if (alGetError() != AL_NO_ERROR)
    {
        return false;
    }

    alDeleteBuffers(2, m_soundBuffers);
    if (alGetError() != AL_NO_ERROR)
    {
        return false;
    }

    Player_Free(m_modStream);
    return true;
}

//-----------------------------------------------------------------------------
bool MusicMikMod::playMusic()
{
    if(isPlaying())
    {
        return true;
    }

    if (!streamIntoBuffer(m_soundBuffers[0]))
    {
        return false;
    }

    if(!streamIntoBuffer(m_soundBuffers[1]))
    {
        return false;
    }
    alSourceQueueBuffers(m_soundSource, 2, m_soundBuffers);

    alSourcePlay(m_soundSource);
    m_pausedMusic= false;
    return true;
}

//-----------------------------------------------------------------------------
bool MusicMikMod::isPlaying()
{
    ALenum state;
    alGetSourcei(m_soundSource, AL_SOURCE_STATE, &state);
    return (state == AL_PLAYING);
}

//-----------------------------------------------------------------------------
bool MusicMikMod::stopMusic()
{
    pauseMusic();
    release();
    return true;
}

//-----------------------------------------------------------------------------
bool MusicMikMod::pauseMusic()
{
    if (m_fileName == "")
    {
        // nothing is loaded
        return true;
    }

    alSourceStop(m_soundSource);
    m_pausedMusic= true;
    return true;
}

//-----------------------------------------------------------------------------
bool MusicMikMod::resumeMusic()
{
    if (m_fileName == "")
    {
        // nothing is loaded
        return true;
    }

    alSourcePlay(m_soundSource);
    m_pausedMusic= false;
    return true;
}

//-----------------------------------------------------------------------------
void MusicMikMod::update()
{
    if (m_pausedMusic)
    {
        // nothing todo
        return;
    }

    int processed= 0;
    alGetSourcei(m_soundSource, AL_BUFFERS_PROCESSED, &processed);

    bool active= true;
    while(processed--)
    {
        ALuint buffer;
        alSourceUnqueueBuffers(m_soundSource, 1, &buffer);
        if (alGetError() != AL_NO_ERROR)
        {
            user_config->setMusic(UserConfig::UC_TEMPORARY_DISABLE);
            fprintf(stderr,"Problems with mikmod:sourceUnqueueBuffers. Disabling music.\n");
            return;
        }

        active= streamIntoBuffer(buffer);
        alSourceQueueBuffers(m_soundSource, 1, &buffer);
        if (alGetError() != AL_NO_ERROR)
        {
            user_config->setMusic(UserConfig::UC_TEMPORARY_DISABLE);
            fprintf(stderr,"Problems with mikmod:sourceQueueBuffers. Disabling music.\n");
            return;
        }
	user_config->setMusic(UserConfig::UC_TEMPORARY_DISABLE);
    }

    // check for underrun
    if (active)
    {
        // we have data, so we should be playing...

        ALenum state;
        alGetSourcei(m_soundSource, AL_SOURCE_STATE, &state);
        if (state != AL_PLAYING)
        {
            alGetSourcei(m_soundSource, AL_BUFFERS_PROCESSED, &processed);
            alSourcePlay(m_soundSource);
        }
    }
    else
    {
        // no more data. Seek to beginning -> loop
        Player_SetPosition(m_modStream->reppos);
    }
}

//-----------------------------------------------------------------------------
bool MusicMikMod::streamIntoBuffer(ALuint buffer)
{
    SBYTE pcm[BUFFER_SIZE];

    // write bytes in pcm
    VC_WriteBytes(pcm, BUFFER_SIZE);

    if (!Player_Active())
    {
        // there aren't bytes that we could write in pcm
        return false;
    }

    alBufferData(buffer, AL_FORMAT_STEREO16, (char*)pcm, BUFFER_SIZE, md_mixfreq);
    if (alGetError() != AL_NO_ERROR)
    {
        return false;
    }

    return true;
}

#endif // HAVE_OPENAL && HAVE_MIKMOD

