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

NetworkHttp *network_http;

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
NetworkHttp::NetworkHttp() : m_news(std::vector<NewsMessage>()), 
                             m_progress(-1.0f), m_abort(false)
{
    m_current_news_message = -1;
    // Don't even start the network threads if networking is disabled.
    if(!UserConfigParams::m_enable_internet)
        return;

    pthread_mutex_init(&m_mutex_command, NULL);
    pthread_cond_init(&m_cond_command, NULL);

    // Since there are no threads at this stage, just init
    // m_command without mutex.
    m_command = HC_SLEEP;
    pthread_attr_t  attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    m_thread_id = new pthread_t();
    int error=pthread_create(m_thread_id, &attr, &NetworkHttp::mainLoop, this);
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
    if(download && UserConfigParams::m_verbosity>=3)
        printf("[addons] Downloading list.\n");

    if(!download || me->downloadFileSynchron("news.xml"))
    {
        std::string xml_file = file_manager->getAddonsFile("news.xml");
        if(download)
            UserConfigParams::m_news_last_updated = Time::getTimeSinceEpoch();
        const XMLNode *xml = new XMLNode(xml_file);
        me->checkRedirect(xml);
        me->updateNews(xml, xml_file);
        me->loadAddonsList(xml, xml_file);
#ifdef ADDONS_MANAGER
        addons_manager->initOnline(xml);
        if(UserConfigParams::m_verbosity>=3)
            printf("[addons] Addons manager list downloaded\n");
#endif
    }
    else
    {
#ifdef ADDONS_MANAGER
        addons_manager->setErrorState();
        if(UserConfigParams::m_verbosity>=3)
            printf("[addons] Can't download addons list.\n");
#endif
    }

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,      NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    // Wait in the main loop till a command is received
    pthread_mutex_lock(&me->m_mutex_command);

    // Handle the case that STK is cancelled before the while loop
    // is entered. If this would happen, this thread hangs in 
    // pthread_cond_wait (since cond_signal was done before the wait),
    // and stk hangs since the thread can't join.
    if(me->m_command==HC_QUIT)
    {
        return NULL;
    }

    while(1)
    {
        pthread_cond_wait(&me->m_cond_command, &me->m_mutex_command);
        switch(me->m_command)
        {
        case HC_QUIT: 
            pthread_exit(NULL);
            break;
        case HC_SLEEP: 
        case HC_INIT:
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
    if(!UserConfigParams::m_enable_internet)
        return;

    // if a download should be active (which means it was cancelled by the
    // user, in which case it will still be ongoing in the background)
    // we can't get the mutex, and would have to wait for a timeout,
    // and we couldn't finish STK. This way we request an abort of
    // a download, which mean we can get the mutex and ask the service
    // thread here to cancel properly.
    cancelDownload();
    if(UserConfigParams::m_verbosity>=3)
        printf("[addons] Trying to lock command mutex.\n");
    pthread_mutex_lock(&m_mutex_command);
    {
        m_command=HC_QUIT;
        pthread_cond_signal(&m_cond_command);
    }
    pthread_mutex_unlock(&m_mutex_command);
    if(UserConfigParams::m_verbosity>=3)
        printf("[addons] Mutex unlocked.\n");

    if(m_thread_id)
    {
        void *result;
        if(UserConfigParams::m_verbosity>=3)
            printf("[addons] Trying to join network thread.\n");
        pthread_join(*m_thread_id, &result);
        delete m_thread_id;
        if(UserConfigParams::m_verbosity>=3)
            printf("[addons] Network thread joined.\n");
    }

    pthread_mutex_destroy(&m_mutex_command);
    pthread_cond_destroy(&m_cond_command);
}   // ~NetworkHttp

// ---------------------------------------------------------------------------
/** Checks if a redirect is received, causing a new server to be used for
 *  downloading addons.
 *  \param xml XML data structure containing the redirect information.
 */
void NetworkHttp::checkRedirect(const XMLNode *xml)
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
    }
}   // checkRedirect

// ----------------------------------------------------------------------------
/** Updates the 'news' string to be displayed in the main menu.
 *  \param xml The XML data from the news file.
 *  \param filename The filename of the news xml file. Only needed
 *         in case of an error (e.g. the file might be corrupted) 
 *         - the file will be deleted so that on next start of stk it 
 *         will be updated again.
 */
void NetworkHttp::updateNews(const XMLNode *xml, const std::string &filename)
{
    bool error = true;
    int frequency=0;
    if(xml->get("frequency", &frequency))
        UserConfigParams::m_news_frequency = frequency;

    for(unsigned int i=0; i<xml->getNumNodes(); i++)
    {
        const XMLNode *node = xml->getNode(i);
        if(node->getName()!="message") continue;
        std::string news;
        node->get("content", &news);
        int id=-1;
        node->get("id", &id);

        std::string cond;
        node->get("condition", &cond);
        if(!conditionFulfilled(cond))
            continue;
        m_news.lock();
        {

            // Define this if news messages should be removed
            // after being shown a certain number of times.
#undef NEWS_MESSAGE_REMOVAL
#ifdef NEWS_MESSAGE_REMOVAL
            // Only add the news if it's not supposed to be ignored.
            if(id>UserConfigParams::m_ignore_message_id)
#endif
            {
                NewsMessage n(core::stringw(news.c_str()), id);
                m_news.getData().push_back(n);
            }
        }
        m_news.unlock();

        error = false;
    }
    if(error)
    {
        // In case of an error (e.g. the file only contains
        // an error message from the server), delete the file
        // so that it is not read again (and this will force
        // a new read on the next start, instead of waiting
        // for some time).
        file_manager->removeFile(filename);
        NewsMessage n(_("Can't access stkaddons server..."), -1);
        m_news.lock();
        m_news.getData().push_back(n);
        m_news.unlock();
    }
#ifdef NEWS_MESSAGE_REMOVAL
    else
        updateMessageDisplayCount();
#endif
    
}   // updateNews

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
        NewsMessage n("Can't access stkaddons server...", -1);
        m_news.lock();
        m_news.getData().push_back(n);
        m_news.unlock();
        return;
    }

    bool download = mtime > UserConfigParams::m_addons_last_updated;
    if(!download)
    {
        std::string filename=file_manager->getAddonsFile("addon_list.xml");
    }
}   // loadAddonsList

// ----------------------------------------------------------------------------
/** Returns the next loaded news message. It will 'wrap around', i.e.
 *  if there is only one message it will be returned over and over again.
 *  To be used by the the main menu to get the next news message after
 *  one message was scrolled off screen.
 */
const core::stringw NetworkHttp::getNextNewsMessage()
{
    if(m_news.getData().size()==0)
        return "";

    core::stringw m("");
    m_news.lock();
    {
        // Check if we have a message that was finished being
        // displayed --> increase display count.
        if(m_current_news_message>-1)
        {
#ifdef NEWS_MESSAGE_REMOVAL
            NewsMessage &n = m_news.getData()[m_current_news_message];
            n.increaseDisplayCount();
#endif

            // If the message is being displayed often enough,
            // ignore it from now on.
#ifdef NEWS_MESSAGE_REMOVAL
            if(n.getDisplayCount()>stk_config->m_max_display_news)
            {
                // Messages have sequential numbers, so we only store
                // the latest message id (which is the current one)
                UserConfigParams::m_ignore_message_id = n.getMessageId();
                m_news.getData().erase(m_news.getData().begin()
                                       +m_current_news_message  );

            }
#endif
            updateUserConfigFile();
            // 
            if(m_news.getData().size()==0)
            {
                m_news.unlock();
                return "";
            }
        }
        m_current_news_message++;
        if(m_current_news_message >= (int)m_news.getData().size())
            m_current_news_message = 0;            

        m = m_news.getData()[m_current_news_message].getNews();
    }
    m_news.unlock();
    return m;
}   // getNextNewsMessage

// ----------------------------------------------------------------------------
/** Saves the information about which message was being displayed how often
 *  to the user config file. It dnoes not actually save the user config
 *  file, this is left to the main program (user config is saved at
 *  the exit of the program).
 *  Note that this function assumes that m_news is already locked!
 */
void NetworkHttp::updateUserConfigFile() const
{
#ifdef NEWS_MESSAGE_REMOVAL
    std::ostringstream o;
    for(unsigned int i=0; i<m_news.getData().size(); i++)
    {
        const NewsMessage &n=m_news.getData()[i];
        o << n.getMessageId()    << ":"
          << n.getDisplayCount() << " ";
    }
    UserConfigParams::m_display_count = o.str();
#else
    // Always set them to be empty to avoid any
    // invalid data that might create a problem later.
    UserConfigParams::m_display_count     = "";
    UserConfigParams::m_ignore_message_id = -1;
#endif
}   // updateUserConfigFile

// ----------------------------------------------------------------------------
/** Checks if the given condition list are all fulfilled.
 *  The conditions must be seperated by ";", and each condition
 *  must be of the form "type comp version".
 *  Type must be 'stkversion'
 *  comp must be one of "<", "=", ">"
 *  version must be a valid STK version string
 *  \param cond The list of conditions
 *  \return True if all conditions are true.
 */
bool NetworkHttp::conditionFulfilled(const std::string &cond)
{
    std::vector<std::string> cond_list;
    cond_list = StringUtils::split(cond, ';');
    for(unsigned int i=0; i<cond_list.size(); i++)
    {
        std::vector<std::string> cond = StringUtils::split(cond_list[i],' ');
        if(cond.size()!=3)
        {
            printf("Invalid condition '%s' - assumed to be true.\n", 
                   cond_list[i].c_str());
            continue;
        }
        if(cond[0]=="stkversion")
        {
            int news_version = versionToInt(cond[2]);
            int stk_version  = versionToInt(STK_VERSION);
            if(cond[1]=="=")
            {
                if(news_version!=stk_version) return false;
                continue;
            }
            if(cond[1]=="<")
            {
                if(news_version>=stk_version) return false;
                continue;
            }
            if(cond[1]==">")
            {
                if(news_version<=stk_version) return false;
                continue;
            }
            printf("Invalid comparison in condition '%s' - assumed true.\n",
                   cond_list[i].c_str());
        }
        else
        {
            printf("Invalid condition '%s' - assumed to be true.\n", 
                   cond_list[i].c_str());
            continue;
        }

    }   // for i < cond_list
    return true;
}   // conditionFulfilled

// ----------------------------------------------------------------------------
/** Converts a version string (in the form of 'X.Y.Za' into an
 *  integer number.
 *  \param s The version string to convert.
 */
int NetworkHttp::versionToInt(const std::string &version_string)
{
    // Special case: SVN
    if(version_string=="SVN" || version_string=="svn")
      // SVN version will be version 99.99.99i
        return 100000*99
              +  1000*99
              +    10*99
              +        9;

    std::vector<std::string> l = StringUtils::split(version_string, '.');
    if(l.size()!=3)
    {
        printf("Invalid version string '%s'.\n", version_string.c_str());
        return -1;
    }
    const std::string &s=l[2];
    int very_minor=0;
    if(s[s.size()-1]>='a' && s[s.size()-1]<='z')
    {
        very_minor = s[s.size()-1]-'a'+1;
        l[2] = l[2].substr(0, s.size()-1);
    }
    int version = 100000*atoi(l[0].c_str())
                +   1000*atoi(l[1].c_str())
                +     10*atoi(l[2].c_str())
                + very_minor;
    if(version<=0)
        printf("Invalid version string '%s'.\n", s.c_str());
    return version;
}   // versionToInt

// ----------------------------------------------------------------------------
/** Reads the information about which message was dislpayed how often from
 *  the user config file.
 */
void NetworkHttp::updateMessageDisplayCount()
{
#ifdef NEWS_MESSAGE_REMOVAL
    m_news.lock();
    std::vector<std::string> pairs = 
        StringUtils::split(UserConfigParams::m_display_count,' ');
    for(unsigned int i=0; i<pairs.size(); i++)
    {
        std::vector<std::string> v = StringUtils::split(pairs[i], ':');
        int id, count;
        StringUtils::fromString(v[0], id);
        StringUtils::fromString(v[1], count);
        // Search all downloaded messages for this id. 
        for(unsigned int j=0; j<m_news.getData().size(); j++)
        {
            if(m_news.getData()[j].getMessageId()!=id)
                continue;
            m_news.getData()[j].setDisplayCount(count);
            if(count>stk_config->m_max_display_news)
                m_news.getData().erase(m_news.getData().begin()+j);
            break;
        }   // for j <m_news.getData().size()
    }
    m_news.unlock();
#endif
}   // updateMessageDisplayCount

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
    m_abort.set(false);
    CURL *session = curl_easy_init();

    std::string full_url = (std::string)UserConfigParams::m_server_addons 
                         + "/" + url;
    curl_easy_setopt(session, CURLOPT_URL, full_url.c_str());
    std::string uagent = (std::string)"SuperTuxKart/" + STK_VERSION;
    curl_easy_setopt(session, CURLOPT_USERAGENT, uagent.c_str());
        
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
    if(UserConfigParams::m_verbosity>=3)
        printf("[addons] Downloading %s\n", file.c_str());
    CURL *session = curl_easy_init();
    std::string full_url = (std::string)UserConfigParams::m_server_addons 
                         + "/" + file;
    curl_easy_setopt(session, CURLOPT_URL, full_url.c_str());
    std::string uagent = (std::string)"SuperTuxKart/" + STK_VERSION;
    curl_easy_setopt(session, CURLOPT_USERAGENT, uagent.c_str());
    FILE * fout = fopen(file_manager->getAddonsFile(save_filename).c_str(),
                         "wb");
        
    //from and out
    curl_easy_setopt(session,  CURLOPT_WRITEDATA,     fout  );
    curl_easy_setopt(session,  CURLOPT_WRITEFUNCTION, fwrite);
    
    // FIXME: if a network problem prevent the first 'list' download
    // to finish, the thread can not be cancelled. 
    // If we disable this test it works as expected, but I am not sure
    // if there are any side effects if synchron downloads use the
    // progress bar as well.
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
/** Signals to the progress function to request any ongoing download to be 
 *  cancelled. This function can also be called if there is actually no 
 *  download atm. The function progressDownload checks m_abort and will 
 *  return a non-zero value which causes libcurl to abort. */
void NetworkHttp::cancelDownload() 
{ 
    if(UserConfigParams::m_verbosity>=3)
        printf("[addons] Requesting cancellation of download.\n");
    m_abort.set(true); 
}   // cancelDownload

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
    if(UserConfigParams::m_verbosity>=3)
        printf("[addons] Download synchron '%s' as '%s'.\n",
               file.c_str(), save_filename.c_str());

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

    if(UserConfigParams::m_verbosity>=3)
        printf("[addons] Download asynchron '%s' as '%s'.\n", 
               file.c_str(), m_save_filename.c_str());
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
    // Check if we are asked to abort the download. If so, signal this
    // back to libcurl by returning a non-zero status.
    if(network_http->m_abort.get())
    {
        if(UserConfigParams::m_verbosity>=3)
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

