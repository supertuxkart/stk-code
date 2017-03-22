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

#include "graphics/gl_headers.hpp"
#include "utils/can_be_deleted.hpp"
#include "utils/no_copy.hpp"
#include "utils/singleton.hpp"
#include "utils/synchronised.hpp"

#include <string>
#include <list>

#include <pthread.h>

#define FOURCC(a,b,c,d) ((uint32_t) (((d)<<24) | ((c)<<16) | ((b)<<8) | (a)))

const uint32_t CC_MJPG = FOURCC('m', 'j', 'p', 'g');
const uint32_t CC_DIB = FOURCC('\0', '\0', '\0', '\0');
const uint32_t CC_VIDS = FOURCC('v', 'i', 'd', 's');

const uint32_t AVIF_HASINDEX = 0x00000010;
const uint32_t AVIF_MUSTUSEINDEX = 0x00000020;
const uint32_t AVIF_ISINTERLEAVED = 0x00000100;
const uint32_t AVIF_TRUSTCKTYPE = 0x00000800;
const uint32_t AVIF_WASCAPTUREFILE = 0x00010000;
const uint32_t AVIF_COPYRIGHTED = 0x00020000;

const uint32_t AVISF_DISABLED = 0x00000001;
const uint32_t AVISF_VIDEO_PALCHANGES = 0x00010000;

const uint32_t AVIIF_LIST = 0x00000001;
const uint32_t AVIIF_KEYFRAME = 0x00000010;
const uint32_t AVIIF_FIRSTPART = 0x00000020;
const uint32_t AVIIF_LASTPART = 0x00000040;
const uint32_t AVIIF_MIDPART = 0x00000060;
const uint32_t AVIIF_NOTIME = 0x00000100;
const uint32_t AVIIF_COMPUSE = 0x0FFF0000;

enum AVIFormat
{
    AVI_FORMAT_BMP,
    AVI_FORMAT_JPG
};

enum AVIErrCode
{
    AVI_SUCCESS,
    AVI_SIZE_LIMIT_ERR,
    AVI_IO_ERR
};

const int MAX_FRAMES = 1000000;
const int MAX_FILE_SIZE = 2000000000;

struct MainAVIHeader
{
    uint32_t dwMicroSecPerFrame;
    uint32_t dwMaxBytesPerSec;
    uint32_t dwPaddingGranularity;
    uint32_t dwFlags;
    uint32_t dwTotalFrames;
    uint32_t dwInitialFrames;
    uint32_t dwStreams;
    uint32_t dwSuggestedBufferSize;
    uint32_t dwWidth;
    uint32_t dwHeight;
    uint32_t dwReserved[4];
};

struct AVIStreamHeader
{
    uint32_t fccType;
    uint32_t fccHandler;
    uint32_t dwFlags;
    uint16_t wPriority;
    uint16_t wLanguage;
    uint32_t dwInitialFrames;
    uint32_t dwScale;
    uint32_t dwRate;
    uint32_t dwStart;
    uint32_t dwLength;
    uint32_t dwSuggestedBufferSize;
    uint32_t dwQuality;
    uint32_t dwSampleSize;
    uint16_t Left;
    uint16_t Top;
    uint16_t Right;
    uint16_t Bottom;
};

struct BitmapInfoHeader
{
    uint32_t biSize;
    uint32_t biWidth;
    uint32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    uint32_t biXPelsPerMeter;
    uint32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
};

struct AVIINDEXENTRY
{
    uint32_t ckid;
    uint32_t dwFlags;
    uint32_t dwChunkOffset;
    uint32_t dwChunkLength;
};

struct CHUNK
{
    uint32_t fcc;
    uint32_t cb;
};

struct AVIHeader
{
    CHUNK riff;
    uint32_t avi;
    CHUNK list1;
    uint32_t hdrl;
    CHUNK avihhdr;
    MainAVIHeader avih;
};

struct FormatHeader
{
    CHUNK list;
    uint32_t strl;
    CHUNK strhhdr;
    AVIStreamHeader strh;
    CHUNK strfhdr;
};

struct IndexTable
{
    uint32_t Offset;
    uint32_t Length;
    uint32_t fcc;
};


class AVIWriter : public CanBeDeleted, public NoCopy,
                  public Singleton<AVIWriter>
{
private:
    FILE* m_file;

    Synchronised<std::string> m_recording_target;

    std::string m_filename;

    int m_last_junk_chunk, m_end_of_header, m_movi_start, m_img_quality,
        m_width, m_height;

    unsigned int m_msec_per_frame, m_stream_bytes, m_total_frames, m_pbo_use;

    float m_accumulated_time, m_remaining_time;

    AVIFormat m_avi_format;

    AVIHeader m_avi_hdr;

    CHUNK m_movi_chunk;

    FormatHeader m_format_hdr;

    IndexTable m_index_table[MAX_FRAMES];

    uint32_t m_chunk_fcc;

    Synchronised<std::list<std::pair<uint8_t*, int> > > m_fbi_queue;

    Synchronised<bool> m_idle;

    pthread_t m_thread;

    pthread_cond_t m_cond_request;

    GLuint m_pbo[3];

    // ------------------------------------------------------------------------
    int bmpToJpg(unsigned char* image_data, unsigned char* image_output,
                 unsigned long buf_length);
    // ------------------------------------------------------------------------
    AVIErrCode addImage(unsigned char* buffer, int size);
    // ------------------------------------------------------------------------
    bool closeFile(bool delete_file = false, bool exiting = false);
    // ------------------------------------------------------------------------
    bool createFile();
    // ------------------------------------------------------------------------
    bool addJUNKChunk(std::string str, unsigned int min_size);
    // ------------------------------------------------------------------------
    void addFrameBufferImage(uint8_t* fbi, int frame_count)
    {
        m_fbi_queue.lock();
        m_fbi_queue.getData().emplace_back(fbi, frame_count);
        pthread_cond_signal(&m_cond_request);
        m_fbi_queue.unlock();
    }
    // ------------------------------------------------------------------------
    int getFrameCount(float dt);
    // ------------------------------------------------------------------------
    void cleanAllFrameBufferImages()
    {
        m_fbi_queue.lock();
        for (auto& p : m_fbi_queue.getData())
            delete [] p.first;
        m_fbi_queue.getData().clear();
        m_fbi_queue.unlock();
    }
    // ------------------------------------------------------------------------
    bool isIdle() const                          { return m_idle.getAtomic(); }

public:
    // ------------------------------------------------------------------------
    AVIWriter();
    // ------------------------------------------------------------------------
    ~AVIWriter();
    // ------------------------------------------------------------------------
    static void* startRoutine(void *obj);
    // ------------------------------------------------------------------------
    void captureFrameBufferImage(float dt);
    // ------------------------------------------------------------------------
    void resetFrameBufferImage();
    // ------------------------------------------------------------------------
    void resetCaptureFormat();
    // ------------------------------------------------------------------------
    void stopRecording()                     { addFrameBufferImage(NULL, -1); }
    // ------------------------------------------------------------------------
    void setRecordingTarget(const std::string& name)
                                        { m_recording_target.setAtomic(name); }

};

#endif
