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
#include "guiengine/widgets/button_widget.hpp"
using namespace GUIEngine;

ButtonWidget::ButtonWidget()
{
    m_type = WTYPE_BUTTON;
}
// -----------------------------------------------------------------------------
void ButtonWidget::add()
{
    rect<s32> widget_size = rect<s32>(x, y, x + w, y + h);
    stringw  message = m_properties[PROP_TEXT].c_str();
    m_element = GUIEngine::getGUIEnv()->addButton(widget_size, m_parent, getNewID(), message.c_str(), L"");
    
    id = m_element->getID();
    m_element->setTabOrder(id);
    m_element->setTabGroup(false);
}
// -----------------------------------------------------------------------------
void ButtonWidget::setLabel(const char* label)
{
    m_element->setText( stringw(label).c_str() );
}

