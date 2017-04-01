//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2015 Dawid Gan
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

#include "utils/avi_writer.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/message_queue.hpp"
#include "recorder/pulseaudio_recorder.hpp"
#include "recorder/vorbis_encoder.hpp"
#include "recorder/wasapi_recorder.hpp"
#include "recorder/webm_writer.hpp"
#include "utils/translation.hpp"
#include "utils/vs.hpp"

#include <turbojpeg.h>
#include <vpx/vpx_encoder.h>
#include <vpx/vp8cx.h>

#include <jpeglib.h>
#include <cstring>

Synchronised<std::string> AVIWriter::m_recording_target("");
// ----------------------------------------------------------------------------
AVIWriter::AVIWriter() : m_idle(true)
{
    resetFrameBufferImage();
    resetCaptureFormat();
    m_file = NULL;
    m_last_junk_chunk = 0;
    m_end_of_header = 0;
    m_movi_start = 0;
    m_stream_bytes = 0;
    m_total_frames = 0;
    m_chunk_fcc = 0;
    m_width = irr_driver->getActualScreenSize().Width;
    m_height = irr_driver->getActualScreenSize().Height;
    glGenBuffers(3, m_pbo);
    for (int i = 0; i < 3; i++)
    {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, m_pbo[i]);
        glBufferData(GL_PIXEL_PACK_BUFFER, m_width * m_height * 4, NULL,
            GL_STREAM_READ);
    }
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    pthread_cond_init(&m_cond_request, NULL);
    pthread_create(&m_record_thread, NULL, &videoRecord, this);
}   // AVIWriter

// ----------------------------------------------------------------------------
AVIWriter::~AVIWriter()
{
    glDeleteBuffers(3, m_pbo);
    addFrameBufferImage(NULL, 0);
    if (!waitForReadyToDeleted(2.0f))
        Log::info("AVIWriter", "AVIWriter not stopping, exiting anyway.");
    pthread_join(m_record_thread, NULL);
    pthread_cond_destroy(&m_cond_request);
}   // ~AVIWriter

// ----------------------------------------------------------------------------
void AVIWriter::resetFrameBufferImage()
{
    m_pbo_use = 0;
    m_accumulated_time = 0.0f;
}   // resetFrameBufferImage

// ----------------------------------------------------------------------------
int bmpToJPG(uint8_t* raw, unsigned width, unsigned height,
             uint8_t** jpeg_buffer, unsigned long* jpeg_size)
{
    tjhandle handle = NULL;
    int ret = 0;
    handle = tjInitCompress();
#ifdef TJFLAG_FASTDCT
    ret = tjCompress2(handle, raw, width, 0, height, TJPF_BGR, jpeg_buffer,
        jpeg_size, TJSAMP_420, UserConfigParams::m_recorder_jpg_quality,
        TJFLAG_FASTDCT);
#else
    ret = tjCompress2(handle, raw, width, 0, height, TJPF_BGR, jpeg_buffer,
        jpeg_size, TJSAMP_420, UserConfigParams::m_recorder_jpg_quality, 0);
#endif
    if (ret != 0)
    {
        char* err = tjGetErrorStr();
        Log::error("vpxEncoder", "Jpeg encode error: %s.", err);
        return ret;
    }
    tjDestroy(handle);
    return ret;
}   // bmpToJPG

// ----------------------------------------------------------------------------
int jpgToYuv(uint8_t* jpeg_buffer, unsigned jpeg_size, uint8_t** yuv_buffer,
             TJSAMP* yuv_type, unsigned* yuv_size)
{
    tjhandle handle = NULL;
    int width, height;
    TJSAMP subsample;
    int ret = 0;
    handle = tjInitDecompress();
    ret = tjDecompressHeader2(handle, jpeg_buffer, jpeg_size, &width, &height,
          (int*)&subsample);
    if (ret != 0)
    {
        char* err = tjGetErrorStr();
        Log::error("vpxEncoder", "Jpeg decode error: %s.", err);
        return ret;
    }

    *yuv_type = subsample;
    *yuv_size = tjBufSizeYUV(width, height, subsample);
    *yuv_buffer = new uint8_t[*yuv_size];
    ret = tjDecompressToYUV(handle, jpeg_buffer, jpeg_size, *yuv_buffer, 0);
    if (ret != 0)
    {
        char* err = tjGetErrorStr();
        Log::error("vpxEncoder", "YUV conversion error: %s.", err);
        return ret;
    }
    tjDestroy(handle);

    return ret;
}   // jpgToYuv

// ----------------------------------------------------------------------------
int vpxEncodeFrame(vpx_codec_ctx_t *codec, vpx_image_t *img, int frame_index,
                   FILE *out)
{
    int got_pkts = 0;
    vpx_codec_iter_t iter = NULL;
    const vpx_codec_cx_pkt_t *pkt = NULL;
    const vpx_codec_err_t res = vpx_codec_encode(codec, img, frame_index, 1, 0,
        VPX_DL_REALTIME);
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
            fwrite(&pkt->data.frame.flags, 1, sizeof(vpx_codec_frame_flags_t),
                out);
            fwrite(pkt->data.frame.buf, 1, pkt->data.frame.sz, out);
        }
    }
    return got_pkts;
}   // vpxEncodeFrame

// ----------------------------------------------------------------------------
void AVIWriter::resetCaptureFormat()
{
    m_img_quality = UserConfigParams::m_recorder_jpg_quality;
    m_msec_per_frame = unsigned(1000 / UserConfigParams::m_record_fps);
    m_avi_format = AVI_FORMAT_JPG;
}   // resetCaptureFormat

// ----------------------------------------------------------------------------
struct EncoderInfo
{
    void* m_data;
    pthread_cond_t* m_enc_request;
};   // EncoderInfo

// ----------------------------------------------------------------------------
void* AVIWriter::vpxEncoder(void *obj)
{
    VS::setThreadName("vpxEncoder");
    FILE* vpx_data = fopen((m_recording_target.getAtomic() + ".vp_data")
        .c_str(), "wb");
    if (vpx_data == NULL)
    {
        Log::error("vorbisEncoder", "Failed to encode ogg file");
        return NULL;
    }
    EncoderInfo* ei = (EncoderInfo*)obj;
    Synchronised<std::list<std::tuple<uint8_t*, unsigned, int> > >* jpg_data =
        (Synchronised<std::list<std::tuple<uint8_t*, unsigned, int> > >*)
        ei->m_data;
    pthread_cond_t* cond_request = ei->m_enc_request;

    vpx_codec_ctx_t codec;
    vpx_codec_enc_cfg_t cfg;
    vpx_codec_err_t res = vpx_codec_enc_config_default(vpx_codec_vp8_cx(),
       &cfg, 0);
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
    if (vpx_codec_enc_init(&codec, vpx_codec_vp8_cx(), &cfg, 0) > 0)
    {
        Log::error("vpxEncoder", "Failed to initialize encoder");
        fclose(vpx_data);
        return NULL;
    }

    while (true)
    {
        jpg_data->lock();
        bool waiting = jpg_data->getData().empty();
        while (waiting)
        {
            pthread_cond_wait(cond_request, jpg_data->getMutex());
            waiting = jpg_data->getData().empty();
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
        TJSAMP yuv_type;
        unsigned yuv_size;
        int ret = jpgToYuv(jpg, jpg_size, &yuv, &yuv_type, &yuv_size);
        if (ret < 0)
        {
            delete [] yuv;
            tjFree(jpg);
            continue;
        }
        assert(yuv_type == TJSAMP_420 && yuv_size != 0);
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


// ----------------------------------------------------------------------------
Synchronised<std::list<std::tuple<uint8_t*, unsigned, int> > > jpg_data;
pthread_cond_t vpx_enc_request;
pthread_t audio_thread, vpx_enc_thread;
EncoderInfo vpx_ei;
// ----------------------------------------------------------------------------
void* AVIWriter::videoRecord(void *obj)
{
    VS::setThreadName("videoRecord");
    vpx_ei.m_data = &jpg_data;
    vpx_ei.m_enc_request = &vpx_enc_request;
    AVIWriter* avi_writer = (AVIWriter*)obj;
    while (true)
    {
        avi_writer->m_fbi_queue.lock();
        bool waiting = avi_writer->m_fbi_queue.getData().empty();
        while (waiting)
        {
            pthread_cond_wait(&avi_writer->m_cond_request,
                avi_writer->m_fbi_queue.getMutex());
            waiting = avi_writer->m_fbi_queue.getData().empty();
        }
        auto& p =  avi_writer->m_fbi_queue.getData().front();
        uint8_t* fbi = p.first;
        int frame_count = p.second;
        if (frame_count == -1)
        {
            avi_writer->m_idle.setAtomic(true);
            jpg_data.lock();
            jpg_data.getData().emplace_back((uint8_t*)NULL, 0, 0);
            pthread_cond_signal(vpx_ei.m_enc_request);
            jpg_data.unlock();
            pthread_join(audio_thread, NULL);
            pthread_join(vpx_enc_thread, NULL);
            Recorder::writeWebm(m_recording_target.getAtomic() + ".vp_data",
                m_recording_target.getAtomic() + ".vb_data");
            avi_writer->m_fbi_queue.getData().clear();
            avi_writer->m_fbi_queue.unlock();
            continue;
        }
        else if (fbi == NULL)
        {
            avi_writer->m_idle.setAtomic(true);
            jpg_data.lock();
            jpg_data.getData().emplace_back((uint8_t*)NULL, 0, 0);
            pthread_cond_signal(vpx_ei.m_enc_request);
            jpg_data.unlock();
            //pthread_join(audio_thread, NULL);
            //pthread_join(vpx_enc_thread, NULL);
            avi_writer->setCanBeDeleted();
            avi_writer->m_fbi_queue.getData().clear();
            avi_writer->m_fbi_queue.unlock();
            return NULL;
        }
        const bool too_slow = avi_writer->m_fbi_queue.getData().size() > 50;
        avi_writer->m_fbi_queue.getData().pop_front();
        avi_writer->m_fbi_queue.unlock();
        if (too_slow)
        {
            MessageQueue::add(MessageQueue::MT_ERROR,
                _("Encoding is too slow, dropping frames."));
            delete [] fbi;
            avi_writer->cleanAllFrameBufferImages();
            continue;
        }
        uint8_t* orig_fbi = fbi;
        const unsigned width = avi_writer->m_width;
        const unsigned height = avi_writer->m_height;
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
        jpg_data.lock();
        jpg_data.getData().emplace_back(jpg, jpg_size, frame_count);
        pthread_cond_signal(vpx_ei.m_enc_request);
        jpg_data.unlock();
    }
    return NULL;
}   // videoRecord

// ----------------------------------------------------------------------------
int AVIWriter::getFrameCount(double rate)
{
    const double frame_rate = 1. / double(UserConfigParams::m_record_fps);
    m_accumulated_time += rate;
    if (m_accumulated_time < frame_rate)
    {
        return 0;
    }
    int frame_count = 0;
    while (m_accumulated_time >= frame_rate)
    {
        frame_count++;
        m_accumulated_time = m_accumulated_time - frame_rate;
    }
    return frame_count;
}   // getFrameCount

// ----------------------------------------------------------------------------
void AVIWriter::captureFrameBufferImage()
{
    if (m_idle.getAtomic())
    {
        m_idle.setAtomic(false);
        pthread_create(&audio_thread, NULL, &Recorder::audioRecorder, &m_idle);
        pthread_cond_init(vpx_ei.m_enc_request, NULL);
        pthread_create(&vpx_enc_thread, NULL, &vpxEncoder, &vpx_ei);
    }
    int pbo_read = -1;
    if (m_pbo_use > 3 && m_pbo_use % 3 == 0)
        m_pbo_use = 3;
    auto rate = std::chrono::high_resolution_clock::now() - m_framerate_timer;
    m_framerate_timer = std::chrono::high_resolution_clock::now();
    glReadBuffer(GL_BACK);
    if (m_pbo_use >= 3)
    {
        int frame_count = getFrameCount(std::chrono::duration_cast
            <std::chrono::duration<double> >(rate).count());
        if (frame_count != 0)
        {
            pbo_read = m_pbo_use % 3;
            glBindBuffer(GL_PIXEL_PACK_BUFFER, m_pbo[pbo_read]);
            void* ptr = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
            const unsigned size = m_width * m_height * 4;
            uint8_t* fbi = new uint8_t[size];
            memcpy(fbi, ptr, size);
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
            addFrameBufferImage(fbi, frame_count);
        }
    }
    int pbo_use = m_pbo_use++ % 3;
    assert(pbo_read == -1 || pbo_use == pbo_read);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, m_pbo[pbo_use]);
    glReadPixels(0, 0, m_width, m_height, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}   // captureFrameBufferImage

// ----------------------------------------------------------------------------
bool AVIWriter::addJUNKChunk(std::string str, unsigned int min_size)
{
    int size = str.size() < min_size ? min_size : str.size() + 1;
    size = (size + 1) & 0xfffffffe;

    CHUNK chunk;
    chunk.fcc = FOURCC('J', 'U', 'N', 'K');
    chunk.cb = size;

    char* buffer = (char*)calloc(size, 1);
    strcpy(buffer, str.c_str());

    int num = fwrite(&chunk, 1, sizeof(chunk), m_file);
    if (num != sizeof(chunk))
        goto error;

    num = fwrite(buffer, 1, size * sizeof(char), m_file);
    free(buffer);
    if (num != size)
        goto error;

    m_last_junk_chunk = ftell(m_file);
    if (m_last_junk_chunk < 0)
        goto error;

    return true;

error:
    closeFile(true/*delete_file*/);
    return false;
}   // addJUNKChunk

// ----------------------------------------------------------------------------
AVIErrCode AVIWriter::addImage(unsigned char* buffer, int buf_size)
{
    if (m_file == NULL)
        goto error;

    int num; num = ftell(m_file);
    if (num < 0)
        goto error;

    if (m_total_frames >= (unsigned)MAX_FRAMES)
        goto size_limit;

    CHUNK chunk;
    chunk.fcc = m_chunk_fcc;
    chunk.cb = buf_size;

    m_index_table[m_total_frames].Offset = num;
    m_index_table[m_total_frames].Length = chunk.cb;
    m_index_table[m_total_frames].fcc = chunk.fcc;

    num = fwrite(&chunk, 1, sizeof(chunk), m_file);
    if (num != sizeof(chunk))
        goto error;

    num = fwrite(buffer, 1, buf_size, m_file);
    if (num != buf_size)
        goto error;

    int fill_size; fill_size = (sizeof(chunk) + buf_size) & 0x00000001;
    if (fill_size > 0)
    {
        uint32_t filler = 0;
        num = fwrite(&filler, 1, fill_size, m_file);
        if (num != fill_size)
            goto error;
    }

    m_stream_bytes += sizeof(chunk) + buf_size + fill_size;
    m_total_frames++;

    num = ftell(m_file);
    if (num < 0)
        goto error;

    if (((num - m_last_junk_chunk) > 20000) && (!addJUNKChunk("", 1)))
        goto error;

    // check if we reached the file size limit
    if (num >= MAX_FILE_SIZE)
        goto size_limit;

    return AVI_SUCCESS;

error:
    closeFile(true/*delete_file*/);
    return AVI_IO_ERR;

size_limit:
    MessageQueue::add(MessageQueue::MT_GENERIC,
       _("Video exceeded size limit, starting a new one."));
    closeFile();
    return AVI_SIZE_LIMIT_ERR;
}   // addImage

// ----------------------------------------------------------------------------
bool AVIWriter::closeFile(bool delete_file, bool exiting)
{
    if (m_file == NULL)
        return false;

    if (delete_file)
        goto error;

    // add the index
    int idx_start; idx_start = ftell(m_file);
    if (idx_start < 0)
        goto error;

    CHUNK chunk;
    chunk.fcc = FOURCC('i', 'd', 'x', '1');
    chunk.cb = sizeof(AVIINDEXENTRY) * m_total_frames;

    int num; num = fwrite(&chunk, 1, sizeof(chunk), m_file);
    if (num != sizeof(chunk))
        goto error;

    for (unsigned int i = 0; i < m_total_frames; i++)
    {
        AVIINDEXENTRY Index;
        Index.ckid = m_index_table[i].fcc;
        Index.dwFlags = AVIIF_KEYFRAME;
        Index.dwChunkOffset = m_index_table[i].Offset;
        Index.dwChunkLength = m_index_table[i].Length;

        num = fwrite(&Index, 1, sizeof(Index), m_file);
        if (num != sizeof(Index))
            goto error;
    }

    // update the header
    if (m_total_frames > 0 && m_msec_per_frame > 0)
    {
        num = fseek(m_file, 0, SEEK_END);
        if (num < 0)
            goto error;

        int size; size = ftell(m_file);
        if (size < 0)
            goto error;

        num = fseek(m_file, 0, SEEK_SET);
        if (num < 0)
            goto error;

        m_avi_hdr.riff.cb = size - sizeof(m_avi_hdr.riff);
        m_avi_hdr.avih.dwMaxBytesPerSec = (uint32_t)
            (((m_stream_bytes / m_total_frames) * m_format_hdr.strh.dwRate) /
            m_msec_per_frame + 0.5f);
        m_avi_hdr.avih.dwTotalFrames = m_total_frames;

        num = fwrite(&m_avi_hdr, 1, sizeof(m_avi_hdr), m_file);
        if (num != sizeof(m_avi_hdr))
            goto error;

        m_format_hdr.strh.dwLength = m_total_frames;

        num = fwrite(&m_format_hdr, 1, sizeof(m_format_hdr), m_file);
        if (num != sizeof(m_format_hdr))
            goto error;
    }

    // update the movi section
    m_movi_chunk.cb = idx_start - m_movi_start;

    num = fseek(m_file, m_movi_start - sizeof(m_movi_chunk), SEEK_SET);
    if (num < 0)
        goto error;

    num = fwrite(&m_movi_chunk, 1, sizeof(m_movi_chunk), m_file);
    if (num != sizeof(m_movi_chunk))
        goto error;

    fclose(m_file);
    m_file = NULL;

    if (!exiting)
    {
        MessageQueue::add(MessageQueue::MT_GENERIC,
            _("Video saved in \"%s\".", m_filename.c_str()));
    }
    return true;

error:
    if (!exiting)
    {
        MessageQueue::add(MessageQueue::MT_ERROR,
            _("Error when saving video."));
    }
    fclose(m_file);
    remove(m_filename.c_str());
    m_file = NULL;
    return false;
}   // closeFile

// ----------------------------------------------------------------------------
bool AVIWriter::createFile()
{
    time_t rawtime;
    time(&rawtime);
    tm* timeInfo = localtime(&rawtime);
    char time_buffer[256];
    sprintf(time_buffer, "%i.%02i.%02i_%02i.%02i.%02i",
        timeInfo->tm_year + 1900, timeInfo->tm_mon + 1,
        timeInfo->tm_mday, timeInfo->tm_hour,
        timeInfo->tm_min, timeInfo->tm_sec);

    m_filename = m_recording_target.getAtomic() + "-" + time_buffer + ".avi";
    m_stream_bytes = 0;
    m_total_frames = 0;
    m_movi_start = 0;
    m_last_junk_chunk = 0;

    BitmapInfoHeader bitmap_hdr;
    bitmap_hdr.biSize = sizeof(BitmapInfoHeader);
    bitmap_hdr.biWidth = m_width;
    bitmap_hdr.biHeight = m_height;
    bitmap_hdr.biPlanes = 1;
    bitmap_hdr.biBitCount = 24;
    bitmap_hdr.biCompression = 0;
    bitmap_hdr.biSizeImage = (m_width * m_height * 3 * bitmap_hdr.biPlanes);
    bitmap_hdr.biXPelsPerMeter = 0;
    bitmap_hdr.biYPelsPerMeter = 0;
    bitmap_hdr.biClrUsed = 0;
    bitmap_hdr.biClrImportant = 0;

    memset(&m_avi_hdr, '\0', sizeof(m_avi_hdr));
    m_avi_hdr.riff.fcc = FOURCC('R', 'I', 'F', 'F');
    m_avi_hdr.riff.cb = 0; // update when finished (size of the file - 8)
    m_avi_hdr.avi = FOURCC('A', 'V', 'I', ' ');
    m_avi_hdr.list1.fcc = FOURCC('L', 'I', 'S', 'T');
    m_avi_hdr.list1.cb = 0;
    m_avi_hdr.hdrl = FOURCC('h', 'd', 'r', 'l');
    m_avi_hdr.avihhdr.fcc = FOURCC('a', 'v', 'i', 'h');
    m_avi_hdr.avihhdr.cb = sizeof(m_avi_hdr.avih);
    m_avi_hdr.avih.dwMicroSecPerFrame = m_msec_per_frame * 1000;
    m_avi_hdr.avih.dwMaxBytesPerSec = 0; // update when finished
    m_avi_hdr.avih.dwPaddingGranularity = 0;
    m_avi_hdr.avih.dwFlags = AVIF_WASCAPTUREFILE | AVIF_HASINDEX;
    m_avi_hdr.avih.dwTotalFrames = 0; // update when finished
    m_avi_hdr.avih.dwInitialFrames = 0;
    m_avi_hdr.avih.dwStreams = 1; // 1 = video, 2 = video and audio
    m_avi_hdr.avih.dwSuggestedBufferSize = 0; // can be just 0
    m_avi_hdr.avih.dwWidth = m_width;
    m_avi_hdr.avih.dwHeight = m_height;

    m_format_hdr.list.fcc = FOURCC('L', 'I', 'S', 'T');
    m_format_hdr.list.cb = (sizeof(m_format_hdr) - 8) +
        sizeof(BitmapInfoHeader);
    m_format_hdr.strl = FOURCC('s', 't', 'r', 'l');
    m_format_hdr.strhhdr.fcc = FOURCC('s', 't', 'r', 'h');
    m_format_hdr.strhhdr.cb = sizeof(m_format_hdr.strh);
    m_format_hdr.strh.fccType = FOURCC('v', 'i', 'd', 's');
    m_format_hdr.strh.fccHandler = CC_DIB;
    m_format_hdr.strh.dwFlags = 0;
    m_format_hdr.strh.wPriority = 0;
    m_format_hdr.strh.wLanguage = 0;
    m_format_hdr.strh.dwInitialFrames = 0;
    m_format_hdr.strh.dwScale = m_msec_per_frame;
    m_format_hdr.strh.dwRate = 1000;
    m_format_hdr.strh.dwStart = 0;
    m_format_hdr.strh.dwLength = 0; // update when finished
    m_format_hdr.strh.dwSuggestedBufferSize = 0; // can be just 0
    m_format_hdr.strh.dwQuality = m_img_quality * 100;
    m_format_hdr.strh.dwSampleSize = 0;
    m_format_hdr.strh.Left = 0;
    m_format_hdr.strh.Top = 0;
    m_format_hdr.strh.Right = m_avi_hdr.avih.dwWidth;
    m_format_hdr.strh.Bottom = m_avi_hdr.avih.dwHeight;
    m_format_hdr.strfhdr.fcc = FOURCC('s', 't', 'r', 'f');
    m_format_hdr.strfhdr.cb = sizeof(BitmapInfoHeader);

    // Format specific changes
    if (m_avi_format == AVI_FORMAT_JPG)
    {
        m_format_hdr.strh.fccHandler = CC_MJPG;
        bitmap_hdr.biCompression = FOURCC('M', 'J', 'P', 'G');
        m_chunk_fcc = FOURCC('0', '0', 'd', 'c');
    }
    else if (m_avi_format == AVI_FORMAT_BMP)
    {
        bitmap_hdr.biHeight = -m_height;
        bitmap_hdr.biCompression = 0;
        m_chunk_fcc = FOURCC('0', '0', 'd', 'b');
    }

    const uint32_t fcc_movi = FOURCC('m', 'o', 'v', 'i');

    m_file = fopen(m_filename.c_str(), "wb");
    if (m_file == NULL)
        return false;

    int num = fwrite(&m_avi_hdr, 1, sizeof(m_avi_hdr), m_file);
    if (num != sizeof(m_avi_hdr))
        goto error;

    num = fwrite(&m_format_hdr, 1, sizeof(m_format_hdr), m_file);
    if (num != sizeof(m_format_hdr))
        goto error;

    num = fwrite(&bitmap_hdr, 1, sizeof(BitmapInfoHeader), m_file);
    if (num != sizeof(BitmapInfoHeader))
        goto error;

    m_end_of_header = ftell(m_file);
    if (m_end_of_header < 0)
        goto error;

    if (!addJUNKChunk("", 2840))
        goto error;

    m_avi_hdr.list1.cb = m_end_of_header - sizeof(m_avi_hdr.riff) -
                        sizeof(m_avi_hdr.avi) - sizeof(m_avi_hdr.list1);
    m_movi_chunk.fcc = FOURCC('L', 'I', 'S', 'T');
    m_movi_chunk.cb = 0; // update when finished

    num = fwrite(&m_movi_chunk, 1, sizeof(m_movi_chunk), m_file);
    if (num != sizeof(m_movi_chunk))
        goto error;

    m_movi_start = ftell(m_file);
    if (m_movi_start < 0)
        goto error;

    num = fwrite(&fcc_movi, 1, sizeof(fcc_movi), m_file);
    if (num != sizeof(fcc_movi))
        goto error;

    return true;

error:
    closeFile(true/*delete_file*/);
    return false;
}   // createFile

// ----------------------------------------------------------------------------
int AVIWriter::bmpToJpg(unsigned char* image_data, unsigned char* image_output,
                        unsigned long buf_length)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);

    jpeg_create_compress(&cinfo);

    cinfo.image_width = m_width;
    cinfo.image_height = m_height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, m_img_quality, true);

    jpeg_mem_dest(&cinfo, &image_output, &buf_length);

    jpeg_start_compress(&cinfo, true);

    JSAMPROW jrow[1];
    while (cinfo.next_scanline < cinfo.image_height)
    {
        jrow[0] = &image_data[cinfo.next_scanline * m_width * 3];
        jpeg_write_scanlines(&cinfo, jrow, 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    return buf_length;
}   // bmpToJpg

#endif
