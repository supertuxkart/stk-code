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


#include "config/player.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
#include "states_screens/dialogs/confirm_resolution_dialog.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

using namespace GUIEngine;
using namespace irr::gui;
using namespace irr::core;

// ------------------------------------------------------------------------------------------------------

ConfirmResolutionDialog::ConfirmResolutionDialog() : ModalDialog(0.7f, 0.7f)
{    
    m_countdown_message = NULL;
    
    IGUIFont* font = GUIEngine::getFont();
    const int textHeight = GUIEngine::getFontHeight();
    const int buttonHeight = textHeight + 10;
    
    const int y_bottom = m_area.getHeight() - 2*(buttonHeight + 10) - 10;

    m_remaining_time = 10.99f;
    
    // ---- Add label
    core::rect<s32> text_area( 15, 15, m_area.getWidth()-15, y_bottom-15 );
    
    m_countdown_message = GUIEngine::getGUIEnv()->addStaticText( L" ",
                                                              text_area, false , true , // border, word warp
                                                              m_irrlicht_window);
    m_countdown_message->setTabStop(false);
     
    updateMessage();
       
    // ---- Add accept button
    {
        ButtonWidget* widget = new ButtonWidget();
        widget->m_properties[PROP_ID] = "accept";
        
        //I18N: In the 'confirm resolution' dialog, that's shown when switching resoluton
        widget->m_text = _("Keep this resolution");
        
        const int textWidth = font->getDimension( widget->m_text.c_str() ).Width + 40;
        
        widget->m_x = m_area.getWidth()/2 - textWidth/2;
        widget->m_y = y_bottom;
        widget->m_w = textWidth;
        widget->m_h = buttonHeight;
        widget->setParent(m_irrlicht_window);
        m_children.push_back(widget);
        widget->add();
    }
    // ---- Add cancel button
    {
        ButtonWidget* widget = new ButtonWidget();
        widget->m_properties[PROP_ID] = "cancel";
        widget->m_text = _("Cancel");
        
        const int textWidth = font->getDimension( widget->m_text.c_str() ).Width + 40;
        
        widget->m_x = m_area.getWidth()/2 - textWidth/2;
        widget->m_y = y_bottom + buttonHeight + 10;
        widget->m_w = textWidth;
        widget->m_h = buttonHeight;
        widget->setParent(m_irrlicht_window);
        m_children.push_back(widget);
        widget->add();
        
        widget->setFocusForPlayer( PLAYER_ID_GAME_MASTER );
    }
    
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

// ------------------------------------------------------------------------------------------------------

void ConfirmResolutionDialog::updateMessage()
{
    assert(m_countdown_message != NULL);
    stringw msg = StringUtils::insertValues(_("Confirm resolution within %i seconds"),
                                            (int)m_remaining_time);
    //std::cout << stringc(msg.c_str()).c_str() << std::endl;
    m_countdown_message->setText( msg.c_str() );
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
