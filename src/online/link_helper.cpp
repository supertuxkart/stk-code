//  SuperTuxKart - A fun racing game with go-karts
//  Copyright (C) 2017-18 QwertyChouskie
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

#include "online/link_helper.hpp"
#include "graphics/irr_driver.hpp"
#include "utils/log.hpp"
#include <string>
#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

#ifdef ANDROID
#include "../../../lib/irrlicht/source/Irrlicht/CIrrDeviceAndroid.h"
#endif

#ifdef IOS_STK
#include "../../../lib/irrlicht/source/Irrlicht/CIrrDeviceiOS.h"
#endif

using namespace Online;

namespace Online
{
    bool LinkHelper::isSupported()
    {
#if defined(_WIN32) || defined(__APPLE__) || (defined(__linux__))
        return true;
#else
        return false;
#endif
    }

    void LinkHelper::openURL (std::string url)
    {
#if defined(ANDROID)
        CIrrDeviceAndroid* android = dynamic_cast<CIrrDeviceAndroid*>(irr_driver->getDevice());
        if (android)
            android->openURL(url);
#elif defined(_WIN32)
        ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
#elif defined(IOS_STK)
        irr::CIrrDeviceiOS::openURLiOS(url.c_str());
#elif defined(__APPLE__)
        std::string command = std::string("open ").append(url);
        if (system(command.c_str()))
        {
            Log::error("OpenURL", "Command returned non-zero exit status");
        }
#elif defined(__linux__)
        std::string command = std::string("xdg-open ").append(url);
        
        const char* lib_path = getenv("LD_LIBRARY_PATH");
        const char* system_lib_path = getenv("SYSTEM_LD_LIBRARY_PATH");

        if (system_lib_path != NULL)
        {
            setenv("LD_LIBRARY_PATH", system_lib_path, 1);
        }

        if (system(command.c_str()))
        {
            Log::error("OpenURL", "Command returned non-zero exit status");
        }

        if (system_lib_path != NULL)
        {
            if (lib_path != NULL)
            {
                setenv("LD_LIBRARY_PATH", lib_path, 1);
            }
            else
            {
                unsetenv("LD_LIBRARY_PATH");
            }
        }
#else
        Log::error("OpenURL", "Not implemented for this platform!");
#endif
    }
}
