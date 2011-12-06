//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 Marianne Gagnon
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

#include "audio/sfx_buffer.hpp"
#include "audio/sfx_manager.hpp"
#include "io/file_manager.hpp"
#include "utils/constants.hpp"

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#if HAVE_OGGVORBIS
#  ifdef __APPLE__
#    include <OpenAL/al.h>
#    include <OpenAL/alc.h>
#  else
#    include <AL/al.h>
#    include <AL/alc.h>
#  endif
#endif

//----------------------------------------------------------------------------

SFXBuffer::SFXBuffer(const std::string& file,
                     bool  positional,
                     float rolloff,
                     float gain)
{
    m_buffer     = 0;
    m_gain       = 1.0f;
    m_rolloff    = 0.1f;
    m_loaded     = false;
    m_file       = file;
    
    m_rolloff    = rolloff;
    m_positional = positional;
    m_gain       = gain;
}

//----------------------------------------------------------------------------

SFXBuffer::SFXBuffer(const std::string& file,
                     const XMLNode* node)
{
    m_buffer     = 0;
    m_gain       = 1.0f;
    m_rolloff    = 0.1f;
    m_positional = false;
    m_loaded     = false;
    m_file       = file;
    
    node->get("rolloff",     &m_rolloff    );
    node->get("positional",  &m_positional );
    node->get("volume",      &m_gain       );
}

//----------------------------------------------------------------------------

bool SFXBuffer::load()
{
#if HAVE_OGGVORBIS
    if (m_loaded) return false;
    
    alGetError(); // clear errors from previously
    
    alGenBuffers(1, &m_buffer);
    if (!SFXManager::checkError("generating a buffer"))
    {
        return false;
    }
    
    assert( alIsBuffer(m_buffer) );
    
    if (!loadVorbisBuffer(m_file, m_buffer))
    {
        fprintf(stderr, "Could not load sound effect %s\n", m_file.c_str());
        // TODO: free al buffer here?
        return false;
    }
#endif
    
    m_loaded = true;
    return true;
}

//----------------------------------------------------------------------------

void SFXBuffer::unload()
{
#if HAVE_OGGVORBIS
    if (m_loaded)
    {
        alDeleteBuffers(1, &m_buffer);
        m_buffer = 0;
    }
#endif
    m_loaded = false;
}

//----------------------------------------------------------------------------
/** Load a vorbis file into an OpenAL buffer
 *  based on a routine by Peter Mulholland, used with permission (quote : 
 *  "Feel free to use")
 */
bool SFXBuffer::loadVorbisBuffer(const std::string &name, ALuint buffer)
{
#if HAVE_OGGVORBIS
    const int ogg_endianness = (IS_LITTLE_ENDIAN ? 0 : 1);
    
    
    bool success = false;
    FILE *file;
    vorbis_info *info;
    OggVorbis_File oggFile;
    
    if (alIsBuffer(buffer) == AL_FALSE)
    {
        printf("Error, bad OpenAL buffer");
        return false;
    }
    
    file = fopen(name.c_str(), "rb");
    
    if(!file)
    {
        fprintf(stderr, "[SFXBuffer] LoadVorbisBuffer() - couldn't open file!\n");
        return false;
    }
    
    if (ov_open_callbacks(file, &oggFile, NULL, 0,  OV_CALLBACKS_NOCLOSE) != 0)
    {
        fclose(file);
        fprintf(stderr, "[SFXBuffer] LoadVorbisBuffer() - ov_open_callbacks() failed, file isn't vorbis?\n");
        return false;
    }
    
    info = ov_info(&oggFile, -1);
    
    long len = (long)ov_pcm_total(&oggFile, -1) * info->channels * 2;    // always 16 bit data
    
    char *data = (char *) malloc(len);
    if(!data)
    {
        ov_clear(&oggFile);
        fprintf(stderr, "[SFXBuffer] loadVorbisBuffer() - Error : LoadVorbisBuffer() - couldn't allocate decode buffer\n");
        return false;
    }
    
    int bs = -1;
    long todo = len;
    char *bufpt = data;
    
    while (todo)
    {
        int read = ov_read(&oggFile, bufpt, todo, ogg_endianness, 2, 1, &bs);
        todo -= read;
        bufpt += read;
    }
    
    alBufferData(buffer, (info->channels == 1) ? AL_FORMAT_MONO16 
                 : AL_FORMAT_STEREO16,
                 data, len, info->rate);
    success = true;
    
    free(data);
    
    ov_clear(&oggFile);
    fclose(file);
    return success;
#else
    return false;
#endif
}

