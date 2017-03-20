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

#ifndef SERVER_ONLY

#include "utils/avi_writer.hpp"
#include "io/file_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "modes/world.hpp"
#include "race/race_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/vs.hpp"

#include <cstring>

// ----------------------------------------------------------------------------
AVIWriter::AVIWriter()
{
    m_file = NULL;
    m_pbo_use = 0;
    m_accumulated_time = 0.0f;
    m_remaining_time = 0.0f;
    m_img_quality = 0;
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
    pthread_mutex_init(&m_fbi_mutex, NULL);
    pthread_cond_init(&m_cond_request, NULL);
    pthread_create(&m_thread, NULL, &startRoutine, this);
}   // AVIWriter

// ----------------------------------------------------------------------------
void* AVIWriter::startRoutine(void *obj)
{
    VS::setThreadName("AVIWriter");
    AVIWriter* avi_writer = (AVIWriter*)obj;
    while (true)
    {
        pthread_mutex_lock(&avi_writer->m_fbi_mutex);
        bool waiting = avi_writer->m_fbi_queue.empty();
        while (waiting)
        {
            pthread_cond_wait(&avi_writer->m_cond_request,
                &avi_writer->m_fbi_mutex);
            waiting = avi_writer->m_fbi_queue.empty();
        }
        auto& p =  avi_writer->m_fbi_queue.front();
        uint8_t* fbi = p.first;
        int frame_count = p.second;
        if (frame_count == -1)
        {
            avi_writer->closeFile();
            avi_writer->m_file = NULL;
            avi_writer->m_fbi_queue.pop_front();
            pthread_mutex_unlock(&avi_writer->m_fbi_mutex);
            continue;
        }
        else if (fbi == NULL)
        {
            avi_writer->setCanBeDeleted();
            avi_writer->m_fbi_queue.pop_front();
            pthread_mutex_unlock(&avi_writer->m_fbi_mutex);
            return NULL;
        }
        avi_writer->m_fbi_queue.pop_front();
        pthread_mutex_unlock(&avi_writer->m_fbi_mutex);
        video::IImage* image = irr_driver->getVideoDriver()
            ->createImageFromData(video::ECF_A8R8G8B8,
            irr_driver->getActualScreenSize(), fbi,
            true/*ownForeignMemory*/);
        video::IImage* rgb = irr_driver->getVideoDriver()->createImage
            (video::ECF_R8G8B8, irr_driver->getActualScreenSize());
        image->copyTo(rgb);
        image->drop();
        rgb->setDeleteMemory(false);
        fbi = (uint8_t*)rgb->lock();
        rgb->unlock();
        rgb->drop();
        uint8_t* orig_fbi = fbi;
        const unsigned width = avi_writer->m_width;
        const unsigned height = avi_writer->m_height;
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
        const unsigned size = width * height * 3;
        uint8_t* jpg = new uint8_t[size];
        int new_len = avi_writer->bmpToJpg(orig_fbi, jpg, size);
        delete [] orig_fbi;
        while (frame_count != 0)
        {
            avi_writer->addImage(jpg, new_len);
            frame_count--;
        }
        delete [] jpg;
    }
    return NULL;
}   // startRoutine

// ----------------------------------------------------------------------------
int AVIWriter::handleFrameBufferImage(float dt)
{
    const float frame_rate = 0.001f * m_msec_per_frame;
    m_accumulated_time += dt;
    if (m_accumulated_time < frame_rate && m_remaining_time < frame_rate)
    {
        return 0;
    }
    int frame_count = 1;
    m_remaining_time += m_accumulated_time - frame_rate;
    m_accumulated_time = 0.0f;
    while (m_remaining_time > frame_rate)
    {
        frame_count++;
        m_remaining_time -= frame_rate;
    }
    return frame_count;
}   // handleFrameBufferImage

// ----------------------------------------------------------------------------
void AVIWriter::captureFrameBufferImage(float dt)
{
    if (m_file == NULL)
    {
        createFile(AVI_FORMAT_JPG, 30/*fps*/, 24/*bits*/, 90/*quality*/);
    }
    glReadBuffer(GL_BACK);

    int pbo_read = -1;
    if (m_pbo_use > 3 && m_pbo_use % 3 == 0)
        m_pbo_use = 3;
    if (m_pbo_use >= 3)
    {
        int frame_count = handleFrameBufferImage(dt);
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
void AVIWriter::stopRecording()
{
    assert(m_file != NULL);
    addFrameBufferImage(NULL, -1);
}   // stopRecording

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
    closeFile(true);
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

    if (m_total_frames >= MAX_FRAMES)
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
    closeFile(true);
    return AVI_IO_ERR;

size_limit:
    closeFile();
    return AVI_SIZE_LIMIT_ERR;
}   // addImage

// ----------------------------------------------------------------------------
bool AVIWriter::closeFile(bool delete_file)
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
        m_avi_hdr.avih.dwMicroSecPerFrame = m_msec_per_frame * 1000;
        m_avi_hdr.avih.dwMaxBytesPerSec = (uint32_t)
            (((m_stream_bytes / m_total_frames) * m_format_hdr.strh.dwRate) /
            m_msec_per_frame + 0.5f);
        m_avi_hdr.avih.dwTotalFrames = m_total_frames;

        num = fwrite(&m_avi_hdr, 1, sizeof(m_avi_hdr), m_file);
        if (num != sizeof(m_avi_hdr))
            goto error;

        m_format_hdr.strh.dwScale = m_msec_per_frame;
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

    return true;

error:
    fclose(m_file);
    remove(m_filename.c_str());
    m_file = NULL;
    return false;
}   // closeFile

// ----------------------------------------------------------------------------
bool AVIWriter::createFile(AVIFormat avi_format, int fps, int bits,
                           int quality)
{
    if (m_file != NULL)
        return false;

    m_img_quality = quality;
    time_t rawtime;
    time(&rawtime);
    tm* timeInfo = localtime(&rawtime);
    char time_buffer[256];
    sprintf(time_buffer, "%i.%02i.%02i_%02i.%02i.%02i",
        timeInfo->tm_year + 1900, timeInfo->tm_mon + 1,
        timeInfo->tm_mday, timeInfo->tm_hour,
        timeInfo->tm_min, timeInfo->tm_sec);

    std::string track_name = World::getWorld() != NULL ?
        race_manager->getTrackName() : "menu";

    m_filename = file_manager->getScreenshotDir() + track_name + "-"
        + time_buffer + ".avi";
    m_stream_bytes = 0;
    m_total_frames = 0;
    m_msec_per_frame = unsigned(1000 / fps);
    m_movi_start = 0;
    m_last_junk_chunk = 0;
    m_total_frames = 0;

    BitmapInfoHeader bitmap_hdr;
    bitmap_hdr.biSize = sizeof(BitmapInfoHeader);
    bitmap_hdr.biWidth = m_width;
    bitmap_hdr.biHeight = m_height;
    bitmap_hdr.biPlanes = 1;
    bitmap_hdr.biBitCount = bits;
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

    //Format specific changes
    if (avi_format == AVI_FORMAT_JPG)
    {
        m_format_hdr.strh.fccHandler = CC_MJPG;
        bitmap_hdr.biCompression = FOURCC('M', 'J', 'P', 'G');
        m_chunk_fcc = FOURCC('0', '0', 'd', 'c');
    }
    else if (avi_format == AVI_FORMAT_BMP)
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
    closeFile(true);
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
