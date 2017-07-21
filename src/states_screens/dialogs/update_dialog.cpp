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

#include "states_screens/dialogs/update_dialog.hpp"

#include "config/user_config.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/widgets.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/translation.hpp"
#include "utils/string_utils.hpp"
#include "online/link_helper.hpp"

#include <IGUIEnvironment.h>

using namespace GUIEngine;
using namespace irr;
using namespace irr::gui;

// -----------------------------------------------------------------------------

UpdateDialog::UpdateDialog() :
        ModalDialog(0.6f,0.6f)
{
    loadFromFile("online/update_dialog.stkgui");
}

// -----------------------------------------------------------------------------

UpdateDialog::~UpdateDialog()
{
}

// -----------------------------------------------------------------------------
/** Process input events.
 *  \event_source name of the widget that triggered the event.
 */
EventPropagation UpdateDialog::processEvent(const std::string& event_source)
{
    if (event_source == "options")
    {
        RibbonWidget *rib = getWidget<RibbonWidget>("options");
        std::string s = rib->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        // Close dialog
        ModalDialog::dismiss();

        // Process response, this is done after closing because LinkHelper::OpenURL can be a bit slow,
        // and waiting for it before closing the dialog makes STK seem slow/stuck
        if (s == "yes")
        {
            Online::LinkHelper::OpenURL("https://SuperTuxKart.net/Download"); // Open downloads page
            StateManager::get()->onStackEmptied(); // Close STK (user is installing new version)
        }
        else if (s == "never")
        {
            UserConfigParams::m_update_popup = false;
        }

        return EVENT_BLOCK;
    }
    return EVENT_LET;
}   // processEvent
