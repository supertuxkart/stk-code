//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 SuperTuxKart-Team
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

#include "http_connector.hpp"

#include <iostream>
#include <stdio.h>
#include <memory.h>
#include "io/file_manager.hpp"
#include "utils/string_utils.hpp"


HTTPConnector::HTTPConnector(const std::string &url){
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if(!curl)
        printf("Error while loading cURL library.\n");
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    if (url.substr(0, 5)!="http:")
    {
        printf("Invalid URL");
    }
}

// ============================================================================


HTTPConnector::~HTTPConnector(){
    curl_easy_cleanup(curl);
    curl_global_cleanup();
}

// ============================================================================

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

XMLNode * HTTPConnector::getXMLFromPage(Parameters & post_parameters)
{
    return file_manager->createXMLTreeFromString(getPage(post_parameters));
}

std::string HTTPConnector::getPage(Parameters & post_parameters)
{
    Parameters::iterator iter;
    std::string postString;
    for (iter = post_parameters.begin(); iter != post_parameters.end(); ++iter)
    {
       if(iter != post_parameters.begin())
           postString.append("&");
       char * escaped = curl_easy_escape(this->curl , iter->first.c_str(), iter->first.size());
       postString.append(escaped);
       curl_free(escaped);
       postString.append("=");
       escaped = curl_easy_escape(this->curl , iter->second.c_str(), iter->second.size());
       postString.append(escaped);
       curl_free(escaped);
    }
    printf("Sending: %s\n", postString.c_str());
    curl_easy_setopt(this->curl, CURLOPT_POSTFIELDS, postString.c_str());
    std::string readBuffer;
    curl_easy_setopt(this->curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(this->curl, CURLOPT_FILE, &readBuffer);
    res = curl_easy_perform(this->curl);

    if(res != CURLE_OK)
    {
        Log::error("online/http_functions", "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }else{
        printf("Retrieved: %s\n", readBuffer.c_str());
    }
    return readBuffer;
}


