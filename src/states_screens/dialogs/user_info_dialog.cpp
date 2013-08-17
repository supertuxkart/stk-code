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

#include "states_screens/dialogs/user_info_dialog.hpp"

#include <IGUIEnvironment.h>

#include "audio/sfx_manager.hpp"
#include "config/player.hpp"
#include "guiengine/engine.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/online_profile_overview.hpp"
#include "utils/translation.hpp"
#include "utils/string_utils.hpp"
#include "network/protocol_manager.hpp"
#include "network/protocols/connect_to_server.hpp"
#include "online/messages.hpp"
#include "states_screens/dialogs/registration_dialog.hpp"
#include "states_screens/networking_lobby.hpp"



using namespace GUIEngine;
using namespace irr;
using namespace irr::gui;
using namespace Online;

// -----------------------------------------------------------------------------

UserInfoDialog::UserInfoDialog(uint32_t visiting_id)
        : ModalDialog(0.8f,0.8f), m_visiting_id(visiting_id)
{
    m_self_destroy = false;
    m_enter_profile = false;

    loadFromFile("online/user_info_dialog.stkgui");
    m_profile = ProfileManager::get()->getProfileByID(visiting_id);
    m_name_widget = getWidget<LabelWidget>("name");
    assert(m_name_widget != NULL);
    //const Server * server = ProfileManager::get()->getServerByID(m_visiting_id);
    m_name_widget->setText(m_profile->getUserName(),false);
    m_info_widget = getWidget<LabelWidget>("info");
    assert(m_info_widget != NULL);
    m_options_widget = getWidget<RibbonWidget>("options");
    assert(m_options_widget != NULL);
    m_enter_widget = getWidget<IconButtonWidget>("enter");
    assert(m_enter_widget != NULL);
    m_cancel_widget = getWidget<IconButtonWidget>("cancel");
    assert(m_cancel_widget != NULL);
    m_options_widget->setFocusForPlayer(PLAYER_ID_GAME_MASTER);

}

// -----------------------------------------------------------------------------
UserInfoDialog::~UserInfoDialog()
{
}

// -----------------------------------------------------------------------------
GUIEngine::EventPropagation UserInfoDialog::processEvent(const std::string& eventSource)
{

    if (eventSource == m_options_widget->m_properties[PROP_ID])
    {
        const std::string& selection = m_options_widget->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        if (selection == m_cancel_widget->m_properties[PROP_ID])
        {
            m_self_destroy = true;
            return GUIEngine::EVENT_BLOCK;
        }
        else if(selection == m_enter_widget->m_properties[PROP_ID])
        {
            ProfileManager::get()->setVisiting(m_profile->getID());
            m_enter_profile = true;
            return GUIEngine::EVENT_BLOCK;
        }
    }
    return GUIEngine::EVENT_LET;
}

// -----------------------------------------------------------------------------

void UserInfoDialog::onEnterPressedInternal()
{

    //If enter was pressed while none of the buttons was focused interpret as join event
    const int playerID = PLAYER_ID_GAME_MASTER;
    if (GUIEngine::isFocusedForPlayer(m_options_widget, playerID))
        return;
    m_self_destroy = true;
}

// -----------------------------------------------------------------------------

bool UserInfoDialog::onEscapePressed()
{
    if (m_cancel_widget->isActivated())
        m_self_destroy = true;
    return false;
}

// -----------------------------------------------------------------------------

void UserInfoDialog::onUpdate(float dt)
{


    //If we want to open the registration dialog, we need to close this one first
    m_enter_profile && (m_self_destroy = true);

    // It's unsafe to delete from inside the event handler so we do it here
    if (m_self_destroy)
    {
        ModalDialog::dismiss();
        if (m_enter_profile)
            StateManager::get()->replaceTopMostScreen(OnlineProfileOverview::getInstance());
        return;
    }
}
