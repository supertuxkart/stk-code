//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014 Joerg Henrichs
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

#include "config/hardware_stats.hpp"

#include "config/user_config.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/irr_driver.hpp"
#include "online/http_request.hpp"
#include "utils/string_utils.hpp"

namespace HardwareStats
{
void reportHardwareStats()
{
    Json json;
#ifdef WIN32
    json.add("os_win", 1);
#endif
#ifdef __APPLE__
    json.add("os_macosx", 1);
#endif
#ifdef __linux__
    json.add("os_linx", 1);
    json.add("os_unix", 1);
#endif
#ifdef DEBUG
    json.add("build_debug", 1);
#endif

    unsigned int ogl_version = irr_driver->getGLSLVersion();
    unsigned int major = ogl_version/100;
    unsigned int minor = ogl_version - 100*major;
    std::string version = 
        StringUtils::insertValues("%d.%d", major, minor);
    json.add("GL_VERSION", version);

    std::string vendor;
    irr_driver->getOpenGLData(&vendor);
    json.add("GL_VENDOR", vendor);

    json.add("video_xres", UserConfigParams::m_width );
    json.add("video_yres", UserConfigParams::m_height);
    // Too long for debugging atm
    //json.add("GL_EXTENSIONS", getGLExtensions());
    json.finish();
    Log::verbose("json", "'%s'", json.toString().c_str());

    Online::HTTPRequest *request = new Online::HTTPRequest(/*manage memory*/true, 1);
    request->addParameter("user_id", 3);
    request->addParameter("generation_date", StkTime::getTimeSinceEpoch());
    request->addParameter("data_type", "hwdetect");
    request->addParameter("data_version", 1);
    request->addParameter("data", json.toString());
    request->setURL("http://stats.supertuxkart.net/upload/v1");
    // FIXME: For now: don't submit
    //request->queue();
}   // reportHardwareStats

}   // namespace HardwareStats
