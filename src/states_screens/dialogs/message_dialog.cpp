//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2013 Marianne Gagnon
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

#include "states_screens/dialogs/message_dialog.hpp"

#include "guiengine/engine.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "modes/world.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/translation.hpp"

using namespace GUIEngine;

// ------------------------------------------------------------------------------------------------------

MessageDialog::MessageDialog(const irr::core::stringw &msg, MessageDialogType type,
                             IConfirmDialogListener* listener, bool own_listener) :
    ModalDialog(0.6f, 0.6f)
{
    m_msg = msg;
    doInit(type, listener, own_listener);
}   // MessageDialog(stringw, type, listener, own_listener)

// ------------------------------------------------------------------------------------------------------

MessageDialog::MessageDialog(const irr::core::stringw &msg, bool from_queue) :
    ModalDialog(0.6f, 0.6f)
{
    m_msg = msg;
    if(!from_queue) load();
}   // MessageDialog(stringw)

// ------------------------------------------------------------------------------------------------------

void MessageDialog::load()
{
    doInit(MessageDialog::MESSAGE_DIALOG_OK, NULL, false);
}

// ------------------------------------------------------------------------------------------------------

MessageDialog::~MessageDialog()
{
    if (m_own_listener) delete m_listener; m_listener = NULL;

    if (StateManager::get()->getGameState() == GUIEngine::GAME)
    {
        World::getWorld()->scheduleUnpause();
    }
}   // ~MessageDialog

// ------------------------------------------------------------------------------------------------------

void MessageDialog::doInit(MessageDialogType type,
                           IConfirmDialogListener* listener, bool own_listener)
{
    if (StateManager::get()->getGameState() == GUIEngine::GAME)
    {
        World::getWorld()->schedulePause(World::IN_GAME_MENU_PHASE);
    }


    loadFromFile("confirm_dialog.stkgui");

    m_listener = listener;
    m_own_listener = own_listener;

    LabelWidget* message = getWidget<LabelWidget>("title");
    message->setText( m_msg.c_str(), false );

    // If the dialog is a simple 'OK' dialog, then hide the "Yes" button and
    // change "Cancel" to "OK"
    if (type == MessageDialog::MESSAGE_DIALOG_OK)
    {
        ButtonWidget* yesbtn = getWidget<ButtonWidget>("confirm");
        yesbtn->setVisible(false);

        ButtonWidget* cancelbtn = getWidget<ButtonWidget>("cancel");
        cancelbtn->setText(_("OK"));
        cancelbtn->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    }
    else if (type == MessageDialog::MESSAGE_DIALOG_YESNO)
    {
        ButtonWidget* yesbtn = getWidget<ButtonWidget>("confirm");

        ButtonWidget* cancelbtn = getWidget<ButtonWidget>("cancel");
        cancelbtn->setText(_("No"));

    }
    else if (type == MessageDialog::MESSAGE_DIALOG_OK_CANCEL)
    {
        // In case of a OK_CANCEL dialog, change the text from 'Yes' to 'Ok'
        ButtonWidget* yesbtn = getWidget<ButtonWidget>("confirm");
        yesbtn->setText(_("OK"));
    }
}


// ------------------------------------------------------------------------------------------------------

void MessageDialog::onEnterPressedInternal()
{
}

// ------------------------------------------------------------------------------------------------------

GUIEngine::EventPropagation MessageDialog::processEvent(const std::string& eventSource)
{

    if (eventSource == "cancel")
    {
        if (m_listener == NULL)
        {
            ModalDialog::dismiss();
        }
        else
        {
            m_listener->onCancel();
        }

        return GUIEngine::EVENT_BLOCK;
    }
    else if (eventSource == "confirm")
    {
        if (m_listener == NULL)
        {
            ModalDialog::dismiss();
        }
        else
        {
            m_listener->onConfirm();
        }

        return GUIEngine::EVENT_BLOCK;
    }

    return GUIEngine::EVENT_LET;
}

// ------------------------------------------------------------------------------------------------------

void MessageDialog::onUpdate(float dt)
{
    if (m_listener != NULL) m_listener->onDialogUpdate(dt);
}
