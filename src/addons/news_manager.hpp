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

#ifndef HEADER_NEWS_MANAGER_HPP
#define HEADER_NEWS_MANAGER_HPP

#ifndef SERVER_ONLY

#include <string>
#include <thread>
#include <vector>


#include <irrString.h>
using namespace irr;

#include "utils/can_be_deleted.hpp"
#include "utils/synchronised.hpp"

class XMLNode;

/**
  * \ingroup addonsgroup
  */
class NewsManager : public CanBeDeleted
{
public:
    // Means the exact place the news being shown
    enum NewsType : uint8_t
    {
        NTYPE_MAINMENU,
        NTYPE_LIST,
        NTYPE_COUNT
    };

private:
    static NewsManager *m_news_manager;

    // A wrapper class to store news message together with
    // a message id and a display count.
    class NewsMessage
    {
        /** The actual news message. */
        core::stringw m_news;
        /** Additional data*/
        std::string   m_date;
        std::string   m_link;
        /** A message id used to store some information in the
         *  user config file. */
        int           m_message_id;
        /** Counts how often a message has been displayed. */
        int           m_display_count;
        /** True if this is an important (i.e. popup) message. */
        bool          m_important;

    public:
        NewsMessage(const core::stringw &m, int id, const std::string &date="", 
                    const std::string &link="", bool important=false)
        {
            m_news          = m;
            m_date          = date;
            m_link          = link;
            m_message_id    = id;
            m_display_count = 0;
            m_important     = important;
        }   // NewsMessage
        /** Returns the news message. */
        const core::stringw& getNews() const { return m_news; }
        /** Returns the date of the news. */
        const std::string& getDate() const { return m_date; }
        /** Returns the link of the news. */
        const std::string& getLink() const { return m_link; }
        /** Increases how often this message was being displayed. */
        void increaseDisplayCount() { m_display_count++; }
        /** Returns the news id. */
        int  getMessageId() const { return m_message_id; }
        /** Returns the display count. */
        int getDisplayCount() const { return m_display_count; }
        /** Sets the display count for this message. */
        void setDisplayCount(int n) { m_display_count = n; }
        /** Returns if this is an important message. */
        bool isImportant() const { return m_important; }
        /** For sorting the news */
        bool operator<(NewsMessage other) const
        {
            return m_important == other.m_important
                 ? m_message_id > other.m_message_id
                 : m_important;
        }
    };   // NewsMessage

    /** The name of the news file on the remote server */
    static std::string m_news_filename;

    /** m_news[NewsType] means all news within this type. */
    mutable Synchronised< std::vector<NewsMessage> > m_news[NTYPE_COUNT];

    /** Index of the current news message that is being displayed. */
    int             m_current_news_ptr[NTYPE_COUNT];
    /** News after this ID would be shown on the top regardless of importance. */
    int             m_news_prioritize_after_id[NTYPE_COUNT];

    /** Stores the news message display count from the user config file.
    */
    std::vector<int> m_saved_display_count;

    /** A high priority error message that is shown instead of
     *  any news message (usually indicating connection problems). */
    Synchronised<core::stringw>    m_error_message;

    /** True when all .xml files should be re-downloaded. */
    bool m_force_refresh;

    std::thread m_download_thread;

    void          checkRedirect(const XMLNode *xml);
    void          updateNews(const XMLNode *xml,
                             const std::string &filename);
    bool          conditionFulfilled(const std::string &cond);
    void          downloadNews();
    NewsManager();
    ~NewsManager();

public:
    /** Singleton: if necessary create and get the news managers */
    static NewsManager* get()
    {
        if(!m_news_manager)
            m_news_manager = new NewsManager();
        return m_news_manager;
    }   // get
    // ------------------------------------------------------------------------
    static bool isRunning() { return m_news_manager != NULL; }
    // ------------------------------------------------------------------------
    static void deallocate()
    {
        if(m_news_manager)
        {
            delete m_news_manager;
            m_news_manager = NULL;
        }
    }   // deallocate
    // ------------------------------------------------------------------------
    /** News are stored in an array, it would iterate to
     * the next one when calling this function and
     * would go back to the first one when the iteration ends. */
    const int     getNextNewsID(NewsType type);
    const core::stringw
                  getCurrentNewsMessage(NewsType type);
    const std::string
                  getCurrentNewsDate(NewsType type);
    const std::string
                  getCurrentNewsLink(NewsType type);
    const bool    isCurrentNewsImportant(NewsType type);
    const int     getNewsCount(NewsType type);

    /** Set id = -1 to disable it */
    void          prioritizeNewsAfterID(NewsType type, int id);
    
    void          init(bool force_refresh);
    void          addNewsMessage(NewsType type, const core::stringw &s);

    // ------------------------------------------------------------------------
    /** Goes back to the place before first message when called. */
    void          resetNewsPtr(NewsType type) { m_current_news_ptr[type] = -1; }
    /** Check if this type of news is on the way of fetching */
    bool          isNewsFetching(NewsType type) { return m_current_news_ptr[type] != -1; }
    // ------------------------------------------------------------------------
    /** Sets an error message that is displayed instead of any news message. */
    void          setErrorMessage(const core::stringw &s)
    {
        m_error_message.setAtomic(s);
    }   // setErrorMessage
    // ------------------------------------------------------------------------
    /** Clears the error message. */
    void          clearErrorMessage() { m_error_message.setAtomic(""); }
    // ------------------------------------------------------------------------
    void          joinDownloadThreadIfExit()
    {
        if (CanBeDeleted::canBeDeletedNow() && m_download_thread.joinable())
            m_download_thread.join();
    }
};   // NewsManager

#endif
#endif
