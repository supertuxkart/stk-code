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

#ifndef HEADER_MUSICOGG_HPP
#define HEADER_MUSICOGG_HPP

#ifdef ENABLE_SOUND

#include <string>

#include <ogg/ogg.h>
// Disable warning about potential loss of precision in vorbisfile.h
#if defined(WIN32) && !defined(__CYGWIN__)  && !defined(__MINGW32__)
#  pragma warning(disable:4244)
#endif
#  include <vorbis/vorbisfile.h>
#if defined(WIN32) && !defined(__CYGWIN__)  && !defined(__MINGW32__)
#  pragma warning(default:4244)
#endif

#include <AL/al.h>
#include "audio/music.hpp"

#include <atomic>

/**
  * \brief ogg files based implementation of the Music interface
  * \ingroup audio
  */
class MusicOggStream : public Music
{
public:
    MusicOggStream(float loop_start, float loop_end);
    virtual ~MusicOggStream();

    virtual void update();
    virtual void updateFaster(float percent, float max_pitch);

    virtual bool load(const std::string& filename);

    virtual bool playMusic();
    virtual bool stopMusic();
    virtual bool pauseMusic();
    virtual bool resumeMusic();
    virtual void setVolume(float volume);
    virtual bool isPlaying();

protected:
    bool empty();
    bool check(const char* what);
    std::string errorString(int code);

private:
    bool release();
    bool streamIntoBuffer(ALuint buffer);

    float           m_loop_start;
    float           m_loop_end;
    std::string     m_fileName;
    FILE*           m_oggFile;
    OggVorbis_File  m_oggStream;
    vorbis_info*    m_vorbisInfo;
    bool            m_error;

    std::atomic_bool m_playing;

    ALuint m_soundBuffers[2];
    ALuint m_soundSource;
    ALenum nb_channels;

    bool m_pausedMusic;

    //one full second of audio at 44100 samples per second
    static const int m_buffer_size = 11025*4;
};

#endif

#endif // HEADER_MUSICOGG_HPP
