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

#include "capture_library.hpp"
#include "recorder_private.hpp"

#include <vpx/vpx_encoder.h>
#include <vpx/vp8cx.h>

namespace Recorder
{
    // ------------------------------------------------------------------------
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
            printf("Failed to encode frame\n");
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
    void vpxEncoder(CaptureLibrary* cl)
    {
        setThreadName("vpxEncoder");
        FILE* vpx_data = fopen((getSavedName() + ".video").c_str(), "wb");
        if (vpx_data == NULL)
        {
            printf("Failed to open file for writing vpx.\n");
            return;
        }

        vpx_codec_ctx_t codec;
        vpx_codec_enc_cfg_t cfg;
        vpx_codec_iface_t* codec_if = NULL;
        switch (cl->getRecorderConfig().m_video_format)
        {
        case OGR_VF_VP8:
            codec_if = vpx_codec_vp8_cx();
            break;
        case OGR_VF_VP9:
            codec_if = vpx_codec_vp9_cx();
            break;
        default:
            assert(false);
            break;
        }
        vpx_codec_err_t res = vpx_codec_enc_config_default(codec_if, &cfg, 0);
        if (res > 0)
        {
            printf("Failed to get default vpx codec config.\n");
            return;
        }

        const unsigned width = cl->getRecorderConfig().m_width;
        const unsigned height = cl->getRecorderConfig().m_height;
        int frames_encoded = 0;
        cfg.g_w = width;
        cfg.g_h = height;
        cfg.g_timebase.num = 1;
        cfg.g_timebase.den = cl->getRecorderConfig().m_record_fps;
        cfg.rc_end_usage = VPX_VBR;
        cfg.rc_target_bitrate = cl->getRecorderConfig().m_video_bitrate;

        if (vpx_codec_enc_init(&codec, codec_if, &cfg, 0) > 0)
        {
            printf("Failed to initialize vpx encoder\n");
            fclose(vpx_data);
            return;
        }
        float last_size = -1.0f;
        int cur_finished_count = 0;
        while (true)
        {
            std::unique_lock<std::mutex> ul(*cl->getJPGListMutex());
            cl->getJPGListCV()->wait(ul, [&cl]
                { return !cl->getJPGList()->empty(); });
            auto& p = cl->getJPGList()->front();
            uint8_t* jpg = std::get<0>(p);
            uint32_t jpg_size = std::get<1>(p);
            int frame_count = std::get<2>(p);
            if (jpg == NULL)
            {
                cl->getJPGList()->clear();
                ul.unlock();
                if (cl->displayingProgress())
                {
                    int rate = 100;
                    runCallback(OGR_CBT_PROGRESS_RECORDING, &rate);
                }
                break;
            }
            cl->getJPGList()->pop_front();
            ul.unlock();
            if (cl->displayingProgress())
            {
                if (last_size == -1.0f)
                    last_size = (float)(cl->getJPGList()->size());
                cur_finished_count += frame_count;
                int rate = (int)(cur_finished_count / last_size * 100.0f);
                rate = rate > 100 ? 100 : rate;
                runCallback(OGR_CBT_PROGRESS_RECORDING, &rate);
            }
            uint8_t* yuv = NULL;
            unsigned yuv_size;
            int ret = cl->yuvConversion(jpg, jpg_size, &yuv,
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
            printf("Failed to destroy vpx codec.\n");
            return;
        }
        fclose(vpx_data);
    }   // vpxEncoder
}
#endif
