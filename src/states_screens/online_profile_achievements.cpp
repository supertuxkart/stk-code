//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 Glenn De Jonghe
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

#include "achievements/achievements_manager.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widget.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/dialogs/user_info_dialog.hpp"
#include "utils/translation.hpp"
#include "online/messages.hpp"

#include <IGUIButton.h>

#include <iostream>
#include <sstream>

using namespace GUIEngine;
using namespace irr::core;
using namespace irr::gui;
using namespace Online;

DEFINE_SCREEN_SINGLETON( OnlineProfileAchievements );

// -----------------------------------------------------------------------------

OnlineProfileAchievements::OnlineProfileAchievements() : OnlineProfileBase("online/profile_achievements.stkgui")
{
    m_selected_achievement_index = -1;
}   // OnlineProfileAchievements

// -----------------------------------------------------------------------------

void OnlineProfileAchievements::loadedFromFile()
{
    OnlineProfileBase::loadedFromFile();
    m_achievements_list_widget = getWidget<GUIEngine::ListWidget>("achievements_list");
    assert(m_achievements_list_widget != NULL);

}   // loadedFromFile

// ----------------------------------------------------------------------------

void OnlineProfileAchievements::beforeAddingWidget()
{
    OnlineProfileBase::beforeAddingWidget();
    m_achievements_list_widget->clearColumns();
    m_achievements_list_widget->addColumn( _("Name"), 2 );
    if(m_visiting_profile->isCurrentUser())
    {
        m_achievements_list_widget->addColumn( _("Progress"), 1 );
    }
}

// -----------------------------------------------------------------------------

void OnlineProfileAchievements::init()
{
    OnlineProfileBase::init();
    m_profile_tabs->select( m_achievements_tab->m_properties[PROP_ID], PLAYER_ID_GAME_MASTER );
    assert(m_visiting_profile != NULL);
    if(m_visiting_profile->isCurrentUser())
    {
        m_waiting_for_achievements = false;
        m_achievements_list_widget->clear();
        const std::map<uint32_t, Achievement *> & all_achievements = AchievementsManager::get()->getActive()->getAllAchievements();
        std::map<uint32_t, Achievement *>::const_iterator it;
        for ( it = all_achievements.begin(); it != all_achievements.end(); ++it ) {
            PtrVector<GUIEngine::ListWidget::ListCell> * row = new PtrVector<GUIEngine::ListWidget::ListCell>;
            row->push_back(new GUIEngine::ListWidget::ListCell(it->second->getInfo()->getTitle(),-1,2));
            row->push_back(new GUIEngine::ListWidget::ListCell(it->second->getProgressAsString(),-1,1, true));
            m_achievements_list_widget->addItem(StringUtils::toString(it->second->getInfo()->getID()), row);
        }
    }
    else
    {
        m_waiting_for_achievements = true;
        m_visiting_profile->fetchAchievements();
        m_achievements_list_widget->clear();
        m_achievements_list_widget->addItem("loading", Messages::fetchingAchievements());
    }
}   // init
// -----------------------------------------------------------------------------

void OnlineProfileAchievements::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
    OnlineProfileBase::eventCallback( widget, name, playerID);
    if (name == m_achievements_list_widget->m_properties[GUIEngine::PROP_ID])
    {
        m_selected_achievement_index = m_achievements_list_widget->getSelectionID();

        //new achievementInfoDialog(atoi(m_achievements_list_widget->getSelectionInternalName().c_str())); //FIXME dialog
    }
}   // eventCallback

// ----------------------------------------------------------------------------
void OnlineProfileAchievements::onUpdate(float delta,  irr::video::IVideoDriver* driver)
{
    if(m_waiting_for_achievements)
    {
        if(m_visiting_profile->isReady())
        {
            m_achievements_list_widget->clear();
            for(unsigned int i = 0; i < m_visiting_profile->getAchievements().size(); i++)
            {
                AchievementInfo * info = AchievementsManager::get()->getAchievementInfo(m_visiting_profile->getAchievements()[i]);
                m_achievements_list_widget->addItem(StringUtils::toString(info->getID()), info->getTitle());
            }
            m_waiting_for_achievements = false;
        }
        else
        {
                m_achievements_list_widget->renameItem("loading", Messages::fetchingFriends());
        }
    }
}
