//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011 Joerg Henrichs
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

#include "addons/news_manager.hpp"

#include "config/user_config.hpp"
#include "io/file_manager.hpp"
#include "states_screens/addons_screen.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "utils/string_utils.hpp"
#include "utils/time.hpp"

#include <iostream>

NewsManager *news_manager=NULL;

// ----------------------------------------------------------------------------
NewsManager::NewsManager() : m_news(std::vector<NewsMessage>())
{
    m_current_news_message = -1;
    m_error_message        = "";
}   // NewsManage

// ---------------------------------------------------------------------------
NewsManager::~NewsManager()
{
}   // ~NewsManager

// ---------------------------------------------------------------------------
/** Initialises the online part of the network manager. It downloads the
 *  news.xml file from the server (if the frequency of downloads makes this
 *  necessary), and (again if necessary) the addons.xml file.
 *  \return 0 if an error happened and no online connection will be available,
 *          1 otherwise.
 */
void NewsManager::init()
{
    UserConfigParams::m_news_last_updated = Time::getTimeSinceEpoch();

    std::string xml_file = file_manager->getAddonsFile("news.xml");
    const XMLNode *xml   = new XMLNode(xml_file);
    checkRedirect(xml);
    updateNews(xml, xml_file);
    delete xml;
}   // init

// ---------------------------------------------------------------------------
/** Checks if a redirect is received, causing a new server to be used for
 *  downloading addons.
 *  \param xml XML data structure containing the redirect information.
 */
void NewsManager::checkRedirect(const XMLNode *xml)
{
    std::string new_server;
    int result = xml->get("redirect", &new_server);
    if(result==1 && new_server!="")
    {
        if(UserConfigParams::logAddons())
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
void NewsManager::updateNews(const XMLNode *xml, const std::string &filename)
{
    // This function is also called in case of a reinit, so
    // we have to delete existing news messages here first.
    m_news.lock();
    m_news.getData().clear();
    m_news.unlock();
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
        }   // m_news.lock()
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
/** Add a news message. This is used to add error messages, e.g. for problems
 *  when downloading addons.
 *  \param s The news message to add.
 */
void NewsManager::addNewsMessage(const core::stringw &s)
{
    NewsMessage n(s, -1);
    m_news.lock();
    m_news.getData().push_back(n);
    m_news.unlock();
}   // addNewsMessage
// ----------------------------------------------------------------------------
/** Returns the next loaded news message. It will 'wrap around', i.e.
 *  if there is only one message it will be returned over and over again.
 *  To be used by the the main menu to get the next news message after
 *  one message was scrolled off screen.
 */
const core::stringw NewsManager::getNextNewsMessage()
{
    // Only display error message in case of a problem.
    if(m_error_message.size()>0)
        return _(m_error_message.c_str());

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
    return _(m.c_str());
}   // getNextNewsMessage

// ----------------------------------------------------------------------------
/** Saves the information about which message was being displayed how often
 *  to the user config file. It dnoes not actually save the user config
 *  file, this is left to the main program (user config is saved at
 *  the exit of the program).
 *  Note that this function assumes that m_news is already locked!
 */
void NewsManager::updateUserConfigFile() const
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
bool NewsManager::conditionFulfilled(const std::string &cond)
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
        // Check for stkversion comparisons
        // ================================
        if(cond[0]=="stkversion")
        {
            int news_version = versionToInt(cond[2]);
            int stk_version  = versionToInt(STK_VERSION);
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
            printf("Invalid comparison in condition '%s' - assumed true.\n",
                   cond_list[i].c_str());
        }
        // Check for addons not installed
        // ==============================
        else if(cond[1]=="not" && cond[2]=="installed")
        {
            // The addons_manager can not be access, since it's
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
            printf("Invalid condition '%s' - assumed to be true.\n", 
                   cond_list[i].c_str());
            continue;
        }

    }   // for i < cond_list
    return true;
}   // conditionFulfilled

// ----------------------------------------------------------------------------
/** Converts a version string (in the form of 'X.Y.Za-rcU' into an
 *  integer number.
 *  \param s The version string to convert.
 */
int NewsManager::versionToInt(const std::string &version_string)
{
    // Special case: SVN
    if(version_string=="SVN" || version_string=="svn")
      // SVN version will be version 99.99.99i-rcJ
        return 1000000*99     
              +  10000*99
              +    100*99
              +     10* 9
              +         9;

    std::string s=version_string;
    // To guarantee that a release gets a higher version number than 
    // a release candidate, we assign a 'release_candidate' number
    // of 9 to versions which are not a RC. We assert that any RC
    // is less than 9 to guarantee the ordering.
    int release_candidate=9;
    if(sscanf(s.substr(s.length()-4, 4).c_str(), "-rc%d", 
           &release_candidate)==1)
    {
        s = s.substr(0, s.length()-4);
        // Otherwise a RC can get a higher version number than
        // the corresponding release! If this should ever get
        // triggered, multiply all scaling factors above and
        // below by 10, to get two digits for RC numbers.
        assert(release_candidate<9);
    }
    int very_minor=0;
    if(s[s.size()-1]>='a' && s[s.size()-1]<='z')
    {
        very_minor = s[s.size()-1]-'a'+1;
        s = s.substr(0, s.size()-1);
    }
    std::vector<std::string> l = StringUtils::split(s, '.');
    while(l.size()<3)
        l.push_back(0);
    int version = 1000000*atoi(l[0].c_str())
                +   10000*atoi(l[1].c_str())
                +     100*atoi(l[2].c_str())
                +      10*very_minor
                +         release_candidate;

    if(version<=0)
        printf("Invalid version string '%s'.\n", s.c_str());
    return version;
}   // versionToInt

// ----------------------------------------------------------------------------
/** Reads the information about which message was dislpayed how often from
 *  the user config file.
 */
void NewsManager::updateMessageDisplayCount()
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

