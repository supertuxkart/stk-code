//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 Lucas Baudin
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

#ifdef ADDONS_MANAGER

#ifndef HEADER_NETWORK_HPP
#define HEADER_NETWORK_HPP

#include <pthread.h>
#include <string>
class NetworkHttp
{
    private:
        
    public:
        NetworkHttp();
        static void * checkNewServer(void * obj);
        static size_t writeStr(char str [], size_t size, size_t nb_char, std::string * stream);
        std::string downloadToStr(std::string url);
};

extern NetworkHttp * network_http;
/* * Download a file. The file name isn't absolute, the server in the config will be added to file.
   * progress_data is used to have the state of the download (in %)*/
bool download(std::string file, std::string save = "", int * progress_data = 0);


int progressDownload (void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);

static pthread_mutex_t download_mutex;
#endif
#endif
