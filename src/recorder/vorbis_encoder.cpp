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

#include "recorder/vorbis_encoder.hpp"
#include "utils/avi_writer.hpp"
#include "utils/log.hpp"
#include "utils/vs.hpp"

#include <ogg/ogg.h>
#include <vorbis/vorbisenc.h>

namespace Recorder
{
    void* vorbisEncoder(void *obj)
    {
        VS::setThreadName("vorbisEncoder");
        VorbisEncoderData* ved = (VorbisEncoderData*)obj;
        vorbis_info vi;
        vorbis_dsp_state vd;
        vorbis_block vb;
        vorbis_info_init(&vi);
        vorbis_encode_init(&vi, ved->m_channels, ved->m_sample_rate, -1,
            112000, -1);
        vorbis_analysis_init(&vd, &vi);
        vorbis_block_init(&vd, &vb);
        vorbis_comment vc;
        vorbis_comment_init(&vc);
        vorbis_comment_add_tag(&vc, "ENCODER", "STK vorbis encoder");
        ogg_packet header;
        ogg_packet header_comm;
        ogg_packet header_code;
        vorbis_analysis_headerout(&vd, &vc, &header, &header_comm,
            &header_code);
        if (header.bytes > 255 || header_comm.bytes > 255)
        {
            Log::error("vorbisEncoder", "Header is too long.");
            return NULL;
        }
        FILE* vb_data = fopen((AVIWriter::getRecordingTarget() + ".vb_data")
            .c_str(), "wb");
        if (vb_data == NULL)
        {
            Log::error("vorbisEncoder", "Failed to open file for encoding"
                " vorbis.");
            return NULL;
        }
        fwrite(&ved->m_sample_rate, 1, sizeof(uint32_t), vb_data);
        fwrite(&ved->m_channels, 1, sizeof(uint32_t), vb_data);
        const uint32_t all = header.bytes + header_comm.bytes +
            header_code.bytes + 3;
        fwrite(&all, 1, sizeof(uint32_t), vb_data);
        uint8_t size = 2;
        fwrite(&size, 1, sizeof(uint8_t), vb_data);
        size = (uint8_t)header.bytes;
        fwrite(&size, 1, sizeof(uint8_t), vb_data);
        size = (uint8_t)header_comm.bytes;
        fwrite(&size, 1, sizeof(uint8_t), vb_data);
        fwrite(header.packet, 1, header.bytes, vb_data);
        fwrite(header_comm.packet, 1, header_comm.bytes, vb_data);
        fwrite(header_code.packet, 1, header_code.bytes, vb_data);
        Synchronised<std::list<int8_t*> >* audio_data =
            (Synchronised<std::list<int8_t*> >*)ved->m_data;
        pthread_cond_t* cond_request = ved->m_enc_request;
        ogg_packet op;
        int64_t last_timestamp = 0;
        bool eos = false;
        while (eos == false)
        {
            audio_data->lock();
            bool waiting = audio_data->getData().empty();
            while (waiting)
            {
                pthread_cond_wait(cond_request, audio_data->getMutex());
                waiting = audio_data->getData().empty();
            }
            int8_t* audio_buf = audio_data->getData().front();
            audio_data->getData().pop_front();
            audio_data->unlock();
            if (audio_buf == NULL)
            {
                vorbis_analysis_wrote(&vd, 0);
                eos = true;
            }
            else
            {
                float **buffer = vorbis_analysis_buffer(&vd, 1024);
                const unsigned channels = ved->m_channels;
                if (ved->m_audio_type == VorbisEncoderData::AT_PCM)
                {
                    for (unsigned j = 0; j < channels; j++)
                    {
                        for (unsigned i = 0; i < 1024; i++)
                        {
                            int8_t* each_channel =
                                &audio_buf[i * channels * 2 + j * 2];
                            buffer[j][i] = float((each_channel[1] << 8) |
                               (0x00ff & (int)each_channel[0])) / 32768.0f;
                        }
                    }
                }
                else
                {
                    for (unsigned j = 0; j < channels; j++)
                    {
                        for (unsigned i = 0; i < 1024; i++)
                        {
                            float* fbuf = reinterpret_cast<float*>(audio_buf);
                            buffer[j][i] = fbuf[i * channels + j];
                        }
                    }
                }
                vorbis_analysis_wrote(&vd, 1024);
            }
            while (vorbis_analysis_blockout(&vd, &vb) == 1)
            {
                vorbis_analysis(&vb, NULL);
                vorbis_bitrate_addblock(&vb);
                while (vorbis_bitrate_flushpacket(&vd, &op))
                {
                    if (op.granulepos > 0)
                    {
                        uint32_t frame_size = (uint32_t)op.bytes;
                        fwrite(&frame_size, 1, sizeof(uint32_t), vb_data);
                        fwrite(&last_timestamp, 1, sizeof(int64_t), vb_data);
                        fwrite(op.packet, 1, frame_size, vb_data);
                        double s =
                            (double)op.granulepos / 44100. * 1000000000.;
                        last_timestamp = (int64_t)s;
                    }
                }
            }
            delete[] audio_buf;
        }
        vorbis_block_clear(&vb);
        vorbis_dsp_clear(&vd);
        vorbis_comment_clear(&vc);
        vorbis_info_clear(&vi);
        fclose(vb_data);
        return NULL;

    }   // vorbisEncode
}
#endif
