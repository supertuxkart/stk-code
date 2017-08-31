//  SuperTuxKart - A fun racing game with go-karts
//  Copyright (C) 2017 QwertyChouskie
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

#include "online/update_manager.hpp"

#include "config/user_config.hpp"
#include "online/http_request.hpp"
#include "online/request_manager.hpp"
#include "utils/constants.hpp"

//#include <cstdio>
#include <string>
#include <iostream>

using namespace Online;

namespace Online
{
    struct Version
    {
        Version(std::string versionStr)
        {
            major = 0;
            minor = 0;
            revision = 0;
            build = 0;
            sscanf(versionStr.c_str(), "%d.%d.%d.%d", &major, &minor, &revision, &build);
        }

        bool operator<(const Version &otherVersion)
        {
            /*std::cout << major << "\n"; // For debugging
            std::cout << minor << "\n";
            std::cout << revision << "\n";
            std::cout << build << "\n";

            std::cout << otherVersion.major << "\n";
            std::cout << otherVersion.minor << "\n";
            std::cout << otherVersion.revision << "\n";
            std::cout << otherVersion.build << "\n";*/

            if(major < otherVersion.major)
                return true;
            if(major > otherVersion.major)
                return false;
            if(minor < otherVersion.minor)
                return true;
            if(minor > otherVersion.minor)
                return false;
            if(revision < otherVersion.revision)
                return true;
            if(revision > otherVersion.revision)
                return false;
            if(build < otherVersion.build)
                return true;
            if(build > otherVersion.build)
                return false;
            return false;
        }

        int major, minor, revision, build;
    };

    bool UpdateManager::UpdateAvailable() {
        if (UserConfigParams::m_internet_status != Online::RequestManager::IPERM_ALLOWED || strcmp(STK_VERSION, "git") == 0)
        {
            return false;
        }

        Log::info("Updater", "Checking for updates.");
        Online::HTTPRequest *download_request = new Online::HTTPRequest();
        download_request->setURL("https://addons.supertuxkart.net/CurrVer.php");
        download_request->executeNow();
        if(download_request->hadDownloadError())
        {
            Log::error("Updater", "Error downloading CurrVer.php: %s.",
                       download_request->getDownloadErrorMessage());
            delete download_request;
            return false;
        }

        if (Version(STK_VERSION) < Version(download_request->getData())) {
            delete download_request;
            Log::info("Updater", "New version available!");
            return true;
        } else {
            delete download_request;
            Log::info("Updater", "No new version.");
            return false;
        }
    }
}
