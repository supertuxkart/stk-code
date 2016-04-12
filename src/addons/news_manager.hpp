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

#include <string>
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
private:
    static NewsManager *m_news_manager;

    // A wrapper class to store news message together with
    // a message id and a display count.
    class NewsMessage
    {
        /** The actual news message. */
        core::stringw m_news;
        /** A message id used to store some information in the
         *  user config file. */
        int           m_message_id;
        /** Counts how often a message has been displayed. */
        int           m_display_count;
        /** True if this is an important (i.e. popup) message. */
        bool          m_important;

    public:
        NewsMessage(const core::stringw &m, int id, bool important=false)
        {
            m_news          = m;
            m_message_id    = id;
            m_display_count = 0;
            m_important     = important;
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
        /** Returns if this is an important message. */
        bool isImportant() const { return m_important; }
    };   // NewsMessage

    mutable Synchronised< std::vector<NewsMessage> > m_news;

    /** Index of the current news message that is being displayed. */
    int             m_current_news_message;

    /** A single string that concatenats all news messages, separated
     *  by "  +++  ". Using this to display the news message avoids
     *  the delay between messages. */
    core::stringw   m_all_news_messages;

    /** Stores the news message display count from the user config file.
    */
    std::vector<int> m_saved_display_count;

    /** A high priority error message that is shown instead of
     *  any news message (usually indicating connection problems). */
    Synchronised<core::stringw>    m_error_message;

    /** True when all .xml files should be re-downloaded. */
    bool m_force_refresh;

    void          checkRedirect(const XMLNode *xml);
    void          updateNews(const XMLNode *xml,
                             const std::string &filename);
    bool          conditionFulfilled(const std::string &cond);
    static void*  downloadNews(void *obj);
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
    static void deallocate()
    {
        if(m_news_manager)
        {
            delete m_news_manager;
            m_news_manager = NULL;
        }
    }   // deallocate
    // ------------------------------------------------------------------------
    const core::stringw
                  getNextNewsMessage();
    const core::stringw
                  getImportantMessage();
    void          init(bool force_refresh);
    void          addNewsMessage(const core::stringw &s);

    // ------------------------------------------------------------------------
    /** Sets an error message that is displayed instead of any news message. */
    void          setErrorMessage(const core::stringw &s)
    {
        m_error_message.setAtomic(s);
    }   // setErrorMessage
    // ------------------------------------------------------------------------
    /** Clears the error message. */
    void          clearErrorMessage() {m_error_message.setAtomic(""); }
    // ------------------------------------------------------------------------
};   // NewsManager

extern NewsManager *news_manager;

#endif

