//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 Lucas Baudin
//                2011 Lucas Baudin, Joerg Henrichs
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

#include "addons/network_http.hpp"

#include <curl/curl.h>
#include <errno.h>
#include <stdio.h>
#include <string>

#if defined(WIN32) && !defined(__CYGWIN__)
#  include <windows.h>
#  define isnan _isnan
#else
#  include <sys/time.h>
#  include <math.h>
#endif

#include "addons/news_manager.hpp"
#include "addons/request.hpp"
#include "config/user_config.hpp"
#include "io/file_manager.hpp"
#include "states_screens/addons_screen.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "utils/string_utils.hpp"
#include "utils/time.hpp"

#if defined(WIN32) && !defined(__CYGWIN__)
// Use Sleep, which takes time in msecs. It must be defined after the
// includes, since otherwise irrlicht's sleep function is changed.
#  define sleep(s) Sleep(1000*(s))
#else
#  include <unistd.h>
#endif

NetworkHttp *network_http=NULL;

// ----------------------------------------------------------------------------
/** Create a thread that handles all network functions independent of the 
 *  main program. NetworkHttp supports only a single thread (i.e. it's not
 *  possible to download two addons at the same time), which makes handling
 *  and synchronisation a lot easier (otherwise all objects using this object
 *  would need an additional handle to get the right data back).
 *  This separate thread is running in NetworkHttp::mainLoop, and is being 
 *  waken up if a command is issued (e.g. using downloadFileAsynchronous).
 *  While UserConfigParams are modified, they can't (easily) be saved here,
 *  since the user might trigger another save in the menu (potentially
 *  ending up with an corrupted file).
 */
NetworkHttp::NetworkHttp() : m_abort(false),
                             m_all_requests(std::vector<Request*>())
{
    // Don't even start the network threads if networking is disabled.
    if(UserConfigParams::m_internet_status!=NetworkHttp::IPERM_ALLOWED )
        return;

    curl_global_init(CURL_GLOBAL_ALL);
    m_curl_session = curl_easy_init();
    // Abort if curl error occurred.
    if(!m_curl_session)
        return;

    pthread_cond_init(&m_cond_request, NULL);

    pthread_attr_t  attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    m_thread_id = new pthread_t();

    Request *request = new Request(Request::HC_INIT, 9999);
    m_all_requests.lock();
    m_all_requests.getData().push_back(request);
    m_all_requests.unlock();

    int error = pthread_create(m_thread_id, &attr, 
                               &NetworkHttp::mainLoop, this);
    if(error)
    {
        delete m_thread_id;
        m_thread_id = 0;
        printf("[addons] Warning: could not create thread, error=%d.\n", errno);
    }
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

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,      NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    me->m_all_requests.lock();
    while(me->m_all_requests.getData().size()           == 0               ||
          me->m_all_requests.getData()[0]->getCommand() != Request::HC_QUIT   )
    {
        bool empty = me->m_all_requests.getData().size()==0;
        // Wait in cond_wait for a request to arrive. The 'while' is necessary
        // since "spurious wakeups from the pthread_cond_wait ... may occur"
        // (pthread_cond_wait man page)!
        while(empty)
        {
            if(UserConfigParams::logAddons())
                printf("[addons] No request, sleeping.\n");

            pthread_cond_wait(&me->m_cond_request, 
                              me->m_all_requests.getMutex());
            empty = me->m_all_requests.getData().size()==0;
        }
        // Get the first (=highest priority) request and remove it from the 
        // queue. Only this code actually removes requests from the queue,
        // so it is certain that even 
        Request *request = me->m_all_requests.getData()[0];
        me->m_all_requests.getData().erase(me->m_all_requests.getData().begin());
        me->m_all_requests.unlock();
        if(UserConfigParams::logAddons())
        {
            if(request->getCommand()==Request::HC_DOWNLOAD_FILE)
                printf("[addons] Executing download '%s' to '%s' priority %d.\n",
                       request->getURL().c_str(), request->getSavePath().c_str(),
                       request->getPriority());
            else
                printf("[addons] Executing command '%d' priority %d.\n",
                       request->getCommand(), request->getPriority()); 
        }
        if(request->getCommand()==Request::HC_QUIT)
            break;
        switch(request->getCommand())
        {
        case Request::HC_QUIT:                break;
        case Request::HC_INIT: me->init();    break;            
        case Request::HC_NEWS: assert(false); break;
        case Request::HC_DOWNLOAD_FILE:
             me->downloadFileInternal(request);
             break;
        }   // switch(request->getCommand())

        if(request->manageMemory())
            delete request;
        // We have to lock here so that we can access m_all_requests
        // in the while condition at the top of the loop
        me->m_all_requests.lock();
    }   // while !quit
    if(UserConfigParams::logAddons())
        printf("[addons] Network thread terminating.\n");

    me->m_thread_id = 0;
    return NULL;
}   // mainLoop

// ---------------------------------------------------------------------------
/** Aborts the thread running here, and returns then.
 */
NetworkHttp::~NetworkHttp()
{
    if(UserConfigParams::m_internet_status!=NetworkHttp::IPERM_ALLOWED)
        return;

    // If a download should be active (which means it was cancelled by the
    // user, in which case it will still be ongoing in the background)
    // we can't get the mutex, and would have to wait for a timeout,
    // and we couldn't finish STK. This way we request an abort of
    // a download, which mean we can get the mutex and ask the service
    // thread here to cancel properly.
    cancelDownload();
    if(UserConfigParams::logAddons())
        printf("[addons] Trying to lock command mutex.\n");

    m_all_requests.lock();
    {
        Request *r = new Request(Request::HC_QUIT, 999);
        // Insert the Quit request as the first element of the queue.
        m_all_requests.getData().insert(m_all_requests.getData().begin(), r);
        pthread_cond_signal(&m_cond_request);
    }
    m_all_requests.unlock();

    if(UserConfigParams::logAddons())
        printf("[addons] Command mutex unlocked.\n");

    if(m_thread_id)
    {
        printf("[addons] Cancelling network thread.\n");
        int e = pthread_cancel(*m_thread_id);
        printf("[addons] Cancelled network thread. Return code: %d\n", e);
        delete m_thread_id;
    }

    pthread_cond_destroy(&m_cond_request);

    curl_easy_cleanup(m_curl_session);
    m_curl_session = NULL;
    curl_global_cleanup();
}   // ~NetworkHttp

// ---------------------------------------------------------------------------
/** Initialises the online part of the network manager. It downloads the
 *  news.xml file from the server (if the frequency of downloads makes this
 *  necessary), and (again if necessary) the addons.xml file.
 *  \return 0 if an error happened and no online connection will be available,
 *          1 otherwise.
 */
int NetworkHttp::init()
{
    // The news message must be updated if either it has never been updated,
    // or if the time of the last update was more than news_frequency ago.
    bool download = UserConfigParams::m_news_last_updated==0  ||
                    UserConfigParams::m_news_last_updated
                        +UserConfigParams::m_news_frequency
                    < Time::getTimeSinceEpoch();

    if(!download)
    {
        // If there is no old news message file, force a new download
        std::string xml_file = file_manager->getAddonsFile("news.xml");
        if(!file_manager->fileExists(xml_file))
            download=true;
    }

    // Initialise the online portion of the addons manager.
    if(download && UserConfigParams::logAddons())
        printf("[addons] Downloading list.\n");

    Request r(Request::HC_DOWNLOAD_FILE, 9999, false,
              "news.xml", "news.xml");
    if(!download || downloadFileInternal(&r))
    {
        std::string xml_file = file_manager->getAddonsFile("news.xml");
        if(download)
            UserConfigParams::m_news_last_updated = Time::getTimeSinceEpoch();
        const XMLNode *xml = new XMLNode(xml_file);
        news_manager->init();
#ifdef ADDONS_MANAGER
        loadAddonsList(xml, xml_file);
#endif
    }
    else
    {
#ifdef ADDONS_MANAGER
        addons_manager->setErrorState();
        if(UserConfigParams::logAddons())
            printf("[addons] Can't download addons list.\n");
        return 0;
#endif
    }
    return 1;
}   // init

// ----------------------------------------------------------------------------
/** Checks the last modified date and if necessary updates the
 *  list of addons.
 *  \param xml The news xml file which contains the data about
 *         the addon list.
 *  \param filename The filename of the news xml file. Only needed
 *         in case of an error (e.g. it might contain a corrupted
 *         url) - the file will be deleted so that on next start
 *         of stk it will be updated again.
 */
void NetworkHttp::loadAddonsList(const XMLNode *xml,
                                 const std::string &filename)
{
    std::string    addon_list_url("");
    Time::TimeType mtime(0);
    const XMLNode *include = xml->getNode("include");
    if(include)
    {
        include->get("file",  &addon_list_url);
        include->get("mtime", &mtime         );
    }
    if(addon_list_url.size()==0)
    {
        file_manager->removeFile(filename);
        news_manager->addNewsMessage(_("Can't access stkaddons server..."));
        return;
    }

    bool download = mtime > UserConfigParams::m_addons_last_updated;
    if(!download)
    {
        std::string filename=file_manager->getAddonsFile("addons.xml");
        if(!file_manager->fileExists(filename))
            download = true;
    }

    Request r(Request::HC_DOWNLOAD_FILE, 9999, false,
              addon_list_url, "addons.xml");
    if(!download || downloadFileInternal(&r))
    {
        std::string xml_file = file_manager->getAddonsFile("addons.xml");
        if(download)
            UserConfigParams::m_addons_last_updated=Time::getTimeSinceEpoch();
        const XMLNode *xml = new XMLNode(xml_file);
#ifdef ADDONS_MANAGER
        addons_manager->initOnline(xml);
        if(UserConfigParams::logAddons())
            printf("[addons] Addons manager list downloaded\n");
#endif

    }
}   // loadAddonsList

// ----------------------------------------------------------------------------
/** Download a file. The file name isn't absolute, the server in the config 
 *  will be added to file. The file is downloaded with a ".part" extention,
 *  and the file is renamed after it was downloaded successfully.
 *  \param request The request object containing the url and the path where
 *         the file is saved to.
 */
bool NetworkHttp::downloadFileInternal(Request *request)
{
    std::string full_save = 
        file_manager->getAddonsFile(request->getSavePath());

    if(UserConfigParams::logAddons())
        printf("[addons] Downloading '%s' as '%s').\n", 
               request->getURL().c_str(), request->getSavePath().c_str());
    std::string full_url = request->getURL();
    if(full_url.substr(0, 5)!="http:" && full_url.substr(0, 4)!="ftp:")
        full_url = (std::string)UserConfigParams::m_server_addons 
                 + "/" + full_url;

    curl_easy_setopt(m_curl_session, CURLOPT_URL, full_url.c_str());
    std::string uagent = (std::string)"SuperTuxKart/" + STK_VERSION;
    curl_easy_setopt(m_curl_session, CURLOPT_USERAGENT, uagent.c_str());
    curl_easy_setopt(m_curl_session, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(m_curl_session, CURLOPT_PROGRESSDATA, request);
    FILE * fout = fopen((full_save+".part").c_str(), "wb");
        
    //from and out
    curl_easy_setopt(m_curl_session,  CURLOPT_WRITEDATA,     fout  );
    curl_easy_setopt(m_curl_session,  CURLOPT_WRITEFUNCTION, fwrite);
    
    curl_easy_setopt(m_curl_session,  CURLOPT_PROGRESSFUNCTION, 
                     &NetworkHttp::progressDownload);
    curl_easy_setopt(m_curl_session,  CURLOPT_NOPROGRESS, 0);
                
    int status = curl_easy_perform(m_curl_session);
    fclose(fout);
    if(status==CURLE_OK)
    {
        if(UserConfigParams::logAddons())
            printf("[addons] Download successful.\n");

        int ret = rename((full_save+".part").c_str(), full_save.c_str());
        // In case of an error, set the status to indicate this
        if(ret!=0)
        {
            if(UserConfigParams::logAddons())
               printf("[addons] Could not rename downloaded file!\n");
            status=CURLE_WRITE_ERROR;
        }
        else
            request->notifyAddon();
    }
    else
    {
        printf("[addons] Problems downloading file - return code %d.\n",
               status);
    }
    
    request->setProgress( (status==CURLE_OK) ? 1.0f : -1.0f );
    return status==CURLE_OK;
}   // downloadFileInternal

// ----------------------------------------------------------------------------
/** Signals to the progress function to request any ongoing download to be 
 *  cancelled. This function can also be called if there is actually no 
 *  download atm. The function progressDownload checks m_abort and will 
 *  return a non-zero value which causes libcurl to abort. */
void NetworkHttp::cancelDownload() 
{ 
    if(UserConfigParams::logAddons())
        printf("[addons] Requesting cancellation of download.\n");
    m_abort.set(true); 
}   // cancelDownload

// ----------------------------------------------------------------------------
/** External interface to download a file asynchronously. This will wake up 
 *  the thread and schedule it to download the file. The calling program has 
 *  to poll using getProgress() to find out if the download has finished.
 *  \param url The file from the server to download.
 *  \param save The name to save the downloaded file under. Defaults to
 *              the name given in file.
 *  \param priority Priority of the request (must be <=99)
 */
Request *NetworkHttp::downloadFileAsynchron(const std::string &url,
                                            const std::string &save,
                                            int                priority,
                                            bool               manage_memory)
{
    // Limit priorities to 99 so that important system requests
    // (init and quit) will have highest priority.
    assert(priority<=99);
    Request *request = new Request(Request::HC_DOWNLOAD_FILE, priority, 
                                   manage_memory,
                                   url, (save!="") ? save : url          );

    if(UserConfigParams::logAddons())
        printf("[addons] Download asynchron '%s' as '%s'.\n", 
               request->getURL().c_str(), request->getSavePath().c_str());
    insertRequest(request);
    return request;
}   // downloadFileAsynchron

// ----------------------------------------------------------------------------
/** Inserts a request into the queue of all requests. The request will be
 *  sorted by priority.
 *  \param request The pointer to the new request to insert.
 */
void NetworkHttp::insertRequest(Request *request)
{
    // Wake up the network http thread
    m_all_requests.lock();
    {
        m_all_requests.getData().push_back(request);
        unsigned int i=m_all_requests.getData().size()-1;
        while(i>0 && *(m_all_requests.getData()[i])<*request)
        {
                m_all_requests.getData()[i] = m_all_requests.getData()[i-1];
                i--;
        }
        m_all_requests.getData()[i] = request;
        pthread_cond_signal(&m_cond_request);
    }
    m_all_requests.unlock();
}   // insertRequest

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
    Request *request = (Request *)clientp;
    // Check if we are asked to abort the download. If so, signal this
    // back to libcurl by returning a non-zero status.
    if(network_http->m_abort.get())
    {
        if(UserConfigParams::logAddons())
            printf("[addons] Aborting download in progress.\n");
        return 1;
    }
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
    request->setProgress(f);
    return 0;
}   // progressDownload


