//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 Glenn De Jonghe
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

#include "states_screens/online_profile_settings.hpp"

#include "guiengine/engine.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widget.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/dialogs/change_password_dialog.hpp"
#include "utils/translation.hpp"

#include <IGUIButton.h>

#include <iostream>
#include <sstream>

using namespace GUIEngine;
using namespace irr::core;
using namespace irr::gui;
using namespace Online;

DEFINE_SCREEN_SINGLETON( OnlineProfileSettings );

// -----------------------------------------------------------------------------

OnlineProfileSettings::OnlineProfileSettings() : OnlineProfileBase("online/profile_settings.stkgui")
{
}   // OnlineProfileSettings

// -----------------------------------------------------------------------------

void OnlineProfileSettings::loadedFromFile()
{
    OnlineProfileBase::loadedFromFile();
    m_change_password_button = this->getWidget<ButtonWidget>("change_password_button");
    assert(m_change_password_button != NULL);
}   // loadedFromFile

// -----------------------------------------------------------------------------

void OnlineProfileSettings::init()
{
    OnlineProfileBase::init();
    m_profile_tabs->select( m_settings_tab->m_properties[PROP_ID], PLAYER_ID_GAME_MASTER );
    m_settings_tab->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
}   // init

// -----------------------------------------------------------------------------

void OnlineProfileSettings::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
    OnlineProfileBase::eventCallback( widget, name, playerID);
    if (name == m_change_password_button->m_properties[GUIEngine::PROP_ID])
   {
       new ChangePasswordDialog();
   }
}   // eventCallback

