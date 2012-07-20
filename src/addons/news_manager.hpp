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

#ifndef HEADER_NEWS_MANAGER_HPP
#define HEADER_NEWS_MANAGER_HPP

#include <string>
#include <vector>


#include <irrString.h>
using namespace irr;

#include "utils/synchronised.hpp"

class XMLNode;

/**
  * \ingroup addonsgroup
  */
class NewsManager
{
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

    /** A high priority error message that is shown instead of
     *  any news message (usually indicating connection problems). */
    core::stringw    m_error_message;

    void          checkRedirect(const XMLNode *xml);
    void          updateNews(const XMLNode *xml,
                             const std::string &filename);
    void          updateUserConfigFile() const;
    bool          conditionFulfilled(const std::string &cond);
    void          updateMessageDisplayCount();

public:
                  NewsManager();
                 ~NewsManager();
    const core::stringw
                  getNextNewsMessage();
    void          init();
    void          addNewsMessage(const core::stringw &s);

    /** Sets an error message that is displayed instead of any news message. */
    void          setErrorMessage(const core::stringw &s) { m_error_message=s;}
    // ------------------------------------------------------------------------
    /** Clears the error message. */
    void          clearErrorMessage() {m_error_message=""; }
    // ------------------------------------------------------------------------
};   // NewsManager

extern NewsManager *news_manager;

#endif

