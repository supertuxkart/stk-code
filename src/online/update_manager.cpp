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
#include "online/request_manager.hpp"
#include "utils/constants.hpp"
#include "utils/log.hpp"
#include "curl/curl.h"
#include <cstdio>
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

    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
    {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    bool UpdateManager::UpdateAvailable() {
        if (UserConfigParams::m_internet_status != Online::RequestManager::IPERM_ALLOWED || strcmp(STK_VERSION, "git") == 0)
        {
            return false;
        }

        CURL *curl;
        CURLcode result;
        std::string readBuffer;

        curl = curl_easy_init();
        if (curl) { // Make sure curl initialized properly
            curl_easy_setopt(curl, CURLOPT_URL, "http://addons.supertuxkart.net/CurrVer.php");
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            result = curl_easy_perform(curl);
            curl_easy_cleanup(curl);

            if (Version(STK_VERSION) < Version(readBuffer) && result == 0) {
                return true;
            } else {
                return false;
            }
        }
        return false; // Return false if curl failed
    }
}
