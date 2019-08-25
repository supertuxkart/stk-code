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

#include "states_screens/dialogs/message_dialog.hpp"

#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "modes/world.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/translation.hpp"

using namespace GUIEngine;

// ----------------------------------------------------------------------------
/** Complete constructor, which allows setting of listener, type etc.
 *  \param msg The text to be shown in the dialog.
 *  \param type The type of dialog (OK, confirm, ok/cancel, ...).
 *  \param listener An optional listener object.
 *  \param own_listener If true the dialog will free the listener.
 *  \param from_queue If the object is placed into the DialogQueue. If so,
 *         loadFromFile() is not called (it will be called when the dialog
 *         is finally being removed from the queue and shown).
 */
MessageDialog::MessageDialog(const irr::core::stringw &msg,
                             MessageDialogType type,
                             IConfirmDialogListener* listener,
                             bool own_listener, bool from_queue,
                             float width, float height)
             : ModalDialog(width, height)
{
    m_msg          = msg;
    m_type         = type;
    m_listener     = listener;
    m_own_listener = own_listener;
    doInit(from_queue);
}   // MessageDialog(stringw, type, listener, own_listener)

// ----------------------------------------------------------------------------
/** Simple constructor for a notification message (i.e. only OK button shown).
 *  \param msg The message to show.
 *  \param from_queue If the object is placed into the DialogQueue. If so,
 *         loadFromFile() is not called (it will be called when the dialog
 *         is finally being removed from the queue and shown).
 */
MessageDialog::MessageDialog(const irr::core::stringw &msg, bool from_queue)
             : ModalDialog(0.6f, 0.6f)
{
    m_msg          = msg;
    m_type         = MessageDialog::MESSAGE_DIALOG_OK;
    m_listener     = NULL;
    m_own_listener = false;
    if (!from_queue) doInit(false);
}   // MessageDialog(stringw)

// ----------------------------------------------------------------------------
/** Called from the DialogQueue, used to load the actual xml file and init
 *  the dialog.
 */
void MessageDialog::load()
{
    doInit(false);
}

// ----------------------------------------------------------------------------
/** If necessary schedules a pause, and loads the xml file if necessary.
 *  \param from_queue If the dialog is queued, do not load the xml file,
 *         this will be done later in the case of a queued dialog.
 */
void MessageDialog::doInit(bool from_queue)
{
    if (StateManager::get()->getGameState() == GUIEngine::GAME)
    {
        World::getWorld()->schedulePause(World::IN_GAME_MENU_PHASE);
    }

    if (!from_queue)
        loadFromFile("confirm_dialog.stkgui");
}   // doInit

// ----------------------------------------------------------------------------

MessageDialog::~MessageDialog()
{
    if (m_own_listener) delete m_listener;
    m_listener = NULL;

    if (StateManager::get()->getGameState() == GUIEngine::GAME)
    {
        World::getWorld()->scheduleUnpause();
    }
}   // ~MessageDialog

// ----------------------------------------------------------------------------
void MessageDialog::loadedFromFile()
{
    LabelWidget* message = getWidget<LabelWidget>("title");
    message->setText( m_msg, false );
    RibbonWidget* ribbon = getWidget<RibbonWidget>("buttons");
    ribbon->setFocusForPlayer(PLAYER_ID_GAME_MASTER);

    // If the dialog is a simple 'OK' dialog, then hide the "Yes" button and
    // change "Cancel" to "OK"
    if (m_type == MessageDialog::MESSAGE_DIALOG_OK)
    {
        IconButtonWidget* yesbtn = getWidget<IconButtonWidget>("cancel");
        yesbtn->setVisible(false);

        IconButtonWidget* cancelbtn = getWidget<IconButtonWidget>("confirm");
        cancelbtn->setText(_("OK"));
        cancelbtn->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    }
    else if (m_type == MessageDialog::MESSAGE_DIALOG_YESNO)
    {
        IconButtonWidget* cancelbtn = getWidget<IconButtonWidget>("cancel");
        cancelbtn->setText(_("No"));
    }
    else if (m_type == MessageDialog::MESSAGE_DIALOG_OK_CANCEL)
    {
        // In case of a OK_CANCEL dialog, change the text from 'Yes' to 'Ok'
        IconButtonWidget* yesbtn = getWidget<IconButtonWidget>("confirm");
        yesbtn->setText(_("OK"));
    }
}

// ----------------------------------------------------------------------------

void MessageDialog::onEnterPressedInternal()
{
}

// ----------------------------------------------------------------------------

GUIEngine::EventPropagation MessageDialog::processEvent(const std::string& eventSource)
{
    RibbonWidget* ribbon = getWidget<RibbonWidget>(eventSource.c_str());
    
    if (ribbon->getSelectionIDString(PLAYER_ID_GAME_MASTER) == "cancel")
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
    else if (ribbon->getSelectionIDString(PLAYER_ID_GAME_MASTER) == "confirm")
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

// ----------------------------------------------------------------------------

void MessageDialog::onUpdate(float dt)
{
    if (m_listener != NULL) m_listener->onDialogUpdate(dt);
}
