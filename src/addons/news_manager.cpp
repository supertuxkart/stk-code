//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011-2015 Joerg Henrichs
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

#ifndef SERVER_ONLY

#include "addons/news_manager.hpp"

#include "config/user_config.hpp"
#include "config/stk_config.hpp"
#include "io/file_manager.hpp"
#include "online/http_request.hpp"
#include "online/request_manager.hpp"
#include "states_screens/addons_screen.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "utils/string_utils.hpp"
#include "utils/time.hpp"
#include "utils/translation.hpp"
#include "utils/vs.hpp"

#include <functional>
#include <iostream>

using namespace Online;

NewsManager *NewsManager::m_news_manager = nullptr;
std::string NewsManager::m_news_filename = "online_news.xml";

// ----------------------------------------------------------------------------
NewsManager::NewsManager()
{
    for (int i = 0; i < NTYPE_COUNT; i++)
    {
        m_current_news_ptr[i] = -1;
        m_news_prioritize_after_id[i] = -1;
    }
    m_error_message.setAtomic("");
    m_force_refresh = false;

    // Clean .part file which may be left behind
    std::string news_part = file_manager->getAddonsFile(m_news_filename + ".part");
    if (file_manager->fileExists(news_part))
        file_manager->removeFile(news_part);

    init(false);
}   // NewsManage

// ---------------------------------------------------------------------------
NewsManager::~NewsManager()
{
    // If the download thread doesn't finish on time we detach the thread to
    // avoid exception
    if (m_download_thread.joinable())
    {
        if (!CanBeDeleted::canBeDeletedNow())
            m_download_thread.detach();
        else
            m_download_thread.join();
    }
}   // ~NewsManager

// ---------------------------------------------------------------------------
/** This function initialises the data for the news manager. It starts a
 *  separate thread to execute downloadNews() - which (if necessary) downloads
 *  the m_news_filename file and updates the list of news messages. It also
 *  initialises the addons manager (which can trigger another download of
 *  m_news_filename).
 *  \param force_refresh Re-download m_news_filename, even if
 */
void NewsManager::init(bool force_refresh)
{
    if (m_download_thread.joinable())
        return;

    m_force_refresh = force_refresh;

    // The rest (which potentially involves downloading m_news_filename) is handled
    // in a separate thread, so that the GUI remains responsive. It is only
    // started if internet access is enabled, else nothing is done in the
    // thread anyway (and the addons menu is disabled as a result).
    if(UserConfigParams::m_internet_status==RequestManager::IPERM_ALLOWED)
    {
        CanBeDeleted::resetCanBeDeleted();
        m_download_thread = std::thread(std::bind(
            &NewsManager::downloadNews, this));
    }
}   //init

// ---------------------------------------------------------------------------
/** This function submits request which will download the m_news_filename file
 *  if necessary. It is running in its own thread, so we can use blocking
 *  download calls without blocking the GUI.
 */
void NewsManager::downloadNews()
{
    VS::setThreadName("downloadNews");
    clearErrorMessage();

    std::string xml_file = file_manager->getAddonsFile(m_news_filename);
    // Prevent downloading when .part file created, which is already downloaded
    std::string xml_file_part = file_manager->getAddonsFile(m_news_filename + ".part");
    bool news_exists = file_manager->fileExists(xml_file);

    // The news message must be updated if either it has never been updated,
    // or if the time of the last update was more than news_frequency ago,
    // or because a 'refresh' was explicitly requested by the user, or no
    // m_news_filename file exists.
    bool download = ( UserConfigParams::m_news_last_updated==0  ||
                      UserConfigParams::m_news_last_updated
                          +UserConfigParams::m_news_frequency
                        < StkTime::getTimeSinceEpoch()          ||
                      m_force_refresh                       ||
                      !news_exists                                    )
         && UserConfigParams::m_internet_status==RequestManager::IPERM_ALLOWED
         && !file_manager->fileExists(xml_file_part);
    const XMLNode *xml = NULL;

    if(!download && news_exists)
    {
        // If (so far) we don't need to download, there should be an existing
        // file. Try to read this, and do some basic checks
        xml = new XMLNode(xml_file);
        // A proper news file has at least a version number, mtime, frequency
        // and an include node (which contains addon data) defined. If this is
        // not the case, assume that it is an invalid download, or a corrupt
        // local file. Try downloading again after resetting the news server
        // back to the default.
        int version=-1;
        if( !xml->get("version",   &version)  || version!=1 ||
            !xml->get("mtime",     &version)  ||
            !xml->getNode("include")          ||
            !xml->get("frequency", &version)                )
        {
            delete xml;
            xml       = NULL;
            download = true;
        }   // if xml not consistemt
    }   // if !download

    if(download)
    {
        core::stringw error_message("");

        auto download_req = std::make_shared<HTTPRequest>(m_news_filename);
        download_req->setAddonsURL(m_news_filename);

        // Initialise the online portion of the addons manager.
        if(UserConfigParams::logAddons())
            Log::info("addons", "Downloading news.");
        download_req->executeNow();

        if(download_req->hadDownloadError())
        {
            // Assume that the server address is wrong. And retry
            // with the default server address again (just in case
            // that a redirect went wrong, or a wrong/incorrect
            // address somehow made its way into the config file.
            // We need a new object, since the state of the old
            // download request is now done.
            download_req = std::make_shared<HTTPRequest>(m_news_filename);

            // make sure the new server address is actually used
            download_req->setAddonsURL(m_news_filename);
            download_req->executeNow();

            if(download_req->hadDownloadError())
            {
                // This message must be translated dynamically in the main menu.
                // If it would be translated here, it wouldn't be translated
                // if the language is changed in the menu!
                error_message = N_("Error downloading news: '%s'.");
                const char *const curl_error = download_req->getDownloadErrorMessage();
                error_message = StringUtils::insertValues(error_message, curl_error);
                addons_manager->setErrorState();
                setErrorMessage(error_message);
                Log::error("news", core::stringc(error_message).c_str());
            }   // hadDownloadError
        }   // hadDownloadError

        if(!download_req->hadDownloadError())
            UserConfigParams::m_news_last_updated = StkTime::getTimeSinceEpoch();

        // No download error, update the last_updated time value, and delete
        // the potentially loaded xml file
        delete xml;
        xml = NULL;
    }   // hadDownloadError

    if(xml) delete xml;
    xml = NULL;

    // Process new.xml now.
    if(file_manager->fileExists(xml_file))
    {
        xml = new XMLNode(xml_file);
        checkRedirect(xml);
        updateNews(xml, xml_file);
        if (addons_manager)
            addons_manager->init(xml, m_force_refresh);
        delete xml;
    }

    // We can't finish stk (esp. delete the file manager) before
    // this part of the code is reached (since otherwise the file
    // manager might be access after it was deleted).
    CanBeDeleted::setCanBeDeleted();
}   // downloadNews

// ---------------------------------------------------------------------------
/** Checks if a redirect is received, causing a new server to be used for
 *  downloading addons.
 *  \param xml XML data structure containing the redirect information.
 */
void NewsManager::checkRedirect(const XMLNode *xml)
{
    if (stk_config->m_allow_news_redirects)
    {
        // NOTE: Before 0.10 there were just two redirect attributes
        // "redirect" - addons server (contains /dl/xml/ path)
        // "hw-report-server" - hardware report server

        // Redirect for the new addons server
        std::string new_addons_server;
        if (xml->get("redirect-server-addons", &new_addons_server) == 1 && !new_addons_server.empty())
        {
            if (UserConfigParams::logAddons())
            {
                Log::info("[Addons]", "Current addons server: '%s'\n [Addons] New addons server: '%s'",
                            stk_config->m_server_addons.c_str(), new_addons_server.c_str());
            }
            stk_config->m_server_addons = new_addons_server;
        }

        // Redirect for the API server
        std::string new_api_server;
        if (xml->get("redirect-server-api", &new_api_server) == 1 && !new_api_server.empty())
        {
            if (UserConfigParams::logAddons())
            {
                Log::info("[Addons]", "Current API server: '%s'\n [Addons] New API server: '%s'",
                            stk_config->m_server_api.c_str(), new_api_server.c_str());
            }
            stk_config->m_server_api = new_api_server;
        }

        // Redirect for the hardware report server
        std::string new_hardware_report_server;
        if (xml->get("redirect-server-hardware-report", &new_hardware_report_server) == 1 && !new_hardware_report_server.empty())
        {
            Log::info("hw report", "Current hardware report  server: '%s'\n [hw report] New hardware report server: '%s'",
                        stk_config->m_server_hardware_report.c_str(), new_hardware_report_server.c_str());
            stk_config->m_server_hardware_report = new_hardware_report_server;
        }
    }

    // Update menu/game polling interval
    float polling;
    if (xml->get("menu-polling-interval", &polling))
    {
        RequestManager::get()->setMenuPollingInterval(polling);
    }
    if (xml->get("game-polling-interval", &polling))
    {
        RequestManager::get()->setGamePollingInterval(polling);
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
void NewsManager::updateNews(const XMLNode *xml, const std::string &filename)
{
    bool error = true;
    int frequency=0;
    if(xml->get("frequency", &frequency))
        UserConfigParams::m_news_frequency = frequency;

    for (int type = 0; type < NTYPE_COUNT; type++)
    {
        // This function is also called in case of a reinit, so
        // we have to delete existing news messages here first.
        m_news[type].lock();
        m_news[type].getData().clear();
    }

    for(unsigned int i=0; i<xml->getNumNodes(); i++)
    {
        const XMLNode *node = xml->getNode(i);

        NewsType type;

        if (node->getName() == "message")
        {
            type = NTYPE_MAINMENU;
        }
        else if (node->getName() == "list")
        {
            type = NTYPE_LIST;
        }
        else continue;

        std::string raw_news;
        node->get("content", &raw_news);
        core::stringw news = StringUtils::xmlDecode(raw_news);
        int id=-1;
        node->get("id", &id);
        bool important=false;
        node->get("important", &important);
        std::string date="";
        node->get("date", &date);
        std::string link="";
        node->get("link", &link);

        std::string cond;
        node->get("condition", &cond);
        if(!conditionFulfilled(cond))
            continue;

        NewsMessage n(news, id, date, link, important);

        m_news[type].getData().push_back(n);

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
        NewsMessage n(_("Failed to connect to the SuperTuxKart add-ons server."), -1);

        for (int type = 0; type < NTYPE_COUNT; type++)
        {
            m_news[type].getData().push_back(n);
            m_news[type].unlock();
        }
    }
    else
    {
        for (int type = 0; type < NTYPE_COUNT; type++)
        {
            std::sort(
                m_news[type].getData().begin(),
                m_news[type].getData().end()
            );

            int after_id = m_news_prioritize_after_id[type];
            if (after_id != -1)
            {
                std::stable_partition(
                    m_news[type].getData().begin(),
                    m_news[type].getData().end(),
                    [after_id](NewsMessage& n)->bool{
                        return n.getMessageId() >= after_id;
                    }
                );
            }
            m_news[type].unlock();
        }
    }
}   // updateNews

// ----------------------------------------------------------------------------
/** Add a news message. This is used to add error messages, e.g. for problems
 *  when downloading addons.
 *  \param s The news message to add.
 */
void NewsManager::addNewsMessage(NewsType type, const core::stringw &s)
{
    NewsMessage n(s, -1);
    m_news[type].lock();
    m_news[type].getData().push_back(n);
    m_news[type].unlock();
}   // addNewsMessage
// ----------------------------------------------------------------------------
/** Returns the message pointed by the current ptr
 */
const core::stringw NewsManager::getCurrentNewsMessage(NewsType type)
{
    // Only display error message in case of a problem.
    if (m_error_message.getAtomic().size()>0)
        return _(m_error_message.getAtomic().c_str());
    
    core::stringw message = L"";

    m_news[type].lock();

    if (m_current_news_ptr[type] >= 0
     && m_current_news_ptr[type] < m_news[type].getData().size())
    {
        message = m_news[type].getData()[m_current_news_ptr[type]].getNews();
    }
    m_news[type].unlock();

    return message;
}   // getCurrentNewsMessage
// ----------------------------------------------------------------------------
/** Returns the date of the news pointed by the current ptr
 */
const std::string NewsManager::getCurrentNewsDate(NewsType type)
{
    std::string date = "";

    m_news[type].lock();

    if (m_current_news_ptr[type] >= 0
     && m_current_news_ptr[type] < m_news[type].getData().size())
    {
        date = m_news[type].getData()[m_current_news_ptr[type]].getDate();
    }
    m_news[type].unlock();

    return date;
}   // getCurrentNewsMessage
// ----------------------------------------------------------------------------
/** Returns the date of the news pointed by the current ptr
 */
const std::string NewsManager::getCurrentNewsLink(NewsType type)
{
    std::string link = "";

    m_news[type].lock();

    if (m_current_news_ptr[type] >= 0
     && m_current_news_ptr[type] < m_news[type].getData().size())
    {
        link = m_news[type].getData()[m_current_news_ptr[type]].getLink();
    }
    m_news[type].unlock();

    return link;
}   // getCurrentNewsMessage
// ----------------------------------------------------------------------------
/** Returns the importance of the message pointed by the current ptr
 */
const bool NewsManager::isCurrentNewsImportant(NewsType type)
{
    bool importance = false;

    m_news[type].lock();

    if (m_current_news_ptr[type] >= 0
     && m_current_news_ptr[type] < m_news[type].getData().size())
    {
        importance = m_news[type].getData()[m_current_news_ptr[type]].isImportant();
    }
    m_news[type].unlock();

    return importance;
}   // isCurrentNewsImportant

// ----------------------------------------------------------------------------
const int NewsManager::getNewsCount(NewsType type)
{
    // Only show error message one time
    if(m_error_message.getAtomic().size()>0)
        return 1;
    
    m_news[type].lock();
    int count = m_news[type].getData().size();
    m_news[type].unlock();

    return count;
}

// ----------------------------------------------------------------------------
void NewsManager::prioritizeNewsAfterID(NewsType type, int id)
{
    m_news[type].lock();

    std::sort(
        m_news[type].getData().begin(),
        m_news[type].getData().end()
    );
    if (id != -1)
    {
        std::stable_partition(
            m_news[type].getData().begin(),
            m_news[type].getData().end(),
            [id](NewsMessage& n)->bool{
                return n.getMessageId() > id;
            }
        );
    }

    m_news_prioritize_after_id[type] = id;
    m_news[type].unlock();
}

// ----------------------------------------------------------------------------
/** Returns the next loaded news message. It will 'wrap around', i.e.
 *  if there is only one message it will be returned over and over again.
 *  To be used by the the main menu to get the next news message after
 *  one message was scrolled off screen.
 */
const int NewsManager::getNextNewsID(NewsType type)
{
    if (m_error_message.getAtomic().size()>0)
        return -1;

    m_news[type].lock();

    if(m_news[type].getData().size()==0)
    {
        // Lock
        m_news[type].unlock();
        return -1;
    }

    int m = -1;
    {
        m_current_news_ptr[type]++;
        if(m_current_news_ptr[type] >= (int)m_news[type].getData().size())
            m_current_news_ptr[type] = 0;

        m = m_news[type].getData()[m_current_news_ptr[type]].getMessageId();
    }
    m_news[type].unlock();
    return m;
}   // getNextNewsMessage

// ----------------------------------------------------------------------------
/** Checks if the given condition list are all fulfilled.
 *  The conditions must be separated by ";", and each condition
 *  must be of the form "type comp version".
 *  Type must be 'stkversion'
 *  comp must be one of "<", "=", ">"
 *  version must be a valid STK version string
 *  \param cond The list of conditions
 *  \return True if all conditions are true.
 */
bool NewsManager::conditionFulfilled(const std::string &cond)
{
    std::vector<std::string> cond_list;
    cond_list = StringUtils::split(cond, ';');
    for(unsigned int i=0; i<cond_list.size(); i++)
    {
        std::vector<std::string> cond = StringUtils::split(cond_list[i],' ');
        if(cond.size()!=3)
        {
            Log::warn("NewsManager", "Invalid condition '%s' - assumed to "
                                     "be true.", cond_list[i].c_str());
            continue;
        }
        // Check for stkversion comparisons
        // ================================
        if(cond[0]=="stkversion")
        {
            int news_version = StringUtils::versionToInt(cond[2]);
            int stk_version  = StringUtils::versionToInt(STK_VERSION);
            if(cond[1]=="=")
            {
                if(stk_version!=news_version) return false;
                continue;
            }
            if(cond[1]=="<")
            {
                if(stk_version>=news_version) return false;
                continue;
            }
            if(cond[1]==">")
            {
                if(stk_version<=news_version) return false;
                continue;
            }
            Log::warn("NewsManager", "Invalid comparison in condition '%s' - "
                                     "assumed true.", cond_list[i].c_str());
        }
        // Check for addons not installed
        // ==============================
        else if(cond[1]=="not" && cond[2]=="installed")
        {
            // The addons_manager cannot be accessed, since it's
            // being initialised after the news manager. So a simple
            // test is made to see if the directory exists. It is
            // necessary to check for karts and tracks separately,
            // since it's not possible to know if the addons is
            // a kart or a track.
            const std::string dir=file_manager->getAddonsDir();
            if(file_manager->fileExists(dir+"/karts/"+cond[0]))
                return false;
            if(file_manager->fileExists(dir+"/tracks/"+cond[0]))
                return false;
            continue;
        }
        else
        {
            Log::warn("NewsManager", "Invalid condition '%s' - assumed to "
                                     "be true.", cond_list[i].c_str());
            continue;
        }

    }   // for i < cond_list
    return true;
}   // conditionFulfilled

// ----------------------------------------------------------------------------

#endif
