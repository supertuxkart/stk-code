//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015 Marianne Gagnon
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
#include <IGUIElement.h>
#include <IGUIEnvironment.h>
#include <IGUIButton.h>
using namespace GUIEngine;
using namespace irr::core;
using namespace irr;

// -----------------------------------------------------------------------------

ButtonWidget::ButtonWidget() : Widget(WTYPE_BUTTON)
{
}

// -----------------------------------------------------------------------------

void ButtonWidget::add()
{
    rect<s32> widget_size = rect<s32>(m_x, m_y, m_x + m_w, m_y + m_h);
    const stringw&  message = getText();
    m_element = GUIEngine::getGUIEnv()->addButton(widget_size, m_parent,
                                                  (m_reserved_id == -1 ? getNewID() : m_reserved_id),
                                                  message.c_str(), L"");

    m_id = m_element->getID();
    m_element->setTabOrder(m_id);
    m_element->setTabGroup(false);
    if (!m_is_visible)
        m_element->setVisible(false);
}

// -----------------------------------------------------------------------------

void ButtonWidget::setLabel(const irr::core::stringw &label)
{
    // This method should only be called AFTER a widget is added
    assert(m_element != NULL);

    m_element->setText( label.c_str() );
    setText(label);
}

// -----------------------------------------------------------------------------

