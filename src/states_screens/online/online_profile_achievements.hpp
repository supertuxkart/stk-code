//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 Glenn De Jonghe
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


#ifndef __HEADER_ONLINE_PROFILE_ACHIEVEMENTS_HPP__
#define __HEADER_ONLINE_PROFILE_ACHIEVEMENTS_HPP__

#include <string>
#include <irrString.h>

#include "achievements/achievement.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widgets.hpp"
#include "states_screens/online/online_profile_base.hpp"
#include "online/profile_manager.hpp"


namespace GUIEngine { class Widget; }


/**
  * \brief Online profiel overview screen
  * \ingroup states_screens
  */
class BaseOnlineProfileAchievements : public OnlineProfileBase,
                                      public GUIEngine::IListWidgetHeaderListener
{
private:

    GUIEngine::ListWidget *     m_achievements_list_widget;

    int                         m_selected_achievement_index;
    bool                        m_waiting_for_achievements;

    /** Which column to use for sorting. */
    int m_sort_column;

    bool m_sort_desc;

    bool m_sort_default;

    void displayResults();
    // True if a < b
    bool goalSort(Achievement *a, Achievement *b);
    bool progressSort(Achievement *a, Achievement *b);

protected:
    BaseOnlineProfileAchievements(const std::string &filename);

public:
    friend class GUIEngine::ScreenSingleton<BaseOnlineProfileAchievements>;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void eventCallback(GUIEngine::Widget* widget, const std::string& name, const int playerID) OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void init() OVERRIDE;

    virtual void onUpdate(float delta) OVERRIDE;

    virtual void beforeAddingWidget() OVERRIDE;
    virtual void onColumnClicked(int column_id, bool sort_desc, bool sort_default) OVERRIDE;

    // ------------------------------------------------------------------------
    virtual void refreshAchievementsList()
    { 
        m_waiting_for_achievements = true; 
    }   // refreshAchievementsList
};

// ============================================================================
/**
* \brief Online profiel overview screen
* \ingroup states_screens
*/
class TabOnlineProfileAchievements : public BaseOnlineProfileAchievements,
                public GUIEngine::ScreenSingleton<TabOnlineProfileAchievements>
{
protected:
    friend class GUIEngine::ScreenSingleton<TabOnlineProfileAchievements>;

    TabOnlineProfileAchievements() 
      : BaseOnlineProfileAchievements("online/profile_achievements_tab.stkgui")
    {}

};   // TabOnlineProfileAchievements

// ============================================================================
/**
* \brief Online profiel overview screen
* \ingroup states_screens
*/
class OnlineProfileAchievements : public BaseOnlineProfileAchievements,
    public GUIEngine::ScreenSingleton < OnlineProfileAchievements >
{
protected:
    friend class GUIEngine::ScreenSingleton<OnlineProfileAchievements>;

    OnlineProfileAchievements()
        : BaseOnlineProfileAchievements("online/profile_achievements.stkgui")
    {}

};   // class


#endif
