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

#include "states_screens/dialogs/general_debug_dialog.hpp"

#include "guiengine/engine.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/string_utils.hpp"

#include <IGUIEnvironment.h>

using namespace GUIEngine;
using namespace irr::core;

// -----------------------------------------------------------------------------
GeneralDebugDialog::GeneralDebugDialog(const wchar_t* title, Callback cb) :
    ModalDialog(0.95f, 0.4f, GUIEngine::MODAL_DIALOG_LOCATION_BOTTOM),
    m_callback(cb)
{
    m_fade_background = false;
    loadFromFile("general_debug_dialog.stkgui");

    TextBoxWidget* text_field = getWidget<TextBoxWidget>("textfield");
    assert(text_field != NULL);
    text_field->setFocusForPlayer(PLAYER_ID_GAME_MASTER);

    LabelWidget* label = getWidget<LabelWidget>("title");
    assert(label != NULL);
    label->setText(title, false/*expandAsNeeded*/);
}   // GeneralDebugDialog

// -----------------------------------------------------------------------------
GeneralDebugDialog::~GeneralDebugDialog()
{
    TextBoxWidget* text_field = getWidget<TextBoxWidget>("textfield");
    text_field->getIrrlichtElement()->remove();
    text_field->clearListeners();
}   // ~GeneralDebugDialog

// -----------------------------------------------------------------------------
GUIEngine::EventPropagation GeneralDebugDialog::processEvent(const std::string& eventSource)
{
    if (eventSource == "close")
    {
        dismiss();
        return GUIEngine::EVENT_BLOCK;
    }
    else if (eventSource == "ok")
    {
        run();
        return GUIEngine::EVENT_BLOCK;
    }
    return GUIEngine::EVENT_LET;
}   // processEvent

// -----------------------------------------------------------------------------
void GeneralDebugDialog::run()
{
    TextBoxWidget* text_field = getWidget<TextBoxWidget>("textfield");
    std::string text = StringUtils::wideToUtf8(text_field->getText());
    m_callback(text);
    text_field->setText(L"");

}   // run
