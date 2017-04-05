
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

#ifndef HEADER_RECORDER_COMMON_HPP
#define HEADER_RECORDER_COMMON_HPP

#include "utils/no_copy.hpp"

#include <string>
#include <list>
#include <pthread.h>

namespace Recorder
{
    // ------------------------------------------------------------------------
    enum VideoFormat { VF_VP8, VF_VP9, VF_MJPEG, VF_H264 };
    // ------------------------------------------------------------------------
    struct ThreadData : public NoCopy
    {
        void* m_data;
        pthread_cond_t* m_request;
    };
    // ------------------------------------------------------------------------
    void setRecordingName(const std::string& name);
    // ------------------------------------------------------------------------
    const std::string& getRecordingName();
    // ------------------------------------------------------------------------
    void prepareCapture();
    // ------------------------------------------------------------------------
    void captureFrameBufferImage();
    // ------------------------------------------------------------------------
    void stopRecording();
    // ------------------------------------------------------------------------
    bool isRecording();
    // ------------------------------------------------------------------------
    void destroyRecorder();
    // ------------------------------------------------------------------------
    bool displayProgress();

};
#endif

#endif
