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

#include "recorder/webm_writer.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"

#include <list>
#include <mkvmuxer/mkvmuxer.h>
#include <mkvmuxer/mkvwriter.h>
#include <mkvparser/mkvparser.h>
#include <sys/stat.h>
#include <vpx/vpx_encoder.h>

namespace Recorder
{
    // ------------------------------------------------------------------------
    void writeWebm(const std::string& video, const std::string& audio)
    {
        time_t rawtime;
        time(&rawtime);
        tm* timeInfo = localtime(&rawtime);
        char time_buffer[256];
        sprintf(time_buffer, "%i.%02i.%02i_%02i.%02i.%02i",
            timeInfo->tm_year + 1900, timeInfo->tm_mon + 1,
            timeInfo->tm_mday, timeInfo->tm_hour,
            timeInfo->tm_min, timeInfo->tm_sec);
        std::string no_ext = StringUtils::removeExtension(video);
        std::string webm_name = no_ext + "-" + time_buffer + ".webm";
        mkvmuxer::MkvWriter writer;
        if (!writer.Open(webm_name.c_str()))
        {
            Log::error("writeWebm", "Error while opening output file.");
            return;
        }
        mkvmuxer::Segment muxer_segment;
        if (!muxer_segment.Init(&writer))
        {
            Log::error("writeWebm", "Could not initialize muxer segment.");
            return;
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
            uint64_t aud_track = muxer_segment.AddAudioTrack(sample_rate,
                channels, 0);
            if (!aud_track)
            {
                Log::error("writeWebm", "Could not add audio track.");
                return;
            }
            mkvmuxer::AudioTrack* const at = static_cast<mkvmuxer::AudioTrack*>
                (muxer_segment.GetTrackByNumber(aud_track));
            if (!at)
            {
                Log::error("writeWebm", "Could not get audio track.");
                return;
            }
            uint32_t codec_private_size;
            fread(&codec_private_size, 1, sizeof(uint32_t), input);
            fread(buf, 1, codec_private_size, input);
            if (!at->SetCodecPrivate(buf, codec_private_size))
            {
                Log::warn("writeWebm", "Could not add audio private data.");
                return;
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
                    Log::error("writeWebm", "Failed to construct a frame.");
                    return;
                }
                audio_frame->set_track_number(aud_track);
                audio_frame->set_timestamp(timestamp);
                audio_frame->set_is_key(true);
                audio_frames.push_back(audio_frame);
            }
            fclose(input);
            if (remove(audio.c_str()) != 0)
            {
                Log::warn("writeWebm", "Failed to remove audio data file");
            }
        }
        uint64_t vid_track = muxer_segment.AddVideoTrack(
            irr_driver->getActualScreenSize().Width,
            irr_driver->getActualScreenSize().Height, 0);
        if (!vid_track)
        {
            Log::error("writeWebm", "Could not add video track.");
            return;
        }
        mkvmuxer::VideoTrack* const vt = static_cast<mkvmuxer::VideoTrack*>(
            muxer_segment.GetTrackByNumber(vid_track));
        if (!vt)
        {
            Log::error("writeWebm", "Could not get video track.");
            return;
        }
        vt->set_frame_rate(UserConfigParams::m_record_fps);
        input = fopen(video.c_str(), "rb");
        while (fread(buf, 1, 16, input) == 16)
        {
            uint32_t frame_size, flag;
            int64_t timestamp;
            memcpy(&frame_size, buf, sizeof(uint32_t));
            memcpy(&timestamp, buf + sizeof(uint32_t), sizeof(int64_t));
            memcpy(&flag, buf + sizeof(uint32_t) + sizeof(int64_t),
                sizeof(uint32_t));
            timestamp *= 1000000000ll / UserConfigParams::m_record_fps;
            fread(buf, 1, frame_size, input);
            mkvmuxer::Frame muxer_frame;
            if (!muxer_frame.Init(buf, frame_size))
            {
                Log::error("writeWebm", "Failed to construct a frame.");
                return;
            }
            muxer_frame.set_track_number(vid_track);
            muxer_frame.set_timestamp(timestamp);
            muxer_frame.set_is_key((flag & VPX_FRAME_IS_KEY) != 0);
            mkvmuxer::Frame* cur_aud_frame =
                audio_frames.empty() ? NULL : audio_frames.front();
            if (cur_aud_frame != NULL)
            {
                while (cur_aud_frame->timestamp() < (uint64_t)timestamp)
                {
                    if (!muxer_segment.AddGenericFrame(cur_aud_frame))
                    {
                        Log::error("writeWebm", "Could not add audio frame.");
                        return;
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
                Log::error("writeWebm", "Could not add video frame.");
                return;
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
            Log::warn("writeWebm", "Failed to remove video data file");
        }
        if (!muxer_segment.Finalize())
        {
            Log::error("writeWebm", "Finalization of segment failed.");
            return;
        }
        writer.Close();
    }   // writeWebm
};
#endif
