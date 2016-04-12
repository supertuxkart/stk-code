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

#include "states_screens/online_profile_achievements.hpp"

#include "achievements/achievement_info.hpp"
#include "achievements/achievements_manager.hpp"
#include "config/player_manager.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widget.hpp"
#include "online/online_profile.hpp"
#include "states_screens/dialogs/message_dialog.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/dialogs/user_info_dialog.hpp"
#include "utils/translation.hpp"

#include <IGUIButton.h>

#include <iostream>
#include <sstream>

using namespace GUIEngine;
using namespace irr::core;
using namespace irr::gui;
using namespace Online;

DEFINE_SCREEN_SINGLETON( OnlineProfileAchievements    );
DEFINE_SCREEN_SINGLETON( TabOnlineProfileAchievements );

// -----------------------------------------------------------------------------
/** Constructor.
 */
BaseOnlineProfileAchievements::BaseOnlineProfileAchievements(const std::string &name)
                             : OnlineProfileBase(name)
{
    m_selected_achievement_index = -1;
}   // BaseOnlineProfileAchievements

// -----------------------------------------------------------------------------
/** Callback when the xml file was loaded.
 */
void BaseOnlineProfileAchievements::loadedFromFile()
{
    OnlineProfileBase::loadedFromFile();
    m_achievements_list_widget = getWidget<ListWidget>("achievements_list");
    assert(m_achievements_list_widget != NULL);

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
        // No need to wait for results, since they are local anyway
        m_waiting_for_achievements = false;
        m_achievements_list_widget->clear();
        const std::map<uint32_t, Achievement *> & all_achievements =
            PlayerManager::getCurrentPlayer()->getAchievementsStatus()
                                                    ->getAllAchievements();
        std::map<uint32_t, Achievement *>::const_iterator it;
        for (it = all_achievements.begin(); it != all_achievements.end(); ++it)
        {
            std::vector<ListWidget::ListCell> row;
            const Achievement *a = it->second;
            if(a->getInfo()->isSecret() && !a->isAchieved())
                continue;
            ListWidget::ListCell title(translations->fribidize(a->getInfo()->getName()), -1, 2);
            ListWidget::ListCell progress(a->getProgressAsString(), -1, 1);
            row.push_back(title);
            row.push_back(progress);
            const std::string id = StringUtils::toString(a->getInfo()->getID());
            m_achievements_list_widget->addItem(id, row);
            if (a->isAchieved())
                m_achievements_list_widget->markItemBlue(id);
        }
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
            new MessageDialog(AchievementsManager::get()
                                   ->getAchievementInfo(id)->getDescription());
    }
}   // eventCallback

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
    m_achievements_list_widget->clear();
    const OnlineProfile::IDList &a = m_visiting_profile->getAchievements();
    for (unsigned int i = 0; i < a.size(); i++)
    {
        AchievementInfo *info =
                          AchievementsManager::get()->getAchievementInfo(a[i]);
        m_achievements_list_widget->addItem(StringUtils::toString(info->getID()),
                                            info->getName()                   );
    }
    m_waiting_for_achievements = false;

}   // onUpdate
