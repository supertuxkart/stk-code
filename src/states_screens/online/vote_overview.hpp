//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2018 Joerg Henrichs
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

#ifndef HEADER_VOTE_OVERVIEW_HPP
#define HEADER_VOTE_OVERVIEW_HPP

#include "guiengine/screen.hpp"
#include "utils/synchronised.hpp"
#include <deque>
#include <limits>
#include <map>
#include <string>

namespace GUIEngine
{
    class CheckBoxWidget;
    class LabelWidget;
    class SpinnerWidget;
}

/**
  * \brief screen where the user can select a track
  * \ingroup states_screens
  */
class VoteOverview: public GUIEngine::Screen,
                    public GUIEngine::ScreenSingleton<VoteOverview>
{
    friend class GUIEngine::ScreenSingleton<VoteOverview>;

private:

    /** Pointer to progress bar widget which is used as a timer
    *  (going backwards). */
    GUIEngine::ProgressBarWidget *m_timer;

    bool m_reverse_checked, m_quit_server;

    int m_bottom_box_height;

    std::map<std::string, core::stringw> m_vote_messages;

    std::deque<std::string> m_random_track_list;

    VoteOverview() : Screen("online/vote_overview.stkgui")
    {
        m_reverse_checked = false;
        m_quit_server = false;
        m_bottom_box_height = -1;
    }

public:

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void eventCallback(GUIEngine::Widget* widget,
                               const std::string& name,
                               const int playerID) OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void init() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void beforeAddingWidget() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void tearDown() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual bool onEscapePressed() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void onUpdate(float dt) OVERRIDE;
    // ------------------------------------------------------------------------
    void setQuitServer() { m_quit_server = true; }
    // ------------------------------------------------------------------------
    void resetVote()
    {
        m_vote_messages.clear();
    }
    // ------------------------------------------------------------------------
    void addVoteMessage(const std::string& user,
                        const irr::core::stringw& message)
    {
        m_vote_messages[user] = message;
    }

};

#endif
