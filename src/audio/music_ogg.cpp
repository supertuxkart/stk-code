//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2007-2015 Damien Morel <divdams@free.fr>
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

#ifdef ENABLE_SOUND

#include "audio/music_ogg.hpp"

#include <stdexcept>

#include "audio/music_manager.hpp"
#include "audio/sfx_manager.hpp"
#include "utils/constants.hpp"
#include "utils/file_utils.hpp"
#include "utils/log.hpp"

MusicOggStream::MusicOggStream(float loop_start, float loop_end)
{
    //m_oggStream= NULL;
    m_soundBuffers[0] = m_soundBuffers[1]= 0;
    m_soundSource     = -1;
    m_pausedMusic     = true;
    m_playing.store(false);
    m_loop_start      = loop_start;
    m_loop_end        = loop_end;
}   // MusicOggStream

//-----------------------------------------------------------------------------
MusicOggStream::~MusicOggStream()
{
    if(stopMusic() == false)
        Log::warn("MusicOgg", "problems while stopping music.");
}   // ~MusicOggStream

//-----------------------------------------------------------------------------
bool MusicOggStream::load(const std::string& filename)
{
    if (isPlaying()) stopMusic();

    m_error = true;
    m_fileName = filename;
    if(m_fileName=="") return false;

    m_oggFile = FileUtils::fopenU8Path(m_fileName, "rb");

    if(!m_oggFile)
    {
        Log::error("MusicOgg", "Loading Music: %s failed (fopen returned NULL)",
                   m_fileName.c_str());
        return false;
    }

#if defined( WIN32 ) || defined( WIN64 )
    const int result = ov_open_callbacks((void *)m_oggFile, &m_oggStream, NULL,
                                         0, OV_CALLBACKS_DEFAULT             );
#else
    const int result = ov_open(m_oggFile, &m_oggStream, NULL, 0);
#endif

    if (result < 0)
    {
        fclose(m_oggFile);


        const char* errorMessage;
        switch (result)
        {
            case OV_EREAD:
                errorMessage = "OV_EREAD";
                break;
            case OV_ENOTVORBIS:
                errorMessage = "OV_ENOTVORBIS";
                break;
            case OV_EVERSION:
                errorMessage = "OV_EVERSION";
                break;
            case OV_EBADHEADER:
                errorMessage = "OV_EBADHEADER";
                break;
            case OV_EFAULT:
                errorMessage = "OV_EFAULT";
                break;
            default:
                errorMessage = "Unknown Error";
        }

        Log::error("MusicOgg", "Loading Music: %s failed : "
                               "ov_open returned error code %i (%s)",
               m_fileName.c_str(), result, errorMessage);
        return false;
    }

    m_vorbisInfo = ov_info(&m_oggStream, -1);

    if (m_vorbisInfo->channels == 1) nb_channels = AL_FORMAT_MONO16;
    else                             nb_channels = AL_FORMAT_STEREO16;

    alGenBuffers(2, m_soundBuffers);
    if (check("alGenBuffers") == false) return false;

    alGenSources(1, &m_soundSource);
    if (check("alGenSources") == false) return false;

    alSource3f(m_soundSource, AL_POSITION,        0.0, 0.0, 0.0);
    alSource3f(m_soundSource, AL_VELOCITY,        0.0, 0.0, 0.0);
    alSource3f(m_soundSource, AL_DIRECTION,       0.0, 0.0, 0.0);
    alSourcef (m_soundSource, AL_ROLLOFF_FACTOR,  0.0          );
    alSourcef (m_soundSource, AL_GAIN,            1.0          );
    alSourcei (m_soundSource, AL_SOURCE_RELATIVE, AL_TRUE      );

    m_error=false;
    return true;
}   // load

//-----------------------------------------------------------------------------
bool MusicOggStream::empty()
{
    int queued= 0;
    alGetSourcei(m_soundSource, AL_BUFFERS_QUEUED, &queued);

    while(queued--)
    {
        ALuint buffer = 0;
        alSourceUnqueueBuffers(m_soundSource, 1, &buffer);

        if (!check("alSourceUnqueueBuffers")) return false;
    }

    return true;
}   // empty

//-----------------------------------------------------------------------------
bool MusicOggStream::release()
{
    if (m_fileName == "")
    {
        // nothing is loaded
        return true;
    }

    pauseMusic();
    m_fileName= "";

    empty();
    alDeleteSources(1, &m_soundSource);
    check("alDeleteSources");
    alDeleteBuffers(2, m_soundBuffers);
    check("alDeleteBuffers");

    // Handle error correctly
    if(!m_error) ov_clear(&m_oggStream);

    m_soundSource = -1;
    m_playing.store(false);

    return true;
}   // release

//-----------------------------------------------------------------------------
bool MusicOggStream::playMusic()
{
    if(isPlaying())
        return true;

    if(!streamIntoBuffer(m_soundBuffers[0]))
        return false;

    if(!streamIntoBuffer(m_soundBuffers[1]))
        return false;

    alSourceQueueBuffers(m_soundSource, 2, m_soundBuffers);

    alSourcePlay(m_soundSource);
    m_pausedMusic = false;
    m_playing.store(true);
    check("playMusic");
    return true;
}   // playMusic

//-----------------------------------------------------------------------------
bool MusicOggStream::isPlaying()
{
    return m_playing.load();

    /*
    if (m_soundSource == -1) return false;

    ALenum state;
    alGetSourcei(m_soundSource, AL_SOURCE_STATE, &state);

    return (state == AL_PLAYING);
     */
}   // isPlaying

//-----------------------------------------------------------------------------
bool MusicOggStream::stopMusic()
{
    m_playing.store(false);
    return (release());
}   // stopMusic

//-----------------------------------------------------------------------------
bool MusicOggStream::pauseMusic()
{
    m_playing.store(false);
    if (m_fileName == "")
    {
        // nothing is loaded
        return true;
    }

    alSourceStop(m_soundSource);
    m_pausedMusic= true;
    return true;
}   // pauseMusic

//-----------------------------------------------------------------------------
bool MusicOggStream::resumeMusic()
{
    if (m_fileName == "")
    {
        // nothing is loaded
        return true;
    }

    m_playing.store(true);

    alSourcePlay(m_soundSource);
    m_pausedMusic= false;
    return true;
}   // resumeMusic

//-----------------------------------------------------------------------------
void MusicOggStream::setVolume(float volume)
{
    volume *= music_manager->getMasterMusicVolume();
    if (volume > 1.0f) volume = 1.0f;
    if (volume < 0.0f) volume = 0.0f;

    alSourcef(m_soundSource, AL_GAIN, volume);
    check("volume music");   // clear errors
}   // setVolume

//-----------------------------------------------------------------------------
void MusicOggStream::updateFaster(float percent, float max_pitch)
{
    alSourcef(m_soundSource,AL_PITCH,1+max_pitch*percent);
    update();
}   // updateFaster

//-----------------------------------------------------------------------------
void MusicOggStream::update()
{

    if (m_pausedMusic || m_soundSource == ALuint(-1))
    {
        // nothing todo
        return;
    }

    int processed= 0;
    bool active= true;

    alGetSourcei(m_soundSource, AL_BUFFERS_PROCESSED, &processed);

    while(processed--)
    {
        ALuint buffer;

        alSourceUnqueueBuffers(m_soundSource, 1, &buffer);
        if(!check("alSourceUnqueueBuffers")) return;

        active = streamIntoBuffer(buffer);
        float cur_time = (float)ov_time_tell(&m_oggStream);
        if(!active || (m_loop_end > 0 && (m_loop_end - cur_time) < 1e-3))
        {
            // No more data, or reached loop end. Seek to loop start (causes the sound to loop)
            ov_time_seek(&m_oggStream, m_loop_start);
            active = streamIntoBuffer(buffer);//now there really should be data
        }

        alSourceQueueBuffers(m_soundSource, 1, &buffer);
        if (!check("alSourceQueueBuffers")) return;
    }

    if (active)
    {
        // For debugging
        SFXManager::checkError("before source state");
        // we have data, so we should be playing...
        ALenum state;
        alGetSourcei(m_soundSource, AL_SOURCE_STATE, &state);
        if (state != AL_PLAYING)
        {
            // Prevent flooding
            static int count = 0;
            count++;
            if (count<10)
                Log::warn("MusicOgg", "Music not playing when it should be. "
                          "Source state: %d", state);
            alGetSourcei(m_soundSource, AL_BUFFERS_PROCESSED, &processed);
            alSourcePlay(m_soundSource);
        }
    }
    else
    {
        Log::warn("MusicOgg", "Attempt to stream music into buffer failed "
                              "twice in a row.");
    }
}   // update

//-----------------------------------------------------------------------------
bool MusicOggStream::streamIntoBuffer(ALuint buffer)
{
    char pcm[m_buffer_size];
    const int isBigEndian = (IS_LITTLE_ENDIAN ? 0 : 1);

    int  size = 0;
    int  portion;

    while(size < m_buffer_size)
    {
        int result = ov_read(&m_oggStream, pcm + size, m_buffer_size - size,
                         isBigEndian, 2, 1, &portion);

        if(result > 0)
            size += result;
        else
            if(result < 0)
                throw errorString(result);
            else
                break;
    }

    if(size == 0) return false;

    alBufferData(buffer, nb_channels, pcm, size, m_vorbisInfo->rate);
    check("alBufferData");

    return true;
}   // streamIntoBuffer

//-----------------------------------------------------------------------------
bool MusicOggStream::check(const char* what)
{
    int error = alGetError();

    if (error != AL_NO_ERROR)
    {
        Log::error("MusicOgg", "OpenAL error at %s : %s (%i)", what,
                  SFXManager::getErrorString(error).c_str(), error);
        return false;
    }

    return true;
}   // check

//-----------------------------------------------------------------------------
std::string MusicOggStream::errorString(int code)
{
    switch(code)
    {
        case OV_EREAD:
            return std::string("Read error from media.");
        case OV_ENOTVORBIS:
            return std::string("It is not Vorbis data.");
        case OV_EVERSION:
            return std::string("Vorbis version mismatch.");
        case OV_EBADHEADER:
            return std::string("Invalid Vorbis bitstream header.");
        case OV_EFAULT:
            return std::string("Internal logic fault (bug or heap/stack corruption).");
        default:
            return std::string("Unknown Vorbis error.");
    }
}   // errorString

#endif // ENABLE_SOUND
