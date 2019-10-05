//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2016 SuperTuxKart-Team
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

#include "states_screens/dialogs/general_text_field_dialog.hpp"

#include "guiengine/engine.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/string_utils.hpp"

#include <IGUIEnvironment.h>

using namespace GUIEngine;
using namespace irr::core;

// -----------------------------------------------------------------------------
GeneralTextFieldDialog::GeneralTextFieldDialog(const core::stringw& title,
                                               DismissCallback dm_cb,
                                               ValidationCallback val_cb)
                  : ModalDialog(0.95f, 0.5f,
                    GUIEngine::MODAL_DIALOG_LOCATION_BOTTOM),
                    m_dm_cb(dm_cb), m_val_cb(val_cb), m_self_destroy(false)
{
    loadFromFile("general_text_field_dialog.stkgui");

    m_text_field = getWidget<TextBoxWidget>("textfield");
    assert(m_text_field != NULL);
    m_text_field->setFocusForPlayer(PLAYER_ID_GAME_MASTER);

    m_title = getWidget<LabelWidget>("title");
    assert(m_title != NULL);
    m_title->setText(title, false/*expandAsNeeded*/);
    assert(m_dm_cb != NULL);
    assert(m_val_cb != NULL);
}   // GeneralTextFieldDialog

// -----------------------------------------------------------------------------
GeneralTextFieldDialog::~GeneralTextFieldDialog()
{
    m_text_field->getIrrlichtElement()->remove();
    m_text_field->clearListeners();
}   // ~GeneralTextFieldDialog

// -----------------------------------------------------------------------------
GUIEngine::EventPropagation GeneralTextFieldDialog::processEvent(const std::string& eventSource)
{ 
    GUIEngine::RibbonWidget* buttons_ribbon =
        getWidget<GUIEngine::RibbonWidget>("buttons");
    
    if(eventSource == "buttons")
    {
        const std::string& button =
	    buttons_ribbon->getSelectionIDString(PLAYER_ID_GAME_MASTER);

	if (button == "cancel")
        {
            dismiss();
            return GUIEngine::EVENT_BLOCK;
        }
        else if (button == "ok")
        {
	    // If validation callback return true, dismiss the dialog
            if (!m_self_destroy && m_val_cb(m_title, m_text_field))
                m_self_destroy = true;
            return GUIEngine::EVENT_BLOCK;
        }
    }
    return GUIEngine::EVENT_LET;
}   // processEvent

// -----------------------------------------------------------------------------
void GeneralTextFieldDialog::onEnterPressedInternal()
{
    // Cancel button pressed
    IconButtonWidget* cancel_button = getWidget<IconButtonWidget>("cancel");
    if (GUIEngine::isFocusedForPlayer(cancel_button, PLAYER_ID_GAME_MASTER))
    {
        std::string fake_event = "cancel";
        processEvent(fake_event);
        return;
    }

    if (!m_self_destroy && m_val_cb(m_title, m_text_field))
        m_self_destroy = true;

}   // onEnterPressedInternal

// -----------------------------------------------------------------------------
void GeneralTextFieldDialog::onUpdate(float dt)
{
    // It's unsafe to delete from inside the event handler so we do it here
    if (m_self_destroy)
    {
        stringw name = m_text_field->getText().trim();

        // irrLicht is too stupid to remove focus from deleted widgets
        // so do it by hand
        GUIEngine::getGUIEnv()
            ->removeFocus(m_text_field->getIrrlichtElement());
        GUIEngine::getGUIEnv()->removeFocus(m_irrlicht_window);

        // We will destroy the dialog before notifying the callback is run,
        // but in order not to crash we must make a local copy of the callback
        // otherwise we will crash
        DismissCallback dm_cb = m_dm_cb;
        ModalDialog::dismiss();
        dm_cb(name);
    }
}   // onUpdate
