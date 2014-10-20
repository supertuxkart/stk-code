//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014 Marc Coll
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

#include "states_screens/dialogs/enter_gp_name_dialog.hpp"

#include "audio/sfx_manager.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"
#include "race/grand_prix_manager.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/translation.hpp"

#include <IGUIEnvironment.h>


using namespace GUIEngine;
using namespace irr::core;

// -----------------------------------------------------------------------------
EnterGPNameDialog::EnterGPNameDialog(INewGPListener* listener,
    const float w, const float h)
    : ModalDialog(w, h), m_listener(listener), m_self_destroy(false)
{
    assert(listener != NULL);
    loadFromFile("enter_gp_name_dialog.stkgui");

    TextBoxWidget* textCtrl = getWidget<TextBoxWidget>("textfield");
    assert(textCtrl != NULL);
    textCtrl->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
}

// -----------------------------------------------------------------------------
EnterGPNameDialog::~EnterGPNameDialog()
{
    // FIXME: what is this code for?
    TextBoxWidget* textCtrl = getWidget<TextBoxWidget>("textfield");
    textCtrl->getIrrlichtElement()->remove();
    textCtrl->clearListeners();
}

// -----------------------------------------------------------------------------
GUIEngine::EventPropagation EnterGPNameDialog::processEvent(const std::string& eventSource)
{
    if (eventSource == "cancel")
    {
        dismiss();
        return GUIEngine::EVENT_BLOCK;
    }
    return GUIEngine::EVENT_LET;
}

// -----------------------------------------------------------------------------
void EnterGPNameDialog::onEnterPressedInternal()
{
    //Cancel button pressed
    ButtonWidget* cancelButton = getWidget<ButtonWidget>("cancel");
    if (GUIEngine::isFocusedForPlayer(cancelButton, PLAYER_ID_GAME_MASTER))
    {
        std::string fakeEvent = "cancel";
        processEvent(fakeEvent);
        return;
    }

    //Otherwise, see if we can accept the new name
    TextBoxWidget* textCtrl = getWidget<TextBoxWidget>("textfield");
    assert(textCtrl != NULL);
    LabelWidget* label = getWidget<LabelWidget>("title");
    assert(label != NULL);

    stringw name = textCtrl->getText().trim();
    if (name.size() == 0)
    {
        label->setText(_("Name is empty."), false);
        SFXManager::get()->quickSound("anvil");
    }
    else if (grand_prix_manager->existsName(name) || name == "Random Grand Prix")
    {
        // check for duplicate names
        label->setText(_("Another grand prix with this name already exists."), false);
        SFXManager::get()->quickSound("anvil");
    }
    else if (name.size() > 30)
    {
        label->setText(_("Name is too long."), false);
        SFXManager::get()->quickSound("anvil");
    }
    else
    {
        // It's unsafe to delete from inside the event handler so we do it
        // in onUpdate (which checks for m_self_destroy)
        m_self_destroy = true;
    }
}

// -----------------------------------------------------------------------------
void EnterGPNameDialog::onUpdate(float dt)
{
    // It's unsafe to delete from inside the event handler so we do it here
    if (m_self_destroy)
    {
        TextBoxWidget* textCtrl = getWidget<TextBoxWidget>("textfield");
        stringw name = textCtrl->getText().trim();

        // irrLicht is too stupid to remove focus from deleted widgets
        // so do it by hand
        GUIEngine::getGUIEnv()->removeFocus( textCtrl->getIrrlichtElement() );
        GUIEngine::getGUIEnv()->removeFocus( m_irrlicht_window );

        // we will destroy the dialog before notifying the listener to be safer.
        // but in order not to crash we must make a local copy of the listern
        // otherwise we will crash
        INewGPListener* listener = m_listener;

        ModalDialog::dismiss();

        if (listener != NULL)
            listener->onNewGPWithName(name);
    }
}
