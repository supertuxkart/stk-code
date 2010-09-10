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
#include "guiengine/modaldialog.hpp"
#include "guiengine/widgets/text_box_widget.hpp"

using namespace GUIEngine;
using namespace irr::core;
using namespace irr::gui;
using namespace irr;

// -----------------------------------------------------------------------------

TextBoxWidget::TextBoxWidget() : Widget(WTYPE_TEXTBOX)
{
}

// -----------------------------------------------------------------------------

void TextBoxWidget::add()
{
    rect<s32> widget_size = rect<s32>(m_x, m_y, m_x + m_w, m_y + m_h);
    
    // Don't call TextBoxWidget::getText(), which assumes that the irrlicht
    // widget already exists. 
    const stringw& text = Widget::getText();
    m_element = GUIEngine::getGUIEnv()->addEditBox(text.c_str(), widget_size,
                                                   true /* border */, m_parent, getNewID());
    m_id = m_element->getID();
    m_element->setTabOrder(m_id);
    m_element->setTabGroup(false);
    m_element->setTabStop(true);
}

// -----------------------------------------------------------------------------

stringw TextBoxWidget::getText() const
{
    const IGUIEditBox* textCtrl =  Widget::getIrrlichtElement<IGUIEditBox>();
    assert(textCtrl != NULL);
    
    return stringw(textCtrl->getText());
}

// -----------------------------------------------------------------------------

EventPropagation TextBoxWidget::focused(const int playerID)
{
    assert(playerID == 0); // No support for multiple players in text areas!
    
    // special case : to work, the text box must receive "irrLicht focus", STK focus is not enough
    GUIEngine::getGUIEnv()->setFocus(m_element);
    setWithinATextBox(true);
    return EVENT_LET;
}

// -----------------------------------------------------------------------------

void TextBoxWidget::unfocused(const int playerID)
{
    assert(playerID == 0); // No support for multiple players in text areas!

    setWithinATextBox(false);
    
    // special case : to work, the text box must receive "irrLicht focus", STK focus is not enough
    // below is a cheap way to unset the irrLicht focus from the widget (nope, 'removeFocus' from
    // IGUIEnv doesn't work reliably, not sure why)
    // currently, text boxes are only used in modal dialogs, so I shift the focus to the dialog
    assert( ModalDialog::isADialogActive() );
    GUIEngine::getGUIEnv()->setFocus( ModalDialog::getCurrent()->getIrrlichtElement() );
}

