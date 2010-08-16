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

#include "states_screens/dialogs/player_info_dialog.hpp"

#include "config/player.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/widgets.hpp"
#include "states_screens/options_screen_players.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

using namespace GUIEngine;
using namespace irr::gui;
using namespace irr::core;

// ------------------------------------------------------------------------------------------------------

PlayerInfoDialog::PlayerInfoDialog(PlayerProfile* player, const float w, const float h) : ModalDialog(w, h)
{
    m_player = player;
    
    showRegularDialog();
}
// ------------------------------------------------------------------------------------------------------
void PlayerInfoDialog::showRegularDialog()
{
    clearWindow();
    
    const int y1 = m_area.getHeight()/6;
    const int y2 = m_area.getHeight()*2/6;
    const int y3 = m_area.getHeight()*3/6;
    const int y4 = m_area.getHeight()*5/6;
    
    ScalableFont* font = GUIEngine::getFont();
    const int textHeight = GUIEngine::getFontHeight();
    const int buttonHeight = textHeight + 10;
    
    {
        textCtrl = new TextBoxWidget();
        textCtrl->m_properties[PROP_ID] = "renameplayer";
        textCtrl->m_text = m_player->getName();
        textCtrl->m_x = 50;
        textCtrl->m_y = y1 - textHeight/2;
        textCtrl->m_w = m_area.getWidth()-100;
        textCtrl->m_h = textHeight + 5;
        textCtrl->setParent(m_irrlicht_window);
        m_widgets.push_back(textCtrl);
        textCtrl->add();
    }
    
    {
        ButtonWidget* widget = new ButtonWidget();
        widget->m_properties[PROP_ID] = "renameplayer";
        
        //I18N: In the player info dialog
        widget->m_text = _("Rename");
        
        const int textWidth = font->getDimension( widget->m_text.c_str() ).Width + 40;
        
        widget->m_x = m_area.getWidth()/2 - textWidth/2;
        widget->m_y = y2;
        widget->m_w = textWidth;
        widget->m_h = buttonHeight;
        widget->setParent(m_irrlicht_window);
        m_widgets.push_back(widget);
        widget->add();
    }
    {
        ButtonWidget* widget = new ButtonWidget();
        widget->m_properties[PROP_ID] = "cancel";
        widget->m_text = _("Cancel");
        
        const int textWidth = font->getDimension( widget->m_text.c_str() ).Width + 40;
        
        widget->m_x = m_area.getWidth()/2 - textWidth/2;
        widget->m_y = y3;
        widget->m_w = textWidth;
        widget->m_h = buttonHeight;
        widget->setParent(m_irrlicht_window);
        m_widgets.push_back(widget);
        widget->add();
    }
    
    {
        ButtonWidget* widget = new ButtonWidget();
        widget->m_properties[PROP_ID] = "removeplayer";
        
        //I18N: In the player info dialog
        widget->m_text = _("Remove");
        
        const int textWidth = font->getDimension( widget->m_text.c_str() ).Width + 40;
        
        widget->m_x = m_area.getWidth()/2 - textWidth/2;
        widget->m_y = y4;
        widget->m_w = textWidth;
        widget->m_h = buttonHeight;
        widget->setParent(m_irrlicht_window);
        m_widgets.push_back(widget);
        widget->add();
    }
    
    textCtrl->setFocusForPlayer( PLAYER_ID_GAME_MASTER );
}

// ------------------------------------------------------------------------------------------------------

void PlayerInfoDialog::showConfirmDialog()
{
    clearWindow();
        
    
    IGUIFont* font = GUIEngine::getFont();
    const int textHeight = GUIEngine::getFontHeight();
    const int buttonHeight = textHeight + 10;
    
    
    irr::core::stringw message = 
        //I18N: In the player info dialog (when deleting)
        StringUtils::insertValues( _("Do you really want to delete player '%s' ?"), m_player->getName());
    
    core::rect< s32 > area_left(5, 0, m_area.getWidth()-5, m_area.getHeight()/2);
    
    // When there is no need to tab through / click on images/labels, we can add directly
    // irrlicht labels (more complicated uses require the use of our widget set)
    IGUIStaticText* a = GUIEngine::getGUIEnv()->addStaticText( message.c_str(),
                                              area_left, false /* border */, true /* word wrap */,
                                              m_irrlicht_window);
    a->setTextAlignment(EGUIA_CENTER, EGUIA_CENTER);

    {
        ButtonWidget* widget = new ButtonWidget();
        widget->m_properties[PROP_ID] = "confirmremove";
        
        //I18N: In the player info dialog (when deleting)
        widget->m_text = _("Confirm Remove");
        
        const int textWidth = font->getDimension( widget->m_text.c_str() ).Width + 40;
        
        widget->m_x = m_area.getWidth()/2 - textWidth/2;
        widget->m_y = m_area.getHeight()/2;
        widget->m_w = textWidth;
        widget->m_h = buttonHeight;
        widget->setParent(m_irrlicht_window);
        m_widgets.push_back(widget);
        widget->add();
    }
    
    {
        ButtonWidget* widget = new ButtonWidget();
        widget->m_properties[PROP_ID] = "cancelremove";
        
        //I18N: In the player info dialog (when deleting)
        widget->m_text = _("Cancel Remove");
        
        const int textWidth = font->getDimension( widget->m_text.c_str() ).Width + 40;
        
        widget->m_x = m_area.getWidth()/2 - textWidth/2;
        widget->m_y = m_area.getHeight()*3/4;
        widget->m_w = textWidth;
        widget->m_h = buttonHeight;
        widget->setParent(m_irrlicht_window);
        m_widgets.push_back(widget);
        widget->add();
        
        widget->setFocusForPlayer( PLAYER_ID_GAME_MASTER );
    }
    
}

// ------------------------------------------------------------------------------------------------------

void PlayerInfoDialog::onEnterPressedInternal()
{
}

// ------------------------------------------------------------------------------------------------------

GUIEngine::EventPropagation PlayerInfoDialog::processEvent(const std::string& eventSource)
{
    if (eventSource == "renameplayer")
    {
        // accept entered name
        stringw playerName = textCtrl->getText();
        if (playerName.size() > 0)
        {
            OptionsScreenPlayers::getInstance()->gotNewPlayerName( playerName, m_player );
        }
        
        // irrLicht is too stupid to remove focus from deleted widgets
        // so do it by hand
        GUIEngine::getGUIEnv()->removeFocus( textCtrl->getIrrlichtElement() );
        GUIEngine::getGUIEnv()->removeFocus( m_irrlicht_window );
        
        ModalDialog::dismiss();
        
        dismiss();
        return GUIEngine::EVENT_BLOCK;
    }
    else if (eventSource == "removeplayer")
    {
        showConfirmDialog();
        return GUIEngine::EVENT_BLOCK;
    }
    else if (eventSource == "confirmremove")
    {
        OptionsScreenPlayers::getInstance()->deletePlayer( m_player );

        // irrLicht is too stupid to remove focus from deleted widgets
        // so do it by hand
        GUIEngine::getGUIEnv()->removeFocus( textCtrl->getIrrlichtElement() );
        GUIEngine::getGUIEnv()->removeFocus( m_irrlicht_window );
        
        ModalDialog::dismiss();
        return GUIEngine::EVENT_BLOCK;
    }
    else if(eventSource == "cancelremove")
    {
        showRegularDialog();
        return GUIEngine::EVENT_BLOCK;
    }
    else if(eventSource == "cancel")
    {   
        // irrLicht is too stupid to remove focus from deleted widgets
        // so do it by hand
        GUIEngine::getGUIEnv()->removeFocus( textCtrl->getIrrlichtElement() );
        GUIEngine::getGUIEnv()->removeFocus( m_irrlicht_window );
        
        ModalDialog::dismiss();
        return GUIEngine::EVENT_BLOCK;
    }
    return GUIEngine::EVENT_LET;
}

// ------------------------------------------------------------------------------------------------------

