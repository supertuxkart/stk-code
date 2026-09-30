//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 Marianne Gagnon
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

#include "states_screens/dialogs/icon_selection_dialog.hpp"

#include "config/player_manager.hpp"
#include "config/player_profile.hpp"
#include "config/user_config.hpp"
#include "karts/kart_model.hpp"
#include "karts/kart_properties.hpp"
#include "karts/kart_properties_manager.hpp"
#include "graphics/irr_driver.hpp"
#include <ge_render_info.hpp>
#include "guiengine/engine.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/model_view_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

using namespace GUIEngine;

// ----------------------------------------------------------------------------
IconSelectionDialog::IconSelectionDialog(PlayerProfile* pp)
                     : ModalDialog(0.75f, 0.75f, MODAL_DIALOG_LOCATION_CENTER)
{
    loadFromFile("icon_selection_dialog.stkgui");
    m_player_profile = pp;

    m_buttons_widget = getWidget<RibbonWidget>("buttons");

}   // IconSelectionDialog

// ----------------------------------------------------------------------------
IconSelectionDialog::~IconSelectionDialog()
{
    PlayerManager::get()->save();
}   // ~IconSelectionDialog

// ----------------------------------------------------------------------------
void IconSelectionDialog::beforeAddingWidgets()
{

}   // beforeAddingWidgets

// ----------------------------------------------------------------------------
GUIEngine::EventPropagation
            IconSelectionDialog::processEvent(const std::string& eventSource)
{

    if (eventSource == "buttons")
    {
        const std::string& selection = m_buttons_widget->
                                    getSelectionIDString(PLAYER_ID_GAME_MASTER);

        if (selection == "apply")
        {
            ModalDialog::dismiss();
            return GUIEngine::EVENT_BLOCK;
        }
        else if (selection == "cancel")
        {
            ModalDialog::dismiss();
            return GUIEngine::EVENT_BLOCK;
        }
    }
    return GUIEngine::EVENT_LET;
}   // processEvent
