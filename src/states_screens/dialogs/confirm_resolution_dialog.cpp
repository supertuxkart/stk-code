//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015 Marianne Gagnon
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

#include "states_screens/dialogs/confirm_resolution_dialog.hpp"

#include "graphics/irr_driver.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

using namespace GUIEngine;
using namespace irr::gui;
using namespace irr::core;

// ------------------------------------------------------------------------------------------------------

ConfirmResolutionDialog::ConfirmResolutionDialog() : ModalDialog(0.7f, 0.7f)
{
    loadFromFile("confirm_resolution_dialog.stkgui");
    m_remaining_time = 10.99f;

    updateMessage();
}

// ------------------------------------------------------------------------------------------------------

void ConfirmResolutionDialog::onEnterPressedInternal()
{
}

// ------------------------------------------------------------------------------------------------------

void ConfirmResolutionDialog::onUpdate(float dt)
{
    const int previous_number = (int)m_remaining_time;
    m_remaining_time -= dt;

    if (m_remaining_time < 0)
    {
        ModalDialog::dismiss();
        irr_driver->cancelResChange();
    }
    else if ((int)m_remaining_time != previous_number)
    {
        updateMessage();
    }

}

// ----------------------------------------------------------------------------

bool ConfirmResolutionDialog::onEscapePressed()
{
    ModalDialog::dismiss();
    irr_driver->cancelResChange();
    return true;
}   // escapePressed

// ------------------------------------------------------------------------------------------------------

void ConfirmResolutionDialog::updateMessage()
{
    //I18N: In the 'confirm resolution' dialog, that's shown when switching resoluton

    stringw msg = _P("Confirm resolution within %i second",
        "Confirm resolution within %i seconds",
        (int)m_remaining_time);

    LabelWidget* countdown_message = getWidget<LabelWidget>("title");
    countdown_message->setText( msg.c_str(), false );
}

// ------------------------------------------------------------------------------------------------------

GUIEngine::EventPropagation ConfirmResolutionDialog::processEvent(const std::string& eventSource)
{

    if (eventSource == "cancel")
    {
        ModalDialog::dismiss();

        irr_driver->cancelResChange();
        return GUIEngine::EVENT_BLOCK;
    }
    else if (eventSource == "accept")
    {
        ModalDialog::dismiss();

        return GUIEngine::EVENT_BLOCK;
    }

    return GUIEngine::EVENT_LET;
}
