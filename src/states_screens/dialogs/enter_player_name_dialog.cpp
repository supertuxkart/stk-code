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

#include "audio/sfx_manager.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/widgets.hpp"
#include "states_screens/options_screen_players.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/translation.hpp"

using namespace GUIEngine;
using namespace irr;
using namespace irr::core;
using namespace irr::gui;

// -----------------------------------------------------------------------------

EnterPlayerNameDialog::EnterPlayerNameDialog(const float w, const float h) :
        ModalDialog(w, h)
{
    m_label_ctrl = new LabelWidget();
    
    //I18N: In the 'add new player' dialog
    m_label_ctrl->m_text = _("Enter the new player's name");
    
    m_label_ctrl->m_properties[PROP_TEXT_ALIGN] = "center";
    m_label_ctrl->m_x = 0;
    m_label_ctrl->m_y = 0;
    m_label_ctrl->m_w = m_area.getWidth();
    m_label_ctrl->m_h = m_area.getHeight()/3;
    m_label_ctrl->setParent(m_irrlicht_window);
    
    m_children.push_back(m_label_ctrl);
    m_label_ctrl->add();
    
    // ----
    
    //IGUIFont* font = GUIEngine::getFont();
    const int textHeight = GUIEngine::getFontHeight();
    
    const int textAreaYFrom = m_area.getHeight()/2 - textHeight/2;
    
    textCtrl = new TextBoxWidget();
    textCtrl->m_text = "";
    textCtrl->m_x = 50;
    textCtrl->m_y = textAreaYFrom - 10;
    textCtrl->m_w = m_area.getWidth()-100;
    textCtrl->m_h = textHeight + 5;
    textCtrl->setParent(m_irrlicht_window);
    m_children.push_back(textCtrl);
    textCtrl->add();
    textCtrl->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    
    // TODO : add Ok button

    cancelButton = new ButtonWidget();
    cancelButton->m_properties[PROP_ID] = "cancel";
    cancelButton->m_text = _("Cancel");
    cancelButton->m_x = 15;
    cancelButton->m_y = m_area.getHeight() - textHeight - 12;
    cancelButton->m_w = m_area.getWidth() - 30;
    cancelButton->m_h = textHeight + 6;
    cancelButton->setParent(m_irrlicht_window);
    
    m_children.push_back(cancelButton);
    cancelButton->add();

}

// -----------------------------------------------------------------------------

EnterPlayerNameDialog::~EnterPlayerNameDialog()
{
    textCtrl->getIrrlichtElement()->remove();
}

// -----------------------------------------------------------------------------

GUIEngine::EventPropagation EnterPlayerNameDialog::processEvent(const std::string& eventSource)
{
    if(eventSource == "cancel")
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
    const int playerID = 0; // FIXME: don't hardcode player 0?
    if (GUIEngine::isFocusedForPlayer(cancelButton, playerID))
    {
        std::string fakeEvent = "cancel";
        processEvent(fakeEvent);
        return;
    }
        
    // ---- Otherwise, accept entered name
    stringw playerName = textCtrl->getText();
    if (playerName.size() > 0)
    {
        const bool success = OptionsScreenPlayers::getInstance()->gotNewPlayerName( playerName );
        if (!success)
        {
            m_label_ctrl->setText(_("Cannot add a player with this name."));
            sfx_manager->quickSound( "use_anvil" );
            return;
        }
    }
    
    // irrLicht is too stupid to remove focus from deleted widgets
    // so do it by hand
    GUIEngine::getGUIEnv()->removeFocus( textCtrl->getIrrlichtElement() );
    GUIEngine::getGUIEnv()->removeFocus( m_irrlicht_window );

    ModalDialog::dismiss();
}

// -----------------------------------------------------------------------------


