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
#include "addons/network_http.hpp"

#include <curl/curl.h>
#include <stdio.h>
#include <string>

#if defined(WIN32) && !defined(__CYGWIN__)
#  define isnan _isnan
#else
#  include <math.h>
#endif


#include "config/user_config.hpp"
#include "io/file_manager.hpp"
#include "states_screens/addons_screen.hpp"
#include "states_screens/main_menu_screen.hpp"

#if defined(WIN32) && !defined(__CYGWIN__)
// Use Sleep, which takes time in msecs. It must be defined after the
// includes, since otherwise irrlicht's sleep function is changed.
#  define sleep(s) Sleep(1000*(s))
#else
#  include <unistd.h>
#endif


pthread_mutex_t download_mutex;
NetworkHttp * network_http = 0;

// ----------------------------------------------------------------------------
/** Create a thread that handles all network functions independent of the 
 *  main program.
 */
NetworkHttp::NetworkHttp()
{
    pthread_mutex_init(&m_mutex_news, NULL);
    m_news_message = "";

    pthread_mutex_init(&m_mutex_command, NULL);
    pthread_cond_init(&m_cond_command, NULL);
    m_command      = HC_SLEEP;
    m_abort        = false;

    pthread_attr_t  attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_create(&m_thread_id, &attr, &NetworkHttp::mainLoop, this);
    pthread_attr_destroy(&attr);
}   // NetworkHttp

// ---------------------------------------------------------------------------
/** The actual main loop, which is started as a separate thread from the
 *  constructor. After testing for a new server, fetching news, the list 
 *  of packages to download, it will wait for commands to be issued.
 *  \param obj: A pointer to this object, passed on by pthread_create
 */
void *NetworkHttp::mainLoop(void *obj)
{
    NetworkHttp *me=(NetworkHttp*)obj;
    me->checkNewServer();
    me->updateNews();
    // Allow this thread to be cancelled anytime
    // FIXME: this mechanism will later not be necessary anymore!
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,      NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    // Wait in the main loop till a command is received
    // (atm only QUIT is used).
    pthread_mutex_lock(&me->m_mutex_command);
    while(1)
    {
        pthread_cond_wait(&me->m_cond_command, &me->m_mutex_command);
        switch(me->m_command)
        {
        case HC_QUIT: 
            pthread_exit(NULL);
            break;
        case HC_SLEEP: 
            break;
        case HC_NEWS:
            me->updateNews();
            break;
        }   // switch(m_command)
    }   // while !m_abort
    return NULL;
}   // mainLoop

// ---------------------------------------------------------------------------
/** Aborts the thread running here, and returns then.
 */
NetworkHttp::~NetworkHttp()
{
    m_abort=1;  // Doesn't really need a mutex

    pthread_mutex_lock(&m_mutex_command);
    {
        m_command=HC_QUIT;
        pthread_cond_signal(&m_cond_command);
    }
    pthread_mutex_unlock(&m_mutex_command);

    void *result;
    pthread_join(m_thread_id, &result);

    pthread_mutex_destroy(&m_mutex_news);
    pthread_mutex_destroy(&m_mutex_command);
    pthread_cond_destroy(&m_cond_command);
}   // ~NetworkHttp

// ---------------------------------------------------------------------------
/** Checks if a redirect is received, causing a new server to be used for
 *  downloading addons.
 */
void NetworkHttp::checkNewServer()
{
    std::string newserver = downloadToStr("redirect");
    if (newserver != "")
    {
        newserver.replace(newserver.find("\n"), 1, "");

        if(UserConfigParams::m_verbosity>=4)
        {
            std::cout << "[Addons] Current server: " 
                      << UserConfigParams::m_server_addons.toString() 
                      << std::endl
                      << "[Addons] New server: " << newserver << std::endl;
        }
        UserConfigParams::m_server_addons = newserver;
        user_config->saveConfig();
    }
    else if(UserConfigParams::m_verbosity>=4)
    {
        std::cout << "[Addons] No new server." << std::endl;
    }
}   // checkNewServer

// ----------------------------------------------------------------------------
/** Updates the 'news' string to be displayed in the main menu.
 */
void NetworkHttp::updateNews()
{
    const std::string tmp_str = downloadToStr("news");

    pthread_mutex_lock(&m_mutex_news);
    {
        // Only lock the actual assignment, not the downloading!
        m_news_message = tmp_str;
    }
    pthread_mutex_unlock(&m_mutex_news);
}   // updateNews

// ----------------------------------------------------------------------------
/** Returns the last loaded news message (using mutex to make sure a valid
 *  value is available).
 */
const std::string NetworkHttp::getNewsMessage() const
{
    pthread_mutex_lock(&m_mutex_news);
    const std::string tmp_str = m_news_message;
    pthread_mutex_unlock(&m_mutex_news);
    return tmp_str;
}   // getNewsMessage

// ----------------------------------------------------------------------------

size_t NetworkHttp::writeStr(char ptr [], size_t size, size_t nb_char, std::string * stream)
{
    static std::string str = std::string(ptr);
    *stream = str;
    
    //needed, otherwise, the download failed
    return nb_char;
}

// ------------------------------------------------------------------------------------------------------

std::string NetworkHttp::downloadToStr(std::string url)
{
	CURL *session = curl_easy_init();

    std::string full_url=UserConfigParams::m_server_addons.toString()+"/"+url;
	curl_easy_setopt(session, CURLOPT_URL, full_url.c_str());
	
	std::string fout;
	
	//from and out
	curl_easy_setopt(session, CURLOPT_WRITEDATA,     &fout                 );
	curl_easy_setopt(session, CURLOPT_WRITEFUNCTION, &NetworkHttp::writeStr);
	
	int success = curl_easy_perform(session);
	
	//stop curl
	curl_easy_cleanup(session);
	
	if (success == 0) return fout;
	else             return "";
}

// ----------------------------------------------------------------------------
/** Download a file. The file name isn't absolute, the server in the config 
 *  will be added to file. 
 *  \param progress_data is used to have the state of the download (in %)
 */
bool download(std::string file, const std::string &save, int * progress_data)
{
	CURL *session = curl_easy_init();
    std::string full_url=UserConfigParams::m_server_addons.toString()+"/"+file;
	curl_easy_setopt(session, CURLOPT_URL, full_url.c_str());
	FILE * fout;
	if(save != "")
	    fout = fopen((file_manager->getAddonsDir() + "/" + save).c_str(), "w");
    else
        fout = fopen((file_manager->getAddonsDir() + "/" + file).c_str(), "w");
	
	
	//from and out
	curl_easy_setopt(session,  CURLOPT_WRITEDATA, fout);
	curl_easy_setopt(session,  CURLOPT_WRITEFUNCTION, fwrite);
	
	//init the mutex for the progress function
    pthread_mutex_init(&download_mutex, NULL);
    
	curl_easy_setopt(session,  CURLOPT_PROGRESSFUNCTION, &progressDownload);
	//needed, else, the progress function doesn't work
	curl_easy_setopt(session,  CURLOPT_NOPROGRESS, 0);
	
	//to update the progress bar
	curl_easy_setopt(session,  CURLOPT_PROGRESSDATA, progress_data);
	
	int success = curl_easy_perform(session);
	
	//close the file where we downloaded the content
	fclose(fout);
	
	//stop curl
	curl_easy_cleanup(session);
	
    return (success == 0);
}

// ------------------------------------------------------------------------------------------------------

//FIXME : this way is a bit ugly but the simplest at the moment
int time_last_print = 0;
int progressDownload (void *clientp, float dltotal, float dlnow, 
                      float ultotal, float ulnow)
{
    int progress = (int)(dlnow/dltotal*100);
    if(isnan(dlnow/dltotal*100))
        progress = 0;
    pthread_mutex_lock(&download_mutex);
    if(clientp != NULL)
    {
        int * progress_data = (int*)clientp;
        *progress_data = progress;
    }
    pthread_mutex_unlock(&download_mutex);
    if(time_last_print > 10)
    {
        std::cout << "Download progress: " << progress << "%" << std::endl;
        time_last_print = 0;
    }
    else
    {
        time_last_print += 1;
    }
    return 0;
}
#endif
