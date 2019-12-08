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

#include "states_screens/online/online_profile_achievements.hpp"

#include "achievements/achievement_info.hpp"
#include "achievements/achievements_manager.hpp"
#include "config/player_manager.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widget.hpp"
#include "online/online_profile.hpp"
#include "states_screens/dialogs/achievement_progress_dialog.hpp"
#include "states_screens/dialogs/player_rankings_dialog.hpp"
#include "states_screens/dialogs/user_info_dialog.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <IGUIButton.h>

#include <iostream>
#include <sstream>

using namespace GUIEngine;
using namespace irr::core;
using namespace irr::gui;
using namespace Online;

// -----------------------------------------------------------------------------
/** Constructor.
 */
BaseOnlineProfileAchievements::BaseOnlineProfileAchievements(const std::string &name)
                             : OnlineProfileBase(name)
{
    m_selected_achievement_index = -1;
    m_sort_column = 0;
    m_sort_desc = false;
    m_sort_default = true;
}   // BaseOnlineProfileAchievements

// -----------------------------------------------------------------------------
/** Callback when the xml file was loaded.
 */
void BaseOnlineProfileAchievements::loadedFromFile()
{
    OnlineProfileBase::loadedFromFile();
    m_achievements_list_widget = getWidget<ListWidget>("achievements_list");
    assert(m_achievements_list_widget != NULL);
    m_achievements_list_widget->setColumnListener(this);

}   // loadedFromFile

// ----------------------------------------------------------------------------
/** Callback before widgets are added. Clears all widgets.
 */
void BaseOnlineProfileAchievements::beforeAddingWidget()
{
    OnlineProfileBase::beforeAddingWidget();
    m_achievements_list_widget->clearColumns();
    m_achievements_list_widget->addColumn( _("Name"), 2 );

    // For the current player (even if not logged in, i.e. m_visiting_profile
    // = NULL) user achievement progress will  also be displayed
    if(!m_visiting_profile || m_visiting_profile->isCurrentUser())
    {
        // I18N: Goals in achievement
        m_achievements_list_widget->addColumn( _("Goals"), 1 );
        // I18N: Progress in achievement
        m_achievements_list_widget->addColumn( _("Progress"), 1 );
    }
}   // beforeAddingWidget

// -----------------------------------------------------------------------------
/** Called when entering this menu (after widgets have been added).
*/
void BaseOnlineProfileAchievements::init()
{
    OnlineProfileBase::init();
    if (m_profile_tabs)
    {
        m_profile_tabs->select(m_achievements_tab->m_properties[PROP_ID],
            PLAYER_ID_GAME_MASTER);
        m_profile_tabs->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    }

    // For current user add the progrss information.
    // m_visiting_profile is NULL if the user is not logged in.
    if(!m_visiting_profile || m_visiting_profile->isCurrentUser())
    {
        displayResults();
    }
    else
    {
        // Show achievements of a remote user. Set the waiting flag
        // and submit a request to get the achievement data.
        m_waiting_for_achievements = true;
        m_visiting_profile->fetchAchievements();
        m_achievements_list_widget->clear();
        m_achievements_list_widget->addItem("loading",
                         StringUtils::loadingDots(_("Fetching achievements")));
    }
}   // init

// -----------------------------------------------------------------------------

void BaseOnlineProfileAchievements::eventCallback(Widget* widget,
                                              const std::string& name,
                                              const int playerID)
{
    OnlineProfileBase::eventCallback( widget, name, playerID);
    if (name == m_achievements_list_widget->m_properties[GUIEngine::PROP_ID])
    {
        m_selected_achievement_index =
                                  m_achievements_list_widget->getSelectionID();

        int id;
        std::string achievement =
            m_achievements_list_widget->getSelectionInternalName();
        // Convert the achievement number into an integer, and if there
        // is no error, show the achievement (it can happen that the
        // string is "" if no achievement exists)
        if(StringUtils::fromString(achievement, id))
        {
            std::map<uint32_t, Achievement *> & all_achievements =
                PlayerManager::getCurrentPlayer()->getAchievementsStatus()
                                                    ->getAllAchievements();
            std::map<uint32_t, Achievement *>::const_iterator it;
            for (it = all_achievements.begin(); it != all_achievements.end(); ++it)
            {
                Achievement *a = it->second;
                if (a->getInfo()->getID() == (unsigned int) id)
                {
                    new AchievementProgressDialog(a);
                    break;
                }
            }
        }
    }
    if (name == "rankings")
    {
        new PlayerRankingsDialog(m_visiting_profile->getID(),
            m_visiting_profile->getUserName());
    }
}   // eventCallback

// ----------------------------------------------------------------------------
void BaseOnlineProfileAchievements::onColumnClicked(int column_id, bool sort_desc, bool sort_default)
{
    m_sort_column = column_id;
    m_sort_desc = sort_desc;
    m_sort_default = sort_default;

    if (!m_waiting_for_achievements)
    {
        displayResults();
    }
}   // onColumnClicked

// ----------------------------------------------------------------------------
/** Displays the achievements from a given profile.
 */
void BaseOnlineProfileAchievements::displayResults()
{
    m_achievements_list_widget->clear();
    m_waiting_for_achievements = false;

    if (!m_visiting_profile || m_visiting_profile->isCurrentUser())
    {
        // No need to wait for results, since they are local anyway
        std::map<uint32_t, Achievement *> & all_achievements =
            PlayerManager::getCurrentPlayer()->getAchievementsStatus()
            ->getAllAchievements();

        // We need to get a vector instead because we need to sort
        std::vector<Achievement *> all_achievements_list;

        std::map<uint32_t, Achievement *>::const_iterator it;
        for (it = all_achievements.begin(); it != all_achievements.end(); ++it)
        {
            all_achievements_list.push_back(it->second);
        }

        auto compAchievement = [=](Achievement *a, Achievement *b)
        {
            // Sort by name
            if (m_sort_column == 0)
                return a->getInfo()->getName().lower_ignore_case(b->getInfo()->getName());
            // Sort by goals
            else if (m_sort_column == 1)
                return goalSort(a, b);
            // Sort by progress
            else
                return progressSort(a, b);
        };

        if (m_sort_desc && !m_sort_default)
        {
            std::sort(all_achievements_list.rbegin(), all_achievements_list.rend(), compAchievement);
        }
        else if (!m_sort_default)
        {
            std::sort(all_achievements_list.begin(), all_achievements_list.end(), compAchievement);
        }

        std::vector<Achievement *>::iterator vit;
        for (vit = all_achievements_list.begin(); vit != all_achievements_list.end(); ++vit)
        {
            std::vector<ListWidget::ListCell> row;
            Achievement *a = *vit;
            if (a->getInfo()->isSecret() && !a->isAchieved())
                continue;
            ListWidget::ListCell title(a->getInfo()->getName(), -1, 2);
            ListWidget::ListCell goals(a->getGoalProgressAsString(), -1, 1);
            ListWidget::ListCell progress(a->getProgressAsString(), -1, 1);
            row.push_back(title);
            row.push_back(goals);
            row.push_back(progress);
            const std::string id = StringUtils::toString(a->getInfo()->getID());
            m_achievements_list_widget->addItem(id, row);
            if (a->isAchieved())
                m_achievements_list_widget->emphasisItem(id);
        }
    }
    else
    {
        OnlineProfile::IDList a = m_visiting_profile->getAchievements();

        auto compAchievementInfo = [](int a, int b) {
            AchievementInfo *a1 = AchievementsManager::get()->getAchievementInfo(a);
            AchievementInfo *a2 = AchievementsManager::get()->getAchievementInfo(b);
            return a1->getName().lower_ignore_case(a2->getName());
        };

        // Because only the name column is visible when viewing other player's achievements, col does not matter
        if (m_sort_desc && !m_sort_default)
        {
            std::sort(a.rbegin(), a.rend(), compAchievementInfo);
        }
        else if (!m_sort_default)
        {
            std::sort(a.begin(), a.end(), compAchievementInfo);
        }

        for (unsigned int i = 0; i < a.size(); i++)
        {
            AchievementInfo *info =
                AchievementsManager::get()->getAchievementInfo(a[i]);
            m_achievements_list_widget->addItem(StringUtils::toString(info->getID()),
                info->getName());
        }
    }
}   // displayResults

// ----------------------------------------------------------------------------
/** True if a's goal progression is <= to b's.
 *  If they are equal, goalSort(a,b) != goalSort(b,a),
 ù  as the bool can't handle the 3 values required to avoid this
 */
bool BaseOnlineProfileAchievements::goalSort(Achievement *a, Achievement *b)
{
    float goals_a = ((float) a->getFullfiledGoals()) / a->getInfo()->getGoalCount();
    float goals_b = ((float) b->getFullfiledGoals()) / b->getInfo()->getGoalCount();
    if (goals_a == goals_b)
        return (a->getInfo()->getGoalCount() < b->getInfo()->getGoalCount());
    else
        return (goals_a < goals_b);
} // goalSort

// ----------------------------------------------------------------------------
/** True if a's single-goal progress is <= to b's.
 */
bool BaseOnlineProfileAchievements::progressSort(Achievement *a, Achievement *b)
{
    if (a->getInfo()->getGoalCount() >= 2)
        return true;
    else if (b->getInfo()->getGoalCount() >= 2)
        return false;

    float progress_a = ((float) a->getProgress()) / a->getInfo()->getProgressTarget();
    float progress_b = ((float) b->getProgress()) / b->getInfo()->getProgressTarget();
    if (progress_a == progress_b)
        return (a->getInfo()->getProgressTarget() < b->getInfo()->getProgressTarget());
    else
        return (progress_a < progress_b);
} // progressSort

// ----------------------------------------------------------------------------
/** Called every frame. It will check if results from an achievement request
 *  have been received, and if so, display them.
 */
void BaseOnlineProfileAchievements::onUpdate(float delta)
{
    if (!m_waiting_for_achievements) return;

    if (!m_visiting_profile->hasFetchedAchievements())
    {
        // This will display an increasing number of dots while waiting.
        m_achievements_list_widget->renameItem("loading",
                       StringUtils::loadingDots(_("Fetching achievements")));
        return;
    }

    // Now reesults are available, display them.
    displayResults();
}   // onUpdate
