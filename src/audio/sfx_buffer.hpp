//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2013 Marianne Gagnon
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

#ifndef HEADER_SFX_BUFFER_HPP
#define HEADER_SFX_BUFFER_HPP

#if HAVE_OGGVORBIS
#  ifdef __APPLE__
#    include <OpenAL/al.h>
#  else
#    include <AL/al.h>
#  endif
#else
typedef unsigned int ALuint;
#endif

#include "utils/no_copy.hpp"
#include "utils/vec3.hpp"
#include "utils/leak_check.hpp"

#include <string>

class SFXBase;
class XMLNode;

/**
 * \brief The buffer (data) for one kind of sound effects
 * \ingroup audio
 */
class SFXBuffer
{
private:

    LEAK_CHECK()

    /** Whether the contents of the file was loaded */
    bool m_loaded;

    /** The file that contains the OGG audio data */
    std::string m_file;

    /** The openal buffer id. */
    ALuint   m_buffer;
    
    /** If the sound is positional. */
    bool     m_positional;

    /** The roll-off value. */
    float    m_rolloff;

    /** The volume gain value. */
    float    m_gain;

    /** Maximum distance the sfx can be heard. */
    float    m_max_dist;

    /** Duration of the sfx. */
    float    m_duration;

    bool loadVorbisBuffer(const std::string &name, ALuint buffer);

public:

    SFXBuffer(const  std::string& file,
              bool   positional,
              float  rolloff,
              float  max_width,
              float  gain);

    SFXBuffer(const std::string& file,
              const XMLNode* node);
    ~SFXBuffer()
    {
    }   // ~SFXBuffer


    bool load();
    void unload();

    // ------------------------------------------------------------------------
    /** \return whether this buffer was loaded from disk */
    bool isLoaded() const { return m_loaded; }
    // ------------------------------------------------------------------------
    /** Only returns a valid buffer if isLoaded() returned true */
    ALuint getBufferID() const { return m_buffer; }
    // ------------------------------------------------------------------------
    /** Returns if the buffer is positional. */
    bool isPositional() const { return m_positional; }
    // ------------------------------------------------------------------------
    /** Returns the rolloff value of this buffer. */
    float getRolloff() const { return m_rolloff; }
    // ------------------------------------------------------------------------
    /** Returns the gain for this sfx. */
    float getGain() const { return m_gain; }
    // ------------------------------------------------------------------------
    /** Returns the maximum distance this sfx can be heard. */
    float getMaxDist() const { return m_max_dist; }
    // ------------------------------------------------------------------------
    /** Returns the file name of this buffer. */
    const std::string& getFileName() const { return m_file; }
    // ------------------------------------------------------------------------
    /** Sets if this buffer is positional or not. */
    void  setPositional(bool positional) { m_positional = positional; }
    // ------------------------------------------------------------------------
    /** Returns how long this buffer will play. */
    float getDuration() const { return m_duration; }

};   // class SFXBuffer


#endif // HEADER_SFX_BUFFER_HPP

