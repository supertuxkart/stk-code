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

#ifndef HEADER_NETWORK_HTTP_HPP
#define HEADER_NETWORK_HTTP_HPP

#include <pthread.h>
#include <string>

#include "utils/synchronised.hpp"

class NetworkHttp
{
public:
    /** List of 'http commands' for this object:
     *  HC_SLEEP: No command, sleep
     *  HC_INIT: Object is being initialised
     *  HC_DOWNLOAD_FILE : download a file
     *  HC_QUIT:  Stop loop and terminate thread.
     *  HC_NEWS:  Update the news
     */
    enum HttpCommands {HC_SLEEP,
                       HC_QUIT,
                       HC_INIT,
                       HC_DOWNLOAD_FILE,
                       HC_NEWS    } ;
private:

    Synchronised<std::string> m_news;

    /** Which command to execute next. Access to this variable is guarded
     *  by m_mutex_command and m_cond_command. */
    HttpCommands    m_command;
    /** A mutex for accessing m_commands. */
    pthread_mutex_t m_mutex_command;
    /** A conditional variable to wake up the main loop. */
    pthread_cond_t  m_cond_command;

    /** The file to download when a file download is triggered. */
    std::string     m_file;

    /** The name and path under which to save the downloaded file. */
    std::string     m_save_filename;

    /** Progress of a download in percent. It is guaranteed that
     *  this value only becomes 1.0f, if the download is completed.*/
    Synchronised<float>  m_progress;

    /** Thread id of the thread running in this object. */
    pthread_t     m_thread_id;

    static void  *mainLoop(void *obj);
    void          checkNewServer();

    void          updateNews();
    std::string   downloadToStrInternal(std::string url);
    bool          downloadFileInternal(const std::string &file,
                                       const std::string &save_filename,
                                       bool is_asynchron);
    static int    progressDownload(void *clientp, double dltotal, double dlnow,
                                   double ultotal, double ulnow);
public:
                  NetworkHttp();
                 ~NetworkHttp();
    static size_t writeStr(char str [], size_t size, size_t nb_char, 
                           std::string * stream);
    void          downloadFileAsynchron(const std::string &file, 
                                        const std::string &save = "");
    bool          downloadFileSynchron(const std::string &file, 
                                       const std::string &save = "");

    const std::string 
                  getNewsMessage() const;
    float         getProgress() const;
};

extern NetworkHttp *network_http;

#endif
#endif
