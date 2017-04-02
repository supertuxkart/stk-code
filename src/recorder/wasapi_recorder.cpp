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

#if !(defined(SERVER_ONLY) || defined(USE_GLES2)) && defined(WIN32)

#include "recorder/vorbis_encoder.hpp"
#include "utils/synchronised.hpp"
#include "utils/log.hpp"
#include "utils/vs.hpp"

#include <list>

#include <audioclient.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <mmdeviceapi.h>
#include <windows.h>

#if defined (__MINGW32__) || defined(__CYGWIN__)
    #include "utils/types.hpp"
    inline GUID uuidFromString(const char* s)
    {
        unsigned long p0;
        unsigned int p1, p2, p3, p4, p5, p6, p7, p8, p9, p10;
        sscanf_s(s, "%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
            &p0, &p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8, &p9, &p10);
        GUID g = { p0, (uint16_t)p1, (uint16_t)p2, { (uint8_t)p3, (uint8_t)p4,
            (uint8_t)p5, (uint8_t)p6, (uint8_t)p7, (uint8_t)p8, (uint8_t)p9,
            (uint8_t)p10 }};
        return g;
    }
    #undef KSDATAFORMAT_SUBTYPE_PCM
    #define KSDATAFORMAT_SUBTYPE_PCM \
        uuidFromString("00000001-0000-0010-8000-00aa00389b71")
    #undef KSDATAFORMAT_SUBTYPE_IEEE_FLOAT
    #define KSDATAFORMAT_SUBTYPE_IEEE_FLOAT \
        uuidFromString("00000003-0000-0010-8000-00aa00389b71")
#endif

namespace Recorder
{
    // ========================================================================
    const REFERENCE_TIME REFTIMES_PER_SEC = 10000000;
    // ========================================================================
    struct WasapiData
    {
        bool m_loaded;
        IMMDeviceEnumerator* m_dev_enum;
        IMMDevice* m_dev;
        IAudioClient* m_client;
        IAudioCaptureClient* m_capture_client;
        WAVEFORMATEX* m_wav_format;
        uint32_t m_buffer_size;
        // --------------------------------------------------------------------
        WasapiData()
        {
            m_loaded = false;
            m_dev_enum = NULL;
            m_dev = NULL;
            m_client = NULL;
            m_capture_client = NULL;
            m_wav_format = NULL;
        }   // WasapiData
        // --------------------------------------------------------------------
        bool load()
        {
            HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL,
                CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
                (void**)&m_dev_enum);
            if (FAILED(hr))
                return false;

            hr = m_dev_enum->GetDefaultAudioEndpoint(eRender, eConsole,
                &m_dev);
            if (FAILED(hr))
                return false;

            hr = m_dev->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL,
                (void**)&m_client);
            if (FAILED(hr))
                return false;

            hr = m_client->GetMixFormat(&m_wav_format);
            if (FAILED(hr))
                return false;

            hr = m_client->Initialize(AUDCLNT_SHAREMODE_SHARED,
                AUDCLNT_STREAMFLAGS_LOOPBACK, REFTIMES_PER_SEC, 0,
                m_wav_format, NULL);
            if (FAILED(hr))
                return false;

            hr = m_client->GetBufferSize(&m_buffer_size);
            if (FAILED(hr))
                return false;

            hr = m_client->GetService(__uuidof(IAudioCaptureClient),
                (void**)&m_capture_client);
            if (FAILED(hr))
                return false;

            m_loaded = true;
            return true;
        }   // load
        // --------------------------------------------------------------------
        ~WasapiData()
        {
            if (m_loaded)
            {
                CoTaskMemFree(m_wav_format);
                if (m_dev_enum)
                    m_dev_enum->Release();
                if (m_dev)
                    m_dev->Release();
                if (m_client)
                    m_client->Release();
                if (m_capture_client)
                    m_capture_client->Release();
            }
        }   // ~WasapiData
    };
    // ========================================================================
    WasapiData g_wasapi_data;
    // ========================================================================
    void* audioRecorder(void *obj)
    {
        VS::setThreadName("audioRecorder");
        if (!g_wasapi_data.m_loaded)
        {
            if (!g_wasapi_data.load())
            {
                Log::error("WasapiRecorder", "Failed to load wasapi data");
                return NULL;
            }
        }
        VorbisEncoderData ved = {};
        if (g_wasapi_data.m_wav_format->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
        {
            WAVEFORMATEXTENSIBLE* wav_for_ext =
                (WAVEFORMATEXTENSIBLE*)g_wasapi_data.m_wav_format;
            ved.m_channels = wav_for_ext->Format.nChannels;
            ved.m_sample_rate = wav_for_ext->Format.nSamplesPerSec;
            if (IsEqualGUID(KSDATAFORMAT_SUBTYPE_PCM, wav_for_ext->SubFormat))
            {
                ved.m_audio_type = VorbisEncoderData::AT_PCM;
                if (wav_for_ext->Format.wBitsPerSample != 16)
                {
                    Log::error("WasapiRecorder", "Only 16bit PCM is"
                        " supported.");
                    return NULL;
                }
            }
            else if (IsEqualGUID(KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, wav_for_ext
                ->SubFormat))
            {
                ved.m_audio_type = VorbisEncoderData::AT_FLOAT;
                if (wav_for_ext->Format.wBitsPerSample != 32)
                {
                    Log::error("WasapiRecorder", "Only 32bit float is"
                        " supported.");
                    return NULL;
                }
            }
            else
            {
                Log::error("WasapiRecorder", "Unsupported audio input"
                    " format.");
                return NULL;
            }
        }
        else if (g_wasapi_data.m_wav_format->wFormatTag == WAVE_FORMAT_PCM)
        {
            ved.m_channels = g_wasapi_data.m_wav_format->nChannels;
            ved.m_sample_rate = g_wasapi_data.m_wav_format->nSamplesPerSec;
            ved.m_audio_type = VorbisEncoderData::AT_PCM;
            if (g_wasapi_data.m_wav_format->wBitsPerSample != 16)
            {
                Log::error("WasapiRecorder", "Only 16bit PCM is supported.");
                return NULL;
            }
        }
        else
        {
            Log::error("WasapiRecorder", "Unsupported audio input format");
            return NULL;
        }
        HRESULT hr = g_wasapi_data.m_client->Reset();
        if (FAILED(hr))
        {
            Log::error("WasapiRecorder", "Failed to reset recorder");
            return NULL;
        }
        hr = g_wasapi_data.m_client->Start();
        if (FAILED(hr))
        {
            Log::error("WasapiRecorder", "Failed to start recorder");
            return NULL;
        }
        REFERENCE_TIME duration = REFTIMES_PER_SEC *
            g_wasapi_data.m_buffer_size / g_wasapi_data.m_wav_format
            ->nSamplesPerSec;

        Synchronised<bool>* idle = (Synchronised<bool>*)obj;
        Synchronised<std::list<int8_t*> > audio_data;
        pthread_cond_t enc_request;
        pthread_cond_init(&enc_request, NULL);
        pthread_t vorbis_enc;
        ved.m_data = &audio_data;
        ved.m_enc_request = &enc_request;
        pthread_create(&vorbis_enc, NULL, &Recorder::vorbisEncoder, &ved);
        const unsigned frag_size = 1024 * ved.m_channels *
            (g_wasapi_data.m_wav_format->wBitsPerSample / 8);
        int8_t* each_audio_buf = new int8_t[frag_size]();
        unsigned readed = 0;
        while (true)
        {
            if (idle->getAtomic())
            {
                audio_data.lock();
                audio_data.getData().push_back(each_audio_buf);
                audio_data.getData().push_back(NULL);
                pthread_cond_signal(&enc_request);
                audio_data.unlock();
                break;
            }
            uint32_t packet_length = 0;
            hr = g_wasapi_data.m_capture_client->GetNextPacketSize(
                &packet_length);
            if (FAILED(hr))
            {
                Log::warn("WasapiRecorder", "Failed to get next packet size");
            }
            if (packet_length == 0)
            {
                REFERENCE_TIME sleep_time = duration / 10000 / 2;
                Sleep((uint32_t)sleep_time);
                continue;
            }
            BYTE* data;
            DWORD flags;
            hr = g_wasapi_data.m_capture_client->GetBuffer(&data,
                &packet_length, &flags, NULL, NULL);
            if (FAILED(hr))
            {
                Log::warn("WasapiRecorder", "Failed to get buffer");
            }
            const unsigned bytes = ved.m_channels * (g_wasapi_data.m_wav_format
                ->wBitsPerSample / 8) * packet_length;
            bool buf_full = readed + bytes > frag_size;
            unsigned copy_size = buf_full ? frag_size - readed : bytes;
            if (!(flags & AUDCLNT_BUFFERFLAGS_SILENT))
            {
                memcpy(each_audio_buf + readed, data, copy_size);
            }
            if (buf_full)
            {
                audio_data.lock();
                audio_data.getData().push_back(each_audio_buf);
                pthread_cond_signal(&enc_request);
                audio_data.unlock();
                each_audio_buf = new int8_t[frag_size]();
                readed = bytes - copy_size;
                if (!(flags & AUDCLNT_BUFFERFLAGS_SILENT))
                {
                    memcpy(each_audio_buf, (uint8_t*)data + copy_size, readed);
                }
            }
            else
            {
                readed += bytes;
            }
            hr = g_wasapi_data.m_capture_client->ReleaseBuffer(packet_length);
            if (FAILED(hr))
            {
                Log::warn("WasapiRecorder", "Failed to release buffer");
            }
        }
        hr = g_wasapi_data.m_client->Stop();
        if (FAILED(hr))
        {
            Log::warn("WasapiRecorder", "Failed to stop recorder");
        }
        pthread_join(vorbis_enc, NULL);
        pthread_cond_destroy(&enc_request);

        return NULL;
    }   // audioRecorder
}
#endif
