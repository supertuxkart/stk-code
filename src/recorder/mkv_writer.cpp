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

#include "recorder_private.hpp"

#include <cstring>
#include <list>
#include <mkvmuxer/mkvmuxer.h>
#include <mkvmuxer/mkvwriter.h>
#include <mkvparser/mkvparser.h>
#include <sys/stat.h>
#include <vpx/vpx_encoder.h>

namespace Recorder
{
    // ------------------------------------------------------------------------
    std::string writeMKV(const std::string& video, const std::string& audio)
    {
        std::string no_ext = video.substr(0, video.find_last_of("."));
        VideoFormat vf = getConfig()->m_video_format;
        std::string file_name = no_ext +
            (vf == OGR_VF_VP8 || vf == OGR_VF_VP9 ? ".webm" : ".mkv");
        mkvmuxer::MkvWriter writer;
        if (!writer.Open(file_name.c_str()))
        {
            printf("Error while opening output file.\n");
            return "";
        }
        mkvmuxer::Segment muxer_segment;
        if (!muxer_segment.Init(&writer))
        {
            printf("Could not initialize muxer segment.\n");;
            return "";
        }
        std::list<mkvmuxer::Frame*> audio_frames;
        uint8_t* buf = (uint8_t*)malloc(1024 * 1024);
        FILE* input = NULL;
        struct stat st;
        int result = stat(audio.c_str(), &st);
        if (result == 0)
        {
            input = fopen(audio.c_str(), "rb");
            uint32_t sample_rate, channels;
            fread(&sample_rate, 1, sizeof(uint32_t), input);
            fread(&channels, 1, sizeof(uint32_t), input);
            uint64_t aud_track = muxer_segment.AddAudioTrack(sample_rate, channels,
                0);
            if (!aud_track)
            {
                printf("Could not add audio track.\n");
                return "";
            }
            mkvmuxer::AudioTrack* const at = static_cast<mkvmuxer::AudioTrack*>
                (muxer_segment.GetTrackByNumber(aud_track));
            if (!at)
            {
                printf("Could not get audio track.\n");
                return "";
            }
            uint32_t codec_private_size;
            fread(&codec_private_size, 1, sizeof(uint32_t), input);
            fread(buf, 1, codec_private_size, input);
            if (!at->SetCodecPrivate(buf, codec_private_size))
            {
                printf("Could not add audio private data.\n");
                return "";
            }
            while (fread(buf, 1, 12, input) == 12)
            {
                uint32_t frame_size;
                int64_t timestamp;
                memcpy(&frame_size, buf, sizeof(uint32_t));
                memcpy(&timestamp, buf + sizeof(uint32_t), sizeof(int64_t));
                fread(buf, 1, frame_size, input);
                mkvmuxer::Frame* audio_frame = new mkvmuxer::Frame();
                if (!audio_frame->Init(buf, frame_size))
                {
                    printf("Failed to construct a frame.\n");
                    return "";
                }
                audio_frame->set_track_number(aud_track);
                audio_frame->set_timestamp(timestamp);
                audio_frame->set_is_key(true);
                audio_frames.push_back(audio_frame);
            }
            fclose(input);
            if (remove(audio.c_str()) != 0)
            {
                printf("Failed to remove audio data file\n");
            }
        }
        uint64_t vid_track = muxer_segment.AddVideoTrack(getConfig()->m_width,
            getConfig()->m_height, 0);
        if (!vid_track)
        {
            printf("Could not add video track.\n");
            return "";
        }
        mkvmuxer::VideoTrack* const vt = static_cast<mkvmuxer::VideoTrack*>(
            muxer_segment.GetTrackByNumber(vid_track));
        if (!vt)
        {
            printf("Could not get video track.\n");
            return "";
        }
        vt->set_frame_rate(getConfig()->m_record_fps);
        switch (vf)
        {
        case OGR_VF_VP8:
            vt->set_codec_id("V_VP8");
            break;
        case OGR_VF_VP9:
            vt->set_codec_id("V_VP9");
            break;
        case OGR_VF_MJPEG:
            vt->set_codec_id("V_MJPEG");
            break;
        case OGR_VF_H264:
            vt->set_codec_id("V_MPEG4/ISO/AVC");
            break;
        default:
            break;
        }
        input = fopen(video.c_str(), "rb");
        while (fread(buf, 1, 16, input) == 16)
        {
            uint32_t frame_size, flag;
            int64_t timestamp;
            memcpy(&frame_size, buf, sizeof(uint32_t));
            memcpy(&timestamp, buf + sizeof(uint32_t), sizeof(int64_t));
            memcpy(&flag, buf + sizeof(uint32_t) + sizeof(int64_t),
                sizeof(uint32_t));
            timestamp *= 1000000000ll / getConfig()->m_record_fps;
            fread(buf, 1, frame_size, input);
            mkvmuxer::Frame muxer_frame;
            if (!muxer_frame.Init(buf, frame_size))
            {
                printf("Failed to construct a frame.\n");
                return "";
            }
            muxer_frame.set_track_number(vid_track);
            muxer_frame.set_timestamp(timestamp);
            if (vf == OGR_VF_VP8 || vf == OGR_VF_VP9)
            {
                muxer_frame.set_is_key((flag & VPX_FRAME_IS_KEY) != 0);
            }
            else
            {
                muxer_frame.set_is_key(true);
            }
            mkvmuxer::Frame* cur_aud_frame =
                audio_frames.empty() ? NULL : audio_frames.front();
            if (cur_aud_frame != NULL)
            {
                while (cur_aud_frame->timestamp() < (uint64_t)timestamp)
                {
                    if (!muxer_segment.AddGenericFrame(cur_aud_frame))
                    {
                        printf("Could not add audio frame.\n");
                        return "";
                    }
                    delete cur_aud_frame;
                    audio_frames.pop_front();
                    if (audio_frames.empty())
                    {
                        cur_aud_frame = NULL;
                        break;
                    }
                    cur_aud_frame = audio_frames.front();
                }
            }
            if (!muxer_segment.AddGenericFrame(&muxer_frame))
            {
                printf("Could not add video frame.\n");
                return "";
            }
        }
        free(buf);
        fclose(input);
        for (mkvmuxer::Frame* aud_frame : audio_frames)
        {
            delete aud_frame;
        }
        if (remove(video.c_str()) != 0)
        {
            printf("Failed to remove video data file\n");
        }
        if (!muxer_segment.Finalize())
        {
            printf("Finalization of segment failed.\n");
            return "";
        }
        writer.Close();
        return file_name;
    }   // writeMKV
};

#endif
