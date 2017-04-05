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

#if defined(ENABLE_RECORDER) && !defined(NO_VPX)

#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "recorder/recorder_common.hpp"
#include "utils/log.hpp"
#include "utils/synchronised.hpp"
#include "utils/vs.hpp"

#include <chrono>
#include <turbojpeg.h>
#include <vpx/vpx_encoder.h>
#include <vpx/vp8cx.h>

namespace Recorder
{
    // ========================================================================
    struct JPGDecoder
    {
        tjhandle m_handle;
        // --------------------------------------------------------------------
        JPGDecoder()
        {
            m_handle = tjInitDecompress();
        }   // JPGDecoder
        // --------------------------------------------------------------------
        ~JPGDecoder()
        {
            tjDestroy(m_handle);
        }   // ~JPGDecoder
        // --------------------------------------------------------------------
        int yuvConversion(uint8_t* jpeg_buffer, unsigned jpeg_size,
                          uint8_t** yuv_buffer, unsigned* yuv_size)
        {
            int width, height;
            TJSAMP subsample;
            int ret = 0;
            ret = tjDecompressHeader2(m_handle, jpeg_buffer, jpeg_size, &width,
                &height, (int*)&subsample);
            if (ret != 0)
            {
                char* err = tjGetErrorStr();
                Log::error("vpxEncoder", "Jpeg decode error: %s.", err);
                return ret;
            }
            *yuv_size = tjBufSizeYUV(width, height, subsample);
            *yuv_buffer = new uint8_t[*yuv_size];
            ret = tjDecompressToYUV(m_handle, jpeg_buffer, jpeg_size,
                *yuv_buffer, 0);
            if (ret != 0)
            {
                char* err = tjGetErrorStr();
                Log::error("vpxEncoder", "YUV conversion error: %s.", err);
                return ret;
            }
            return ret;
        }   // yuvConversion
    };
    // ========================================================================
    JPGDecoder g_jpg_decoder;
    // ========================================================================
    int vpxEncodeFrame(vpx_codec_ctx_t *codec, vpx_image_t *img,
                       int frame_index, FILE *out)
    {
        int got_pkts = 0;
        vpx_codec_iter_t iter = NULL;
        const vpx_codec_cx_pkt_t *pkt = NULL;
        const vpx_codec_err_t res = vpx_codec_encode(codec, img, frame_index,
            1, 0, VPX_DL_REALTIME);
        if (res != VPX_CODEC_OK)
        {
            Log::error("vpxEncoder", "Failed to encode frame");
            return -1;
        }
        while ((pkt = vpx_codec_get_cx_data(codec, &iter)) != NULL)
        {
            got_pkts = 1;
            if (pkt->kind == VPX_CODEC_CX_FRAME_PKT)
            {
                fwrite(&pkt->data.frame.sz, 1, sizeof(uint32_t), out);
                fwrite(&pkt->data.frame.pts, 1, sizeof(int64_t), out);
                fwrite(&pkt->data.frame.flags, 1,
                    sizeof(vpx_codec_frame_flags_t), out);
                fwrite(pkt->data.frame.buf, 1, pkt->data.frame.sz, out);
            }
        }
        return got_pkts;
    }   // vpxEncodeFrame
    // ------------------------------------------------------------------------
    void* vpxEncoder(void *obj)
    {
        VS::setThreadName("vpxEncoder");
        FILE* vpx_data = fopen((getRecordingName() + ".video").c_str(), "wb");
        if (vpx_data == NULL)
        {
            Log::error("vpxEncoder", "Failed to open file for writing");
            return NULL;
        }
        ThreadData* td = (ThreadData*)obj;
        Synchronised<std::list<std::tuple<uint8_t*, unsigned, int> > >*
            jpg_data = (Synchronised<std::list<std::tuple<uint8_t*,
            unsigned, int> > >*)td->m_data;
        pthread_cond_t* cond_request = td->m_request;

        vpx_codec_ctx_t codec;
        vpx_codec_enc_cfg_t cfg;
        vpx_codec_iface_t* codec_if = NULL;
        VideoFormat vf = (VideoFormat)(int)UserConfigParams::m_record_format;
        switch (vf)
        {
        case VF_VP8:
            codec_if = vpx_codec_vp8_cx();
            break;
        case VF_VP9:
            codec_if = vpx_codec_vp9_cx();
            break;
        case VF_MJPEG:
        case VF_H264:
            assert(false);
            break;
        }
        vpx_codec_err_t res = vpx_codec_enc_config_default(codec_if, &cfg, 0);
        if (res > 0)
        {
            Log::error("vpxEncoder", "Failed to get default codec config.");
            return NULL;
        }

        const unsigned width = irr_driver->getActualScreenSize().Width;
        const unsigned height = irr_driver->getActualScreenSize().Height;
        int frames_encoded = 0;
        cfg.g_w = width;
        cfg.g_h = height;
        cfg.g_timebase.num = 1;
        cfg.g_timebase.den = UserConfigParams::m_record_fps;
        int end_usage = UserConfigParams::m_vp_end_usage;
        cfg.rc_end_usage = (vpx_rc_mode)end_usage;
        cfg.rc_target_bitrate = UserConfigParams::m_vp_bitrate;

        if (vpx_codec_enc_init(&codec, codec_if, &cfg, 0) > 0)
        {
            Log::error("vpxEncoder", "Failed to initialize encoder");
            fclose(vpx_data);
            return NULL;
        }
        std::chrono::high_resolution_clock::time_point tp;
        while (true)
        {
            jpg_data->lock();
            bool waiting = jpg_data->getData().empty();
            while (waiting)
            {
                pthread_cond_wait(cond_request, jpg_data->getMutex());
                waiting = jpg_data->getData().empty();
            }

            if (displayProgress())
            {
                auto rate = std::chrono::high_resolution_clock::now() -
                    tp;
                double t = std::chrono::duration_cast<std::chrono::
                    duration<double> >(rate).count();
                if (t > 3.)
                {
                    tp = std::chrono::high_resolution_clock::now();
                    Log::info("vpxEncoder", "%d frames remaining.",
                        jpg_data->getData().size());
                }
            }
            auto& p =  jpg_data->getData().front();
            uint8_t* jpg = std::get<0>(p);
            unsigned jpg_size = std::get<1>(p);
            int frame_count = std::get<2>(p);
            if (jpg == NULL)
            {
                jpg_data->getData().clear();
                jpg_data->unlock();
                break;
            }
            jpg_data->getData().pop_front();
            jpg_data->unlock();
            uint8_t* yuv = NULL;
            unsigned yuv_size;
            int ret = g_jpg_decoder.yuvConversion(jpg, jpg_size, &yuv,
                &yuv_size);
            if (ret < 0)
            {
                delete [] yuv;
                tjFree(jpg);
                continue;
            }
            assert(yuv_size != 0);
            tjFree(jpg);
            vpx_image_t each_frame;
            vpx_img_wrap(&each_frame, VPX_IMG_FMT_I420, width, height, 1, yuv);
            while (frame_count != 0)
            {
                vpxEncodeFrame(&codec, &each_frame, frames_encoded++, vpx_data);
                frame_count--;
            }
            delete [] yuv;
        }

        while (vpxEncodeFrame(&codec, NULL, -1, vpx_data));
        if (vpx_codec_destroy(&codec))
        {
            Log::error("vpxEncoder", "Failed to destroy codec.");
            return NULL;
        }
        fclose(vpx_data);
        return NULL;

    }   // vpxEncoder
}
#endif
