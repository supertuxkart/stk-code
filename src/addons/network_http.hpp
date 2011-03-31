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

#ifndef HEADER_NETWORK_HTTP_HPP
#define HEADER_NETWORK_HTTP_HPP

#include <pthread.h>
#include <string>
#include <vector>

#include "irrlicht.h"
using namespace irr;

#include "utils/synchronised.hpp"


class XMLNode;

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
                       HC_NEWS    };

    /** If stk has permission to access the internet (for news
     *  server etc).
     *  IPERM_NOT_ASKED: The user needs to be asked if he wants to 
     *                   grant permission
     *  IPERM_ALLOWED:   STK is allowed to access server.
     *  IPERM_NOT_ALLOWED: STK must not access external servers. */
    enum InternetPermission {IPERM_NOT_ASKED  =0,
                             IPERM_ALLOWED    =1,
                             IPERM_NOT_ALLOWED=2 };
private:
    // A wrapper class to store news message together with
    // a message id and a display count.
    class NewsMessage
    {
        // The actual news message
        core::stringw m_news;
        // A message id used to store some information in the
        // user config file.
        int           m_message_id;
        // Counts how often a message has been displayed.
        int           m_display_count;
    public:
        NewsMessage(const core::stringw &m, int id)
        {
            m_news          = m;
            m_message_id    = id;
            m_display_count = 0;
        }   // NewsMessage
        /** Returns the news message. */
        const core::stringw& getNews() const {return m_news;}
        /** Increases how often this message was being displayed. */
        void increaseDisplayCount() {m_display_count++;}
        /** Returns the news id. */
        int  getMessageId() const {return m_message_id;}
        /** Returns the display count. */
        int getDisplayCount() const {return m_display_count; }
        /** Sets the display count for this message. */
        void setDisplayCount(int n) {m_display_count = n; }
    };   // NewsMessage

    mutable Synchronised< std::vector<NewsMessage> > m_news;

    /** Index of the current news message that is being displayed. */
    int             m_current_news_message;

    /** Stores the news message display count from the user config file. 
    */
    std::vector<int> m_saved_display_count;

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

    /** Signal an abort in case that a download is still happening. */
    Synchronised<bool>   m_abort;

    /** Thread id of the thread running in this object. */
    pthread_t     *m_thread_id;

    static void  *mainLoop(void *obj);
    void          checkRedirect(const XMLNode *xml);

    void          updateNews(const XMLNode *xml,
                             const std::string &filename);
    void          updateUserConfigFile() const;
    void          loadAddonsList(const XMLNode *xml,
                                 const std::string &filename);
    std::string   downloadToStrInternal(std::string url);
    void          updateMessageDisplayCount();
    bool          downloadFileInternal(const std::string &file,
                                       const std::string &save_filename,
                                       bool is_asynchron);
    static int    progressDownload(void *clientp, double dltotal, double dlnow,
                                   double ultotal, double ulnow);
    bool          conditionFulfilled(const std::string &cond);
    int           versionToInt(const std::string &s);
public:
                  NetworkHttp();
                 ~NetworkHttp();
    static size_t writeStr(char str [], size_t size, size_t nb_char, 
                           std::string * stream);
    void          downloadFileAsynchron(const std::string &file, 
                                        const std::string &save = "");
    bool          downloadFileSynchron(const std::string &file, 
                                       const std::string &save = "");

    const core::stringw
                  getNextNewsMessage();
    float         getProgress() const;
    void          cancelDownload();
};   // NetworkHttp

extern NetworkHttp *network_http;

#endif

