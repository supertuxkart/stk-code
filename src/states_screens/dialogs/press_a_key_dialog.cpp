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


#include "guiengine/engine.hpp"
#include "guiengine/widgets.hpp"
#include "input/input_manager.hpp"
#include "states_screens/dialogs/press_a_key_dialog.hpp"
#include "utils/translation.hpp"

using namespace GUIEngine;
using namespace irr::gui;

// ------------------------------------------------------------------------------------------------------

PressAKeyDialog::PressAKeyDialog(const float w, const float h) :
        ModalDialog(w, h)
{
    LabelWidget* widget = new LabelWidget();
    widget->m_text = _("Press a key");
    widget->m_properties[PROP_TEXT_ALIGN] = "center";
    widget->m_x = 0;
    widget->m_y = 0;
    widget->m_w = m_area.getWidth();
    widget->m_h = m_area.getHeight()/2;
    widget->setParent(m_irrlicht_window);
    
    m_children.push_back(widget);
    widget->add();
    
    
    //IGUIFont* font = GUIEngine::getFont();
    const int textHeight = GUIEngine::getFontHeight();
        
    ButtonWidget* widget2 = new ButtonWidget();
    widget2->m_properties[PROP_ID] = "cancel";
    widget2->m_text = _("Press ESC to cancel");
    widget2->m_x = 15;
    widget2->m_y = m_area.getHeight() - textHeight - 12;
    widget2->m_w = m_area.getWidth() - 30;
    widget2->m_h = textHeight + 6;
    widget2->setParent(m_irrlicht_window);
    
    m_children.push_back(widget2);
    widget2->add();
}

// ------------------------------------------------------------------------------------------------------

GUIEngine::EventPropagation PressAKeyDialog::processEvent(const std::string& eventSource)
{
    if(eventSource == "cancel")
    {
        input_manager->setMode(InputManager::MENU);
        dismiss();
        return GUIEngine::EVENT_BLOCK;
    }
    return GUIEngine::EVENT_LET;
}

// ------------------------------------------------------------------------------------------------------
