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
#include "utils/string_utils.hpp"
#include <string>

#include <GlyphLayout.h>
#include <IGUIStaticText.h>

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#ifdef _MSC_VER
#pragma comment (lib, "Shell32.lib")
#endif
#endif

#ifndef SERVER_ONLY
#include "SDL_version.h"

#if SDL_VERSION_ATLEAST(2, 0, 14)
#include "SDL_misc.h"
#endif

#endif

#ifdef IOS_STK
#include "../../../lib/irrlicht/source/Irrlicht/CIrrDeviceiOS.h"
#endif

using namespace Online;

namespace Online
{
    bool LinkHelper::openURLIrrElement(irr::gui::IGUIStaticText* text,
                                       irr::SEvent::SMouseInput mouse)
    {
        if (mouse.Event != EMIE_LMOUSE_PRESSED_DOWN)
            return false;
        std::shared_ptr<std::u32string> s;
        int glyph_idx = -1;
        int cluster = text->getCluster(mouse.X, mouse.Y, &s, &glyph_idx);
        if (cluster == -1 || (unsigned)cluster > s->size())
            return false;
        const std::vector<gui::GlyphLayout>& gls = text->getGlyphLayouts();
        std::u32string url = gui::extractURLFromGlyphLayouts(gls, glyph_idx);
        if (url.empty())
            return false;
        openURL(StringUtils::utf32ToUtf8(url));
        return true;
    }

    bool LinkHelper::isSupported()
    {
#ifdef SERVER_ONLY
        return false;
#else

#if defined(_WIN32) || defined(__APPLE__) || (!defined(__ANDROID__) && (defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__HAIKU__)))
        return true;
#elif SDL_VERSION_ATLEAST(2, 0, 14)
        return true;
#else
        return false;
#endif

#endif
    }

    void LinkHelper::openURL(const std::string& url)
    {
#ifndef SERVER_ONLY

#if defined(_WIN32)
        ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
#elif defined(IOS_STK)
        irr::CIrrDeviceiOS::openURLiOS(url.c_str());
#elif defined(__APPLE__) || defined(__HAIKU__)
        std::string command = std::string("open ").append(url);
        if (system(command.c_str()))
        {
            Log::error("OpenURL", "Command returned non-zero exit status");
        }
#elif !defined(__ANDROID__) && (defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__))
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
#elif SDL_VERSION_ATLEAST(2, 0, 14)
        SDL_OpenURL(url.c_str());
#else
        Log::error("OpenURL", "Not implemented for this platform!");
#endif

#endif
    }
}
