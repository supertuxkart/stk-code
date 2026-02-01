//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 Marianne Gagnon
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
#include "utils/file_utils.hpp"
#include "utils/log.hpp"

#ifdef ENABLE_SOUND
#  include <vorbis/codec.h>
#  include <vorbis/vorbisfile.h>
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
    
#ifdef ENABLE_SOUND
    if (UserConfigParams::m_enable_sound)
    {
        if (m_loaded) return false;
    
        alGetError(); // clear errors from previously
    
        alGenBuffers(1, &m_buffer);
        if (!SFXManager::checkError("generating a buffer"))
        {
            return false;
        }
    
        assert(alIsBuffer(m_buffer));
    
        if (!loadVorbisBuffer(m_file, m_buffer))
        {
            Log::error("SFXBuffer", "Could not load sound effect %s",
                       m_file.c_str());
            // TODO: free al buffer here?
            return false;
        }
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
#ifdef ENABLE_SOUND
    if (UserConfigParams::m_enable_sound)
    {
        if (m_loaded)
        {
            alDeleteBuffers(1, &m_buffer);
            m_buffer = 0;
        }
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
#ifdef ENABLE_SOUND
    if (!UserConfigParams::m_enable_sound)
        return false;
        
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

    file = FileUtils::fopenU8Path(name, "rb");

    if(!file)
    {
        Log::error("SFXBuffer", "LoadVorbisBuffer() - couldn't open file!");
        return false;
    }

    if (ov_open_callbacks(file, &oggFile, NULL, 0,  OV_CALLBACKS_NOCLOSE) != 0)
    {
        fclose(file);
        Log::error("SFXBuffer", "LoadVorbisBuffer() - ov_open_callbacks() failed, "
                                "file isn't vorbis?");
        return false;
    }

    info = ov_info(&oggFile, -1);

    // always 16 bit data
    long len = (long)ov_pcm_total(&oggFile, -1) * info->channels * 2;

    std::unique_ptr<char []> data = std::unique_ptr<char []>(new char[len]);

    int bs = -1;
    long todo = len;
    char *bufpt = data.get();

    while (todo)
    {
        int read = ov_read(&oggFile, bufpt, todo, ogg_endianness, 2, 1, &bs);
        todo -= read;
        bufpt += read;
    }

    alBufferData(buffer, (info->channels == 1) ? AL_FORMAT_MONO16
                 : AL_FORMAT_STEREO16,
                 data.get(), len, info->rate);
    success = true;

    if (m_positional && info->channels > 1)
        Log::error("SFXBuffer", "Positional audio is not supported with stereo files, "
            "but %s is stereo", m_file.c_str());

    int buffer_size, frequency, bits_per_sample, channels;
    buffer_size = len;
    frequency = info->rate;
    // We use AL_FORMAT_MONO16 or AL_FORMAT_STEREO16 so it's always 16
    bits_per_sample = 16;
    channels = info->channels;

    ov_clear(&oggFile);
    fclose(file);

    // Allow the xml data to overwrite the duration, but if there is no
    // duration (which is the norm), compute it:
    if(m_duration < 0)
    {
        m_duration = float(buffer_size) 
                   / (frequency*channels*(bits_per_sample / 8));
    }
    return success;
#else
    return false;
#endif
}   // loadVorbisBuffer

