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

#include "capture_library.hpp"
#include "recorder_private.hpp"

#include <turbojpeg.h>

namespace Recorder
{
    // ------------------------------------------------------------------------
    void mjpegWriter(CaptureLibrary* cl)
    {
        setThreadName("mjpegWriter");
        FILE* mjpeg_writer = fopen((getSavedName() + ".video").c_str(), "wb");
        if (mjpeg_writer == NULL)
        {
            printf("Failed to open file for writing mjpeg.\n");
            return;
        }
        int64_t frames_encoded = 0;
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
                break;
            }
            cl->getJPGList()->pop_front();
            ul.unlock();
            while (frame_count != 0)
            {
                fwrite(&jpg_size, 1, sizeof(uint32_t), mjpeg_writer);
                fwrite(&frames_encoded, 1, sizeof(int64_t), mjpeg_writer);
                fwrite(&jpg_size, 1, sizeof(uint32_t), mjpeg_writer);
                fwrite(jpg, 1, jpg_size, mjpeg_writer);
                frame_count--;
                frames_encoded++;
            }
            tjFree(jpg);
        }
        fclose(mjpeg_writer);
    }   // mjpegWriter
};
#endif
