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

#if !(defined(SERVER_ONLY) || defined(USE_GLES2)) && !defined(WIN32)

#include "recorder/vorbis_encoder.hpp"
#include "utils/synchronised.hpp"
#include "utils/log.hpp"
#include "utils/vs.hpp"

#include <cstring>
#include <dlfcn.h>
#include <list>
#include <pulse/pulseaudio.h>
#include <string>

namespace Recorder
{
    // ========================================================================
    void serverInfoCallBack(pa_context* c, const pa_server_info* i, void* data)
    {
        *(std::string*)data = i->default_sink_name;
    }   // serverInfoCallBack
    // ========================================================================
    struct PulseAudioData
    {
        bool m_loaded;
        pa_mainloop* m_loop;
        pa_context* m_context;
        void* m_dl_handle;
        pa_sample_spec m_sample_spec;
        std::string m_default_sink;

        typedef pa_stream* (*pa_stream_new_t)(pa_context*, const char*,
            const pa_sample_spec*, const pa_channel_map*);
        pa_stream_new_t pa_stream_new;

        typedef int (*pa_stream_connect_record_t)(pa_stream*, const char*,
            const pa_buffer_attr*, pa_stream_flags_t);
        pa_stream_connect_record_t pa_stream_connect_record;

        typedef pa_stream_state_t (*pa_stream_get_state_t)(pa_stream*);
        pa_stream_get_state_t pa_stream_get_state;

        typedef size_t (*pa_stream_readable_size_t)(pa_stream*);
        pa_stream_readable_size_t pa_stream_readable_size;

        typedef int (*pa_stream_peek_t)(pa_stream*, const void**, size_t*);
        pa_stream_peek_t pa_stream_peek;

        typedef int (*pa_stream_drop_t)(pa_stream*);
        pa_stream_drop_t pa_stream_drop;

        typedef int (*pa_stream_disconnect_t)(pa_stream*);
        pa_stream_disconnect_t pa_stream_disconnect;

        typedef void (*pa_stream_unref_t)(pa_stream*);
        pa_stream_unref_t pa_stream_unref;

        typedef pa_mainloop* (*pa_mainloop_new_t)(void);
        pa_mainloop_new_t pa_mainloop_new;

        typedef pa_mainloop_api* (*pa_mainloop_get_api_t)(pa_mainloop*);
        pa_mainloop_get_api_t pa_mainloop_get_api;

        typedef pa_context* (*pa_context_new_t)(pa_mainloop_api*, const char*);
        pa_context_new_t pa_context_new;

        typedef int (*pa_context_connect_t)(pa_context*, const char*,
            pa_context_flags_t, const pa_spawn_api*);
        pa_context_connect_t pa_context_connect;

        typedef int (*pa_mainloop_iterate_t)(pa_mainloop*, int, int*);
        pa_mainloop_iterate_t pa_mainloop_iterate;

        typedef pa_context_state_t (*pa_context_get_state_t)(pa_context*);
        pa_context_get_state_t pa_context_get_state;

        typedef pa_operation* (*pa_context_get_server_info_t)(pa_context*,
            pa_server_info_cb_t, void*);
        pa_context_get_server_info_t pa_context_get_server_info;

        typedef pa_operation_state_t (*pa_operation_get_state_t)
            (pa_operation*);
        pa_operation_get_state_t pa_operation_get_state;

        typedef void (*pa_operation_unref_t)(pa_operation*);
        pa_operation_unref_t pa_operation_unref;

        typedef void (*pa_context_disconnect_t)(pa_context*);
        pa_context_disconnect_t pa_context_disconnect;

        typedef void (*pa_context_unref_t)(pa_context*);
        pa_context_unref_t pa_context_unref;

        typedef void (*pa_mainloop_free_t)(pa_mainloop*);
        pa_mainloop_free_t pa_mainloop_free;
        // --------------------------------------------------------------------
        PulseAudioData()
        {
            m_loaded = false;
            m_loop = NULL;
            m_context = NULL;
            m_dl_handle = NULL;
            pa_stream_new = NULL;
            pa_stream_connect_record = NULL;
            pa_stream_get_state = NULL;
            pa_stream_readable_size = NULL;
            pa_stream_peek = NULL;
            pa_stream_drop = NULL;
            pa_stream_disconnect = NULL;
            pa_stream_unref = NULL;
            pa_mainloop_new = NULL;
            pa_mainloop_get_api = NULL;
            pa_context_new = NULL;
            pa_context_connect = NULL;
            pa_mainloop_iterate = NULL;
            pa_context_get_state = NULL;
            pa_context_get_server_info = NULL;
            pa_operation_get_state = NULL;
            pa_operation_unref = NULL;
            pa_context_disconnect = NULL;
            pa_context_unref = NULL;
            pa_mainloop_free = NULL;
        }   // PulseAudioData
        // --------------------------------------------------------------------
        bool loadPulseAudioLibrary()
        {
            m_dl_handle = dlopen("libpulse.so", RTLD_LAZY);
            if (m_dl_handle == NULL)
            {
                Log::error("PulseAudioRecorder", "Failed to open PulseAudio"
                " library");
                return false;
            }
            pa_stream_new = (pa_stream_new_t)dlsym(m_dl_handle,
                "pa_stream_new");
            if (pa_stream_new == NULL)
            {
                Log::error("PulseAudioRecorder", "Cannot load function"
                    " 'pa_stream_new'");
                return false;
            }
            pa_stream_connect_record = (pa_stream_connect_record_t)dlsym
                (m_dl_handle, "pa_stream_connect_record");
            if (pa_stream_connect_record == NULL)
            {
                Log::error("PulseAudioRecorder", "Cannot load function"
                    " 'pa_stream_connect_record'");
                return false;
            }
            pa_stream_get_state = (pa_stream_get_state_t)dlsym(m_dl_handle,
                "pa_stream_get_state");
            if (pa_stream_get_state == NULL)
            {
                Log::error("PulseAudioRecorder", "Cannot load function"
                    " 'pa_stream_get_state'");
                return false;
            }
            pa_stream_readable_size = (pa_stream_readable_size_t)dlsym
                (m_dl_handle, "pa_stream_readable_size");
            if (pa_stream_readable_size == NULL)
            {
                Log::error("PulseAudioRecorder", "Cannot load function"
                    " 'pa_stream_readable_size'");
                return false;
            }
            pa_stream_peek = (pa_stream_peek_t)dlsym(m_dl_handle,
                "pa_stream_peek");
            if (pa_stream_peek == NULL)
            {
                Log::error("PulseAudioRecorder", "Cannot load function"
                    " 'pa_stream_peek'");
                return false;
            }
            pa_stream_drop = (pa_stream_drop_t)dlsym(m_dl_handle,
                "pa_stream_drop");
            if (pa_stream_drop == NULL)
            {
                Log::error("PulseAudioRecorder", "Cannot load function"
                    " 'pa_stream_drop'");
                return false;
            }
            pa_stream_disconnect = (pa_stream_disconnect_t)dlsym(m_dl_handle,
                "pa_stream_disconnect");
            if (pa_stream_disconnect == NULL)
            {
                Log::error("PulseAudioRecorder", "Cannot load function"
                    " 'pa_stream_disconnect'");
                return false;
            }
            pa_stream_unref = (pa_stream_unref_t)dlsym(m_dl_handle,
                "pa_stream_unref");
            if (pa_stream_unref == NULL)
            {
                Log::error("PulseAudioRecorder", "Cannot load function"
                    " 'pa_stream_unref'");
                return false;
            }
            pa_mainloop_new = (pa_mainloop_new_t)dlsym(m_dl_handle,
                "pa_mainloop_new");
            if (pa_mainloop_new == NULL)
            {
                Log::error("PulseAudioRecorder", "Cannot load function"
                    " 'pa_mainloop_new'");
                return false;
            }
            pa_mainloop_get_api = (pa_mainloop_get_api_t)dlsym(m_dl_handle,
                "pa_mainloop_get_api");
            if (pa_mainloop_get_api == NULL)
            {
                Log::error("PulseAudioRecorder", "Cannot load function"
                    " 'pa_mainloop_get_api'");
                return false;
            }
            pa_context_new = (pa_context_new_t)dlsym(m_dl_handle,
                "pa_context_new");
            if (pa_context_new == NULL)
            {
                Log::error("PulseAudioRecorder", "Cannot load function"
                    " 'pa_context_new'");
                return false;
            }
            pa_context_connect = (pa_context_connect_t)dlsym(m_dl_handle,
                "pa_context_connect");
            if (pa_context_connect == NULL)
            {
                Log::error("PulseAudioRecorder", "Cannot load function"
                    " 'pa_context_connect'");
                return false;
            }
            pa_mainloop_iterate = (pa_mainloop_iterate_t)dlsym(m_dl_handle,
                "pa_mainloop_iterate");
            if (pa_mainloop_iterate == NULL)
            {
                Log::error("PulseAudioRecorder", "Cannot load function"
                    " 'pa_mainloop_iterate'");
                return false;
            }
            pa_context_get_state = (pa_context_get_state_t)dlsym(m_dl_handle,
                "pa_context_get_state");
            if (pa_context_get_state == NULL)
            {
                Log::error("PulseAudioRecorder", "Cannot load function"
                    " 'pa_context_get_state'");
                return false;
            }
            pa_context_get_server_info = (pa_context_get_server_info_t)dlsym
                (m_dl_handle, "pa_context_get_server_info");
            if (pa_context_get_server_info == NULL)
            {
                Log::error("PulseAudioRecorder", "Cannot load function"
                    " 'pa_context_get_server_info'");
                return false;
            }
            pa_operation_get_state = (pa_operation_get_state_t)dlsym
                (m_dl_handle, "pa_operation_get_state");
            if (pa_operation_get_state == NULL)
            {
                Log::error("PulseAudioRecorder", "Cannot load function"
                    " 'pa_operation_get_state'");
                return false;
            }
            pa_operation_unref = (pa_operation_unref_t)dlsym(m_dl_handle,
                "pa_operation_unref");
            if (pa_operation_unref == NULL)
            {
                Log::error("PulseAudioRecorder", "Cannot load function"
                    " 'pa_operation_unref'");
                return false;
            }
            pa_context_disconnect = (pa_context_disconnect_t)dlsym(m_dl_handle,
                "pa_context_disconnect");
            if (pa_context_disconnect == NULL)
            {
                Log::error("PulseAudioRecorder", "Cannot load function"
                    " 'pa_context_disconnect'");
                return false;
            }
            pa_context_unref = (pa_context_unref_t)dlsym(m_dl_handle,
                "pa_context_unref");
            if (pa_context_unref == NULL)
            {
                Log::error("PulseAudioRecorder", "Cannot load function"
                    " 'pa_context_unref'");
                return false;
            }
            pa_mainloop_free = (pa_mainloop_free_t)dlsym(m_dl_handle,
                "pa_mainloop_free");
            if (pa_mainloop_free == NULL)
            {
                Log::error("PulseAudioRecorder", "Cannot load function"
                    " 'pa_mainloop_free'");
                return false;
            }
            return true;
        }   // loadPulseAudioLibrary
        // --------------------------------------------------------------------
        bool load()
        {
            if (!loadPulseAudioLibrary())
            {
                if (m_dl_handle != NULL)
                {
                    dlclose(m_dl_handle);
                    m_dl_handle = NULL;
                }
                return false;
            }
            m_loop = pa_mainloop_new();
            if (m_loop == NULL)
            {
                Log::error("PulseAudioRecorder", "Failed to create mainloop");
                return false;
            }
            m_context = pa_context_new(pa_mainloop_get_api(m_loop),
                "audioRecord");
            if (m_context == NULL)
            {
                Log::error("PulseAudioRecorder", "Failed to create context");
                return false;
            }
            pa_context_connect(m_context, NULL, PA_CONTEXT_NOAUTOSPAWN , NULL);
            while (true)
            {
                while (pa_mainloop_iterate(m_loop, 0, NULL) > 0);
                pa_context_state_t state = pa_context_get_state(m_context);
                if (state == PA_CONTEXT_READY)
                    break;
                if (!PA_CONTEXT_IS_GOOD(state))
                {
                    Log::error("PulseAudioRecorder", "Failed to connect to"
                        " context");
                    return false;
                }
            }
            pa_operation* pa_op = pa_context_get_server_info(m_context,
                serverInfoCallBack, &m_default_sink);
            enum pa_operation_state op_state;
            while ((op_state =
                pa_operation_get_state(pa_op)) == PA_OPERATION_RUNNING)
            pa_mainloop_iterate(m_loop, 0, NULL);
            pa_operation_unref(pa_op);
            if (m_default_sink.empty())
            {
                Log::error("PulseAudioRecorder", "Failed to get default sink");
                return false;
            }
            m_default_sink += ".monitor";
            m_sample_spec.format = PA_SAMPLE_S16LE;
            m_sample_spec.rate = 44100;
            m_sample_spec.channels = 2;

            m_loaded = true;
            return true;
        }   // load
        // --------------------------------------------------------------------
        ~PulseAudioData()
        {
            if (m_loaded)
            {
                if (m_context != NULL)
                {
                    pa_context_disconnect(m_context);
                    pa_context_unref(m_context);
                }
                if (m_loop != NULL)
                {
                    pa_mainloop_free(m_loop);
                }
                if (m_dl_handle != NULL)
                {
                    dlclose(m_dl_handle);
                }
            }
        }   // ~PulseAudioData
    };
    // ========================================================================
    PulseAudioData g_pa_data;
    // ========================================================================
    void* audioRecorder(void *obj)
    {
        VS::setThreadName("audioRecorder");
        if (!g_pa_data.m_loaded)
        {
            if (!g_pa_data.load())
            {
                Log::error("PulseAudioRecord", "Cannot pulseaudio data");
                return NULL;
            }
        }

        pa_stream* stream = g_pa_data.pa_stream_new(g_pa_data.m_context,
            "input", &g_pa_data.m_sample_spec, NULL);
        if (stream == NULL)
        {
            Log::error("PulseAudioRecorder", "Failed to create stream");
            return NULL;
        }
        pa_buffer_attr buf_attr;
        const unsigned frag_size = 1024 * g_pa_data.m_sample_spec.channels *
            sizeof(int16_t);
        buf_attr.fragsize = frag_size;
        const unsigned max_uint = -1;
        buf_attr.maxlength = max_uint;
        buf_attr.minreq = max_uint;
        buf_attr.prebuf = max_uint;
        buf_attr.tlength = max_uint;
        g_pa_data.pa_stream_connect_record(stream,
            g_pa_data.m_default_sink.c_str(), &buf_attr,
            (pa_stream_flags_t)(PA_STREAM_ADJUST_LATENCY));

        while (true)
        {
            while (g_pa_data.pa_mainloop_iterate(g_pa_data.m_loop, 0, NULL)
                > 0);
            pa_stream_state_t state = g_pa_data.pa_stream_get_state(stream);
            if (state == PA_STREAM_READY)
                break;
            if (!PA_STREAM_IS_GOOD(state))
            {
                Log::error("PulseAudioRecorder", "Failed to connect to"
                    " stream");
                return NULL;
            }
        }

        Synchronised<bool>* idle = (Synchronised<bool>*)obj;
        Synchronised<std::list<int8_t*> > pcm_data;
        pthread_cond_t enc_request;
        pthread_cond_init(&enc_request, NULL);
        pthread_t vorbis_enc;

        Recorder::VorbisEncoderData ved;
        ved.m_sample_rate = g_pa_data.m_sample_spec.rate;
        ved.m_channels = g_pa_data.m_sample_spec.channels;
        ved.m_audio_type = Recorder::VorbisEncoderData::AT_PCM;
        ved.m_data = &pcm_data;
        ved.m_enc_request = &enc_request;
        pthread_create(&vorbis_enc, NULL, &Recorder::vorbisEncoder, &ved);
        int8_t* each_pcm_buf = new int8_t[frag_size]();
        unsigned readed = 0;
        while (true)
        {
            if (idle->getAtomic())
            {
                pcm_data.lock();
                pcm_data.getData().push_back(each_pcm_buf);
                pthread_cond_signal(&enc_request);
                pcm_data.unlock();
                break;
            }
            while (g_pa_data.pa_mainloop_iterate(g_pa_data.m_loop, 0, NULL)
                > 0);
            const void* data;
            size_t bytes;
            size_t readable = g_pa_data.pa_stream_readable_size(stream);
            if (readable == 0)
                continue;
            g_pa_data.pa_stream_peek(stream, &data, &bytes);
            if (data == NULL)
            {
                if (bytes > 0)
                    g_pa_data.pa_stream_drop(stream);
                continue;
            }
            bool buf_full = readed + (unsigned)bytes > frag_size;
            unsigned copy_size = buf_full ?
                frag_size - readed : (unsigned)bytes;
            memcpy(each_pcm_buf + readed, data, copy_size);
            if (buf_full)
            {
                pcm_data.lock();
                pcm_data.getData().push_back(each_pcm_buf);
                pthread_cond_signal(&enc_request);
                pcm_data.unlock();
                each_pcm_buf = new int8_t[frag_size]();
                readed = (unsigned)bytes - copy_size;
                memcpy(each_pcm_buf, (uint8_t*)data + copy_size, readed);
            }
            else
            {
                readed += (unsigned)bytes;
            }
            g_pa_data.pa_stream_drop(stream);
        }
        pcm_data.lock();
        pcm_data.getData().push_back(NULL);
        pthread_cond_signal(&enc_request);
        pcm_data.unlock();
        pthread_join(vorbis_enc, NULL);
        pthread_cond_destroy(&enc_request);
        g_pa_data.pa_stream_disconnect(stream);
        g_pa_data.pa_stream_unref(stream);
        return NULL;
    }   // audioRecorder
}
#endif
