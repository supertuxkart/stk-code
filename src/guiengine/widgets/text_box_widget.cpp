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
#include "guiengine/widgets/text_box_widget.hpp"
using namespace GUIEngine;

// -----------------------------------------------------------------------------
TextBoxWidget::TextBoxWidget()
{
    m_type = WTYPE_TEXTBOX;
}
// -----------------------------------------------------------------------------
void TextBoxWidget::add()
{
    rect<s32> widget_size = rect<s32>(x, y, x + w, y + h);
    
    stringw text = m_properties[PROP_TEXT].c_str();
    m_element = GUIEngine::getGUIEnv()->addEditBox(text.c_str(), widget_size,
                                                   true /* border */, m_parent, getNewID());
    id = m_element->getID();
    m_element->setTabOrder(id);
    m_element->setTabGroup(false);
}
// -----------------------------------------------------------------------------
stringw TextBoxWidget::getText() const
{
    IGUIEditBox* textCtrl = dynamic_cast<IGUIEditBox*>(m_element);
    assert(textCtrl != NULL);
    
    return stringw(textCtrl->getText());
}

