//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Marianne Gagnon
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

#include "states_screens/dialogs/enter_player_name_dialog.hpp"

#include <IGUIEnvironment.h>

#include "audio/sfx_manager.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/translation.hpp"

using namespace GUIEngine;
using namespace irr;
using namespace irr::core;
using namespace irr::gui;

// -----------------------------------------------------------------------------

EnterPlayerNameDialog::EnterPlayerNameDialog(INewPlayerListener* listener,
                                             const float w, const float h) :
        ModalDialog(w, h)
{
    m_listener = listener;
    m_self_destroy = false;
    loadFromFile("enter_player_name_dialog.stkgui");
    
    TextBoxWidget* textCtrl = getWidget<TextBoxWidget>("textfield");
    assert(textCtrl != NULL);
    textCtrl->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    
    //if (translations->isRTLLanguage()) textCtrl->addListener(this);
}

// -----------------------------------------------------------------------------

EnterPlayerNameDialog::~EnterPlayerNameDialog()
{
    // FIXME: what is this code for?
    TextBoxWidget* textCtrl = getWidget<TextBoxWidget>("textfield");
    textCtrl->getIrrlichtElement()->remove();
    textCtrl->clearListeners();
}

// -----------------------------------------------------------------------------

/*
void EnterPlayerNameDialog::onTextUpdated()
{
    TextBoxWidget* textCtrl = getWidget<TextBoxWidget>("textfield");
    LabelWidget* lbl = getWidget<LabelWidget>("preview");
    
    lbl->setText( core::stringw(translations->fribidize(textCtrl->getText())), false );
}
*/
// -----------------------------------------------------------------------------

GUIEngine::EventPropagation EnterPlayerNameDialog::processEvent(const std::string& eventSource)
{
    if (eventSource == "cancel")
    {
        dismiss();
        return GUIEngine::EVENT_BLOCK;
    }
    return GUIEngine::EVENT_LET;
}

// -----------------------------------------------------------------------------

void EnterPlayerNameDialog::onEnterPressedInternal()
{
    // ---- Cancel button pressed
    const int playerID = PLAYER_ID_GAME_MASTER;
    ButtonWidget* cancelButton = getWidget<ButtonWidget>("cancel");
    if (GUIEngine::isFocusedForPlayer(cancelButton, playerID))
    {
        std::string fakeEvent = "cancel";
        processEvent(fakeEvent);
        return;
    }
        
    // ---- Otherwise, accept entered name
    TextBoxWidget* textCtrl = getWidget<TextBoxWidget>("textfield");
    stringw playerName = textCtrl->getText();
    const int size = playerName.size();
    
    // sanity check
    int nonEmptyChars = 0;
    for (int n=0; n<size; n++)
    {
        if (playerName[n] != L' ')
        {
            nonEmptyChars++;
        }
    }
    
    
    if (size > 0 && nonEmptyChars > 0)
    {
        // check for duplicates
        const int amount = UserConfigParams::m_all_players.size();
        for (int n=0; n<amount; n++)
        {
            if (UserConfigParams::m_all_players[n].getName() == playerName)
            {
                LabelWidget* label = getWidget<LabelWidget>("title");
                label->setText(_("Cannot add a player with this name."), false);
                sfx_manager->quickSound( "anvil" );
                return;
            }
        }
        
        UserConfigParams::m_all_players.push_back( new PlayerProfile(playerName) );
        
        // It's unsafe to delete from inside the event handler so we do it later
        m_self_destroy = true;
    }
    else
    {
        LabelWidget* label = getWidget<LabelWidget>("title");
        label->setText(_("Cannot add a player with this name."), false);
        sfx_manager->quickSound( "anvil" );
    }
}

// -----------------------------------------------------------------------------

void EnterPlayerNameDialog::onUpdate(float dt)
{
    // It's unsafe to delete from inside the event handler so we do it later
    if (m_self_destroy)
    {
        TextBoxWidget* textCtrl = getWidget<TextBoxWidget>("textfield");
        stringw playerName = textCtrl->getText();
    
        // irrLicht is too stupid to remove focus from deleted widgets
        // so do it by hand
        GUIEngine::getGUIEnv()->removeFocus( textCtrl->getIrrlichtElement() );
        GUIEngine::getGUIEnv()->removeFocus( m_irrlicht_window );
        
        // we will destroy the dialog before notifying the listener to be safer.
        // but in order not to crash we must make a local copy of the listern
        // otherwise we will crash
        INewPlayerListener* listener = m_listener;
        
        ModalDialog::dismiss();
        
        if (listener != NULL) listener->onNewPlayerWithName( playerName );
    }
}
