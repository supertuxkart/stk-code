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

#ifdef ENABLE_RECORDER

#include "recorder/recorder_common.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/gl_headers.hpp"
#include "guiengine/message_queue.hpp"
#include "recorder/jpg_writer.hpp"
#include "recorder/mkv_writer.hpp"
#include "recorder/pulseaudio_recorder.hpp"
#include "recorder/vpx_encoder.hpp"
#include "recorder/wasapi_recorder.hpp"
#include "utils/synchronised.hpp"
#include "utils/translation.hpp"
#include "utils/vs.hpp"

#include <chrono>
#include <cassert>
#include <turbojpeg.h>

namespace Recorder
{
    // ========================================================================
    tjhandle g_compress_handle;
    // ========================================================================
    Synchronised<std::list<std::tuple<uint8_t*, unsigned, int> > > g_jpg_list;
    // ========================================================================
    pthread_cond_t g_jpg_request;
    // ========================================================================
    ThreadData g_jpg_thread_data;
    // ========================================================================
    int bmpToJPG(uint8_t* raw, unsigned width, unsigned height,
                 uint8_t** jpeg_buffer, unsigned long* jpeg_size)
    {
        int ret = 0;
#ifdef TJFLAG_FASTDCT
        ret = tjCompress2(g_compress_handle, raw, width, 0, height, TJPF_BGR,
            jpeg_buffer, jpeg_size, TJSAMP_420,
            UserConfigParams::m_recorder_jpg_quality, TJFLAG_FASTDCT);
#else
        ret = tjCompress2(g_compress_handle, raw, width, 0, height, TJPF_BGR,
            jpeg_buffer, jpeg_size, TJSAMP_420,
            UserConfigParams::m_recorder_jpg_quality, 0);
#endif
        if (ret != 0)
        {
            char* err = tjGetErrorStr();
            Log::error("RecorderCommon", "Jpeg encode error: %s.", err);
            return ret;
        }
        return ret;
    }   // bmpToJPG
    // ========================================================================
    pthread_t g_audio_thread;
    // ========================================================================
    Synchronised<pthread_t*> g_video_thread(NULL);
    // ========================================================================
    Synchronised<bool> g_idle(true);
    // ========================================================================
    bool g_destroy;
    // ========================================================================
    std::string g_recording_name;
    // ========================================================================
    void* fbiConversion(void* obj)
    {
        VS::setThreadName("fbiConversion");
        ThreadData* td = (ThreadData*)obj;
        Synchronised<std::list<std::pair<uint8_t*, int> > >* fbi_queue =
            (Synchronised<std::list<std::pair<uint8_t*, int> > >*)td->m_data;
        pthread_cond_t* cond_request = td->m_request;
        while (true)
        {
            fbi_queue->lock();
            bool waiting = fbi_queue->getData().empty();
            while (waiting)
            {
                pthread_cond_wait(cond_request, fbi_queue->getMutex());
                waiting = fbi_queue->getData().empty();
            }
            auto& p = fbi_queue->getData().front();
            uint8_t* fbi = p.first;
            int frame_count = p.second;
            if (frame_count == -1)
            {
                fbi_queue->getData().clear();
                fbi_queue->unlock();
                g_idle.setAtomic(true);
                pthread_join(g_audio_thread, NULL);
                g_jpg_list.lock();
                g_jpg_list.getData().emplace_back((uint8_t*)NULL, 0, 0);
                pthread_cond_signal(&g_jpg_request);
                g_jpg_list.unlock();
                pthread_join(*g_video_thread.getData(), NULL);
                delete g_video_thread.getData();
                g_video_thread.setAtomic(NULL);
                Recorder::writeMKV(g_recording_name + ".video",
                    g_recording_name + ".audio");
                if (g_destroy)
                {
                    return NULL;
                }
                continue;
            }
            else if (fbi == NULL)
            {
                fbi_queue->getData().clear();
                fbi_queue->unlock();
                if (g_destroy)
                {
                    return NULL;
                }
                continue;
            }
            const bool too_slow = fbi_queue->getData().size() > 50;
            if (too_slow)
            {
                MessageQueue::add(MessageQueue::MT_ERROR,
                    _("Encoding is too slow, dropping frames."));
                delete [] fbi;
                fbi_queue->getData().pop_front();
                for (auto& p : fbi_queue->getData())
                    delete [] p.first;
                fbi_queue->getData().clear();
                fbi_queue->unlock();
                continue;
            }
            fbi_queue->getData().pop_front();
            fbi_queue->unlock();
            uint8_t* orig_fbi = fbi;
            const unsigned width = irr_driver->getActualScreenSize().Width;
            const unsigned height = irr_driver->getActualScreenSize().Height;
            const unsigned area = width * height;
            int size = area * 4;
            int dest = size - 3;
            int src = size - 4;
            int copied = 0;
            while (true)
            {
                if (copied++ > 1)
                    memcpy(fbi + dest, fbi + src, 3);
                else
                    memmove(fbi + dest, fbi + src, 3);
                if (src == 0)
                    break;
                dest -= 3;
                src -= 4;
            }
            fbi = fbi + area;
            const int pitch = width * 3;
            uint8_t* p2 = fbi + (height - 1) * pitch;
            uint8_t* tmp_buf = new uint8_t[pitch];
            for (unsigned i = 0; i < height; i += 2)
            {
                memcpy(tmp_buf, fbi, pitch);
                memcpy(fbi, p2, pitch);
                memcpy(p2, tmp_buf, pitch);
                fbi += pitch;
                p2 -= pitch;
            }
            delete [] tmp_buf;
            uint8_t* jpg = NULL;
            unsigned long jpg_size = 0;
            bmpToJPG(orig_fbi + area, width, height, &jpg, &jpg_size);
            delete[] orig_fbi;
            g_jpg_list.lock();
            g_jpg_list.getData().emplace_back(jpg, jpg_size, frame_count);
            pthread_cond_signal(&g_jpg_request);
            g_jpg_list.unlock();
        }
        return NULL;
    }   // fbiConversion
    // ========================================================================
    struct CommonData : public NoCopy
    {
        GLuint m_pbo[3];
        unsigned m_pbo_use;
        bool m_loaded;
        Synchronised<std::list<std::pair<uint8_t*, int> > > m_fbi_queue;
        pthread_cond_t m_fbi_request;
        pthread_t m_fbi_thread;
        ThreadData m_common_thread_data;
        // --------------------------------------------------------------------
        void addFrameBufferImage(uint8_t* fbi, int frame_count)
        {
            m_fbi_queue.lock();
            m_fbi_queue.getData().emplace_back(fbi, frame_count);
            pthread_cond_signal(&m_fbi_request);
            m_fbi_queue.unlock();
        }   // addFrameBufferImage
        // --------------------------------------------------------------------
        CommonData()
        {
            m_loaded = false;
            m_pbo_use = 0;
            g_compress_handle = tjInitCompress();
        }   // CommonData
        // --------------------------------------------------------------------
        ~CommonData()
        {
            destroy();
            tjDestroy(g_compress_handle);
        }   // ~CommonData
        // --------------------------------------------------------------------
        void destroy()
        {
            if (m_loaded)
            {
                glDeleteBuffers(3, m_pbo);
                addFrameBufferImage(NULL, 0);
                pthread_join(m_fbi_thread, NULL);
                pthread_cond_destroy(&m_fbi_request);
                pthread_cond_destroy(&g_jpg_request);
                g_destroy = false;
            }
            m_loaded = false;
        }   // destroy
        // --------------------------------------------------------------------
        void load()
        {
            if (m_loaded) return;
            m_loaded = true;
            glGenBuffers(3, m_pbo);
            for (int i = 0; i < 3; i++)
            {
                glBindBuffer(GL_PIXEL_PACK_BUFFER, m_pbo[i]);
                glBufferData(GL_PIXEL_PACK_BUFFER,
                    irr_driver->getActualScreenSize().Width *
                    irr_driver->getActualScreenSize().Height * 4, NULL,
                GL_STREAM_READ);
            }
            glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
            pthread_cond_init(&m_fbi_request, NULL);
            pthread_cond_init(&g_jpg_request, NULL);
            m_common_thread_data.m_data = &m_fbi_queue;
            m_common_thread_data.m_request = &m_fbi_request;
            g_jpg_thread_data.m_data = &g_jpg_list;
            g_jpg_thread_data.m_request = &g_jpg_request;
            pthread_create(&m_fbi_thread, NULL, &fbiConversion,
                &m_common_thread_data);
        }   // load
    };
    // ========================================================================
    std::chrono::high_resolution_clock::time_point g_framerate_timer;
    // ========================================================================
    double g_accumulated_time;
    // ========================================================================
    CommonData g_common_data;
    // ========================================================================
    void setRecordingName(const std::string& name)
    {
        g_recording_name = name;
    }   // setRecordingName
    // ------------------------------------------------------------------------
    const std::string& getRecordingName()
    {
        return g_recording_name;
    }   // getRecordingName
    // ------------------------------------------------------------------------
    void prepareCapture()
    {
        g_common_data.load();
        g_common_data.m_pbo_use = 0;
        g_accumulated_time = 0.;
        assert(g_idle.getAtomic() && g_video_thread.getAtomic() == NULL);
        g_idle.setAtomic(false);
        pthread_create(&g_audio_thread, NULL, &Recorder::audioRecorder,
            &g_idle);
        g_video_thread.setAtomic(new pthread_t());
        VideoFormat vf = (VideoFormat)(int)UserConfigParams::m_record_format;
        switch (vf)
        {
        case VF_VP8:
        case VF_VP9:
            pthread_create(g_video_thread.getAtomic(), NULL,
                &Recorder::vpxEncoder, &g_jpg_thread_data);
            break;
        case VF_MJPEG:
            pthread_create(g_video_thread.getAtomic(), NULL,
                &Recorder::jpgWriter, &g_jpg_thread_data);
            break;
        case VF_H264:
            break;
        }
    }   // prepareCapture
    // ------------------------------------------------------------------------
    int getFrameCount(double rate)
    {
        const double frame_rate = 1. / double(UserConfigParams::m_record_fps);
        g_accumulated_time += rate;
        if (g_accumulated_time < frame_rate)
        {
            return 0;
        }
        int frame_count = 0;
        while (g_accumulated_time >= frame_rate)
        {
            frame_count++;
            g_accumulated_time = g_accumulated_time - frame_rate;
        }
        return frame_count;
    }   // getFrameCount
    // ------------------------------------------------------------------------
    void captureFrameBufferImage()
    {
        assert(g_common_data.m_loaded);
        int pbo_read = -1;
        if (g_common_data.m_pbo_use > 3 && g_common_data.m_pbo_use % 3 == 0)
            g_common_data.m_pbo_use = 3;
        auto rate = std::chrono::high_resolution_clock::now() -
            g_framerate_timer;
        g_framerate_timer = std::chrono::high_resolution_clock::now();
        glReadBuffer(GL_BACK);
        const unsigned width = irr_driver->getActualScreenSize().Width;
        const unsigned height = irr_driver->getActualScreenSize().Height;
        if (g_common_data.m_pbo_use >= 3)
        {
            int frame_count = getFrameCount(std::chrono::duration_cast
                <std::chrono::duration<double> >(rate).count());
            if (frame_count != 0)
            {
                pbo_read = g_common_data.m_pbo_use % 3;
                glBindBuffer(GL_PIXEL_PACK_BUFFER,
                    g_common_data.m_pbo[pbo_read]);
                void* ptr = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
                const unsigned size = width * height * 4;
                uint8_t* fbi = new uint8_t[size];
                memcpy(fbi, ptr, size);
                glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
                g_common_data.addFrameBufferImage(fbi, frame_count);
            }
        }
        int pbo_use = g_common_data.m_pbo_use++ % 3;
        assert(pbo_read == -1 || pbo_use == pbo_read);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, g_common_data.m_pbo[pbo_use]);
        glReadPixels(0, 0, width, height, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    }   // captureFrameBufferImage
    // ------------------------------------------------------------------------
    void stopRecording()
    {
        if (!isRecording())
        {
            g_common_data.addFrameBufferImage(NULL, -1);
        }
    }   // stopRecording
    // ------------------------------------------------------------------------
    bool isRecording()
    {
        return g_video_thread.getAtomic() == NULL;
    }   // isRecording
    // ------------------------------------------------------------------------
    void destroyRecorder()
    {
        g_destroy = true;
        stopRecording();
        g_common_data.destroy();
    }   // destroyRecorder

}
#endif
