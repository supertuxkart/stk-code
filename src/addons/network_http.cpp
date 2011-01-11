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
#include "utils/string_utils.hpp"

#if defined(WIN32) && !defined(__CYGWIN__)
// Use Sleep, which takes time in msecs. It must be defined after the
// includes, since otherwise irrlicht's sleep function is changed.
#  define sleep(s) Sleep(1000*(s))
#else
#  include <unistd.h>
#endif

NetworkHttp *network_http;

// ----------------------------------------------------------------------------
/** Create a thread that handles all network functions independent of the 
 *  main program. NetworkHttp supports only a single thread (i.e. it's not
 *  possible to download two addons at the same time), which makes handling
 *  and synchronisation a lot easier (otherwise all objects using this object
 *  would need an additional handle to get the right data back).
 *  This separate thread is running in NetworkHttp::mainLoop, and is being 
 *  waken up if a command is issued (e.g. using downloadFileAsynchronous).
 */
NetworkHttp::NetworkHttp() : m_news(""), m_progress(-1.0f)
{
    pthread_mutex_init(&m_mutex_command, NULL);
    pthread_cond_init(&m_cond_command, NULL);

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

    // Initialise the online portion of the addons manager.
    if(UserConfigParams::m_verbosity>=3)
        printf("[addons] Downloading list.\n");
    if(me->downloadFileSynchron("list"))
    {
        std::string xml_file = file_manager->getAddonsFile("list");

        const XMLNode *xml = new XMLNode(xml_file);
        me->checkNewServer(xml);
        me->updateNews(xml);
        addons_manager->initOnline(xml);
        if(UserConfigParams::m_verbosity>=3)
            printf("[addons] Addons manager list downloaded\n");
    }
    else
    {
        addons_manager->setErrorState();
        if(UserConfigParams::m_verbosity>=3)
            printf("[addons] Can't download addons list.\n");
    }

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,      NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    // Wait in the main loop till a command is received
    // (atm only QUIT is used).
    pthread_mutex_lock(&me->m_mutex_command);
    me->m_command = HC_SLEEP;
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
            assert(false);
            break;
        case HC_DOWNLOAD_FILE:
            me->downloadFileInternal(me->m_file, me->m_save_filename,
                                     /*is_asynchron*/true    );
        }   // switch(m_command)
        me->m_command = HC_SLEEP;
    }   // while 1
    pthread_mutex_unlock(&me->m_mutex_command);
    return NULL;
}   // mainLoop

// ---------------------------------------------------------------------------
/** Aborts the thread running here, and returns then.
 */
NetworkHttp::~NetworkHttp()
{
    pthread_mutex_lock(&m_mutex_command);
    {
        m_command=HC_QUIT;
        pthread_cond_signal(&m_cond_command);
    }
    pthread_mutex_unlock(&m_mutex_command);

    void *result;
    pthread_join(m_thread_id, &result);

    pthread_mutex_destroy(&m_mutex_command);
    pthread_cond_destroy(&m_cond_command);
}   // ~NetworkHttp

// ---------------------------------------------------------------------------
/** Checks if a redirect is received, causing a new server to be used for
 *  downloading addons.
 */
void NetworkHttp::checkNewServer(const XMLNode *xml)
{
    std::string new_server;
    int result = xml->get("redirect", &new_server);
    if(result==1 && new_server!="")
    {
        if(UserConfigParams::m_verbosity>=3)
        {
            std::cout << "[Addons] Current server: " 
                      << (std::string)UserConfigParams::m_server_addons 
                      << std::endl
                      << "[Addons] New server: " << new_server << std::endl;
        }
        UserConfigParams::m_server_addons = new_server;
        user_config->saveConfig();
    }
}   // checkNewServer

// ----------------------------------------------------------------------------
/** Updates the 'news' string to be displayed in the main menu.
 */
void NetworkHttp::updateNews(const XMLNode *xml)
{
    for(unsigned int i=0; i<xml->getNumNodes(); i++)
    {
        const XMLNode *node = xml->getNode(i);
        if(node->getName()!="news") continue;
        std::string news;
        node->get("text", &news);
        m_news.set(news);
    }
}   // updateNews

// ----------------------------------------------------------------------------
/** Returns the last loaded news message (using mutex to make sure a valid
 *  value is available).
 */
const std::string NetworkHttp::getNewsMessage() const
{
    return m_news.get();
}   // getNewsMessage

// ----------------------------------------------------------------------------

size_t NetworkHttp::writeStr(char ptr [], size_t size, size_t nb_char, 
                             std::string * stream)
{
    *stream = std::string(ptr);
    
    //needed, otherwise, the download failed
    return nb_char;
}   // writeStr

// ----------------------------------------------------------------------------

std::string NetworkHttp::downloadToStrInternal(std::string url)
{
	CURL *session = curl_easy_init();

    std::string full_url = (std::string)UserConfigParams::m_server_addons 
                         + "/" + url;
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
}   // downloadToStrInternal

// ----------------------------------------------------------------------------
/** Download a file. The file name isn't absolute, the server in the config 
 *  will be added to file. 
 *  \param progress_data is used to have the state of the download (in %)
 */
bool NetworkHttp::downloadFileInternal(const std::string &file,
                                       const std::string &save_filename,
                                       bool is_asynchron)
{
printf("Downloading %s\n", file.c_str());
	CURL *session = curl_easy_init();
    std::string full_url = (std::string)UserConfigParams::m_server_addons 
                         + "/" + file;
	curl_easy_setopt(session, CURLOPT_URL, full_url.c_str());
    FILE * fout = fopen(file_manager->getAddonsFile(save_filename).c_str(),
                         "wb");
	
	//from and out
	curl_easy_setopt(session,  CURLOPT_WRITEDATA,     fout  );
	curl_easy_setopt(session,  CURLOPT_WRITEFUNCTION, fwrite);
	    
    if(is_asynchron)
    {
        curl_easy_setopt(session,  CURLOPT_PROGRESSFUNCTION, 
            &NetworkHttp::progressDownload);
        // Necessary, oyherwise the progress function doesn't work
        curl_easy_setopt(session,  CURLOPT_NOPROGRESS, 0);
    }
		
	int success = curl_easy_perform(session);
	
	//close the file where we downloaded the content
	fclose(fout);
	
	//stop curl
	curl_easy_cleanup(session);

    if(is_asynchron)
        m_progress.set( (success==CURLE_OK) ? 1.0f : -1.0f );
    return success==CURLE_OK;
}   // downloadFileInternal

// ----------------------------------------------------------------------------
/** External interface to download a file synchronously, i.e. it will only 
 *  return once the download is complete. 
 *  \param file The file from the server to download.
 *  \param save The name to save the downloaded file under. Defaults to
 *              the name given in file.
 */
bool NetworkHttp::downloadFileSynchron(const std::string &file, 
                                       const std::string &save)
{
    const std::string &save_filename = (save!="") ? save : file;

    return downloadFileInternal(file, save_filename,
                                /*is_asynchron*/false);
}   // downloadFileSynchron

// ----------------------------------------------------------------------------
/** External interface to download a file asynchronously. This will wake up 
 *  the thread and schedule it to download the file. The calling program has 
 *  to poll using getProgress() to find out if the download has finished.
 *  \param file The file from the server to download.
 *  \param save The name to save the downloaded file under. Defaults to
 *              the name given in file.
 */
void NetworkHttp::downloadFileAsynchron(const std::string &file, 
                                        const std::string &save)
{
    m_progress.set(0.0f);
    m_file          = file;
    m_save_filename = (save!="") ? save : file;

    // Wake up the network http thread
    pthread_mutex_lock(&m_mutex_command);
    {
        m_command = HC_DOWNLOAD_FILE;
        pthread_cond_signal(&m_cond_command);
    }
    pthread_mutex_unlock(&m_mutex_command);
}   // downloadFileAsynchron

// ----------------------------------------------------------------------------
/** Callback function from curl: inform about progress.
 *  \param clientp 
 *  \param download_total Total size of data to download.
 *  \param download_now   How much has been downloaded so far.
 *  \param upload_total   Total amount of upload.
 *  \param upload_now     How muc has been uploaded so far.
 */
int NetworkHttp::progressDownload(void *clientp, 
                                  double download_total, double download_now, 
                                  double upload_total,   double upload_now)
{
    float f;
    if(download_now < download_total)
    {
        f = (float)download_now / (float)download_total;
        // In case of floating point rouding errors make sure that 
        // 1.0 is only reached when downloadFileInternal is finished
        if (f>=1.0f) f=0.99f;
    }
    else 
    {
        // Don't set progress to 1.0f; this is done in loadFileInternal
        // after checking curls return code!
        f= download_total==0 ? 0 : 0.99f;
    }
    network_http->m_progress.set(f);
    return 0;
}   // progressDownload

// ----------------------------------------------------------------------------
/** Returns the progress of a download that has been started.
 *  \return <0 in case of an error, between 0 and smaller than 1 while the
 *  download is in progress, and 1.0f if the download has finished.
 */
float NetworkHttp::getProgress() const
{
    return m_progress.get();
}   // getProgress

#endif
