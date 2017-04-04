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

#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "recorder/recorder_common.hpp"
#include "utils/log.hpp"
#include "utils/synchronised.hpp"
#include "utils/vs.hpp"

#include <turbojpeg.h>

namespace Recorder
{
    // ------------------------------------------------------------------------
    void* jpgWriter(void *obj)
    {
        VS::setThreadName("jpgWriter");
        FILE* jpg_writer = fopen((getRecordingName() + ".video").c_str(), "wb");
        if (jpg_writer == NULL)
        {
            Log::error("jpgWriter", "Failed to open file for writing");
            return NULL;
        }
        ThreadData* td = (ThreadData*)obj;
        Synchronised<std::list<std::tuple<uint8_t*, unsigned, int> > >*
            jpg_data = (Synchronised<std::list<std::tuple<uint8_t*,
                unsigned, int> > >*)td->m_data;
        pthread_cond_t* cond_request = td->m_request;

        const unsigned width = irr_driver->getActualScreenSize().Width;
        const unsigned height = irr_driver->getActualScreenSize().Height;
        int64_t frames_encoded = 0;
        while (true)
        {
            jpg_data->lock();
            bool waiting = jpg_data->getData().empty();
            while (waiting)
            {
                pthread_cond_wait(cond_request, jpg_data->getMutex());
                waiting = jpg_data->getData().empty();
            }
            auto& p = jpg_data->getData().front();
            uint8_t* jpg = std::get<0>(p);
            uint32_t jpg_size = std::get<1>(p);
            int frame_count = std::get<2>(p);
            if (jpg == NULL)
            {
                jpg_data->getData().clear();
                jpg_data->unlock();
                break;
            }
            jpg_data->getData().pop_front();
            jpg_data->unlock();
            while (frame_count != 0)
            {
                fwrite(&jpg_size, 1, sizeof(uint32_t), jpg_writer);
                fwrite(&frames_encoded, 1, sizeof(int64_t), jpg_writer);
                fwrite(&jpg_size, 1, sizeof(uint32_t), jpg_writer);
                fwrite(jpg, 1, jpg_size, jpg_writer);
                frame_count--;
                frames_encoded++;
            }
            tjFree(jpg);
        }
        fclose(jpg_writer);
        return NULL;

    }   // jpgWriter
}
#endif
