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

#include "audio/sfx_buffer.hpp"
#include "audio/sfx_manager.hpp"
#include "config/user_config.hpp"
#include "io/file_manager.hpp"
#include "utils/constants.hpp"
#include "utils/log.hpp"

#if HAVE_OGGVORBIS
#  include <vorbis/codec.h>
#  include <vorbis/vorbisfile.h>
#  ifdef __APPLE__
#    include <OpenAL/al.h>
#    include <OpenAL/alc.h>
#  else
#    include <AL/al.h>
#    include <AL/alc.h>
#  endif
#endif

//----------------------------------------------------------------------------
/** Creates a sfx. The parameter are taken from the parameters:
 *  \param file File name of the buffer.
 *  \param positional If the sfx is positional.
 *  \param rolloff Rolloff value of this sfx.
 *  \param max_dist Maximum distance the sfx can be heard.
 *  \param gain Gain value of this sfx.
 */
SFXBuffer::SFXBuffer(const std::string& file,
                     bool  positional,
                     float rolloff,
                     float max_dist,
                     float gain)
{
    m_buffer      = 0;
    m_gain        = 1.0f;
    m_rolloff     = 0.1f;
    m_loaded      = false;
    m_max_dist    = max_dist;
    m_duration    = -1.0f;
    m_file        = file;

    m_rolloff     = rolloff;
    m_positional  = positional;
    m_gain        = gain;
}   // SFXBuffer

//----------------------------------------------------------------------------
/** Constructor getting the sfx parameters from an XML node.
 *  \param file File name of the data.
 *  \param node XML Node with the data for this sfx.
 */
SFXBuffer::SFXBuffer(const std::string& file,
                     const XMLNode* node)
{
    m_buffer      = 0;
    m_gain        = 1.0f;
    m_rolloff     = 0.1f;
    m_max_dist    = 300.0f;
    m_duration    = -1.0f;
    m_positional  = false;
    m_loaded      = false;
    m_file        = file;

    node->get("rolloff",     &m_rolloff    );
    node->get("positional",  &m_positional );
    node->get("volume",      &m_gain       );
    node->get("max_dist",    &m_max_dist   );
    node->get("duration",    &m_duration   );
}   // SFXBuffer(XMLNode)

//----------------------------------------------------------------------------
/** \brief load the buffer from file into OpenAL.
 *  \note If this buffer is already loaded, this call does nothing and 
  *       returns false.
 *  \return Whether loading was successful.
 */
bool SFXBuffer::load()
{
    if (UserConfigParams::m_sfx == false) return false;
    
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
        Log::error("SFXBuffer", "Could not load sound effect %s\n", m_file.c_str());
        // TODO: free al buffer here?
        return false;
    }
#endif

    m_loaded = true;
    return true;
}   // load

//----------------------------------------------------------------------------
/** \brief Frees the loaded buffer.
 *  Cannot appear in destructor because copy-constructors may be used,
 *  and the OpenAL source must not be deleted on a copy
 */

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
}   // unload

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
        Log::error("SFXBuffer", "Error, bad OpenAL buffer");
        return false;
    }

    file = fopen(name.c_str(), "rb");

    if(!file)
    {
        Log::error("SFXBuffer", "[SFXBuffer] LoadVorbisBuffer() - couldn't open file!\n");
        return false;
    }

    if (ov_open_callbacks(file, &oggFile, NULL, 0,  OV_CALLBACKS_NOCLOSE) != 0)
    {
        fclose(file);
        Log::error("SFXBuffer", "[SFXBuffer] LoadVorbisBuffer() - ov_open_callbacks() failed, file isn't vorbis?\n");
        return false;
    }

    info = ov_info(&oggFile, -1);

    long len = (long)ov_pcm_total(&oggFile, -1) * info->channels * 2;    // always 16 bit data

    char *data = (char *) malloc(len);
    if(!data)
    {
        ov_clear(&oggFile);
        Log::error("SFXBuffer", "[SFXBuffer] Could not allocate decode buffer.");
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

    // Allow the xml data to overwrite the duration, but if there is no
    // duration (which is the norm), compute it:
    if(m_duration < 0)
    {
        ALint buffer_size, frequency, bits_per_sample, channels;
        alGetBufferi(buffer, AL_SIZE,      &buffer_size    );
        alGetBufferi(buffer, AL_FREQUENCY, &frequency      );
        alGetBufferi(buffer, AL_CHANNELS,  &channels       );
        alGetBufferi(buffer, AL_BITS,      &bits_per_sample);
        m_duration = float(buffer_size) / (frequency*channels*(bits_per_sample / 8));
    }
    return success;
#else
    return false;
#endif
}   // loadVorbisBuffer

