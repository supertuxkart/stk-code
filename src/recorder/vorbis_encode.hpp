//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2017 SuperTuxKart-Team
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

#if !(defined(SERVER_ONLY) || defined(USE_GLES2))

#ifndef HEADER_VORBIS_ENCODE_HPP
#define HEADER_VORBIS_ENCODE_HPP

#include "utils/no_copy.hpp"
#include "utils/types.hpp"

#include <pthread.h>

namespace Recorder
{
    struct VorbisEncoderData : public NoCopy
    {
        enum AudioType { AT_FLOAT, AT_PCM };
        void* m_data;
        pthread_cond_t* m_enc_request;
        uint32_t m_sample_rate;
        uint32_t m_channels;
        AudioType m_audio_type;
    };

    void* vorbisEncoder(void *obj);
};

#endif

#endif
