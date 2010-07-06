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
#include "guiengine/widgets/progress_bar_widget.hpp"
#include "utils/string_utils.hpp"
#include <string.h>
using namespace GUIEngine;
using namespace irr::core;

// -----------------------------------------------------------------------------

ProgressBarWidget::ProgressBarWidget() : Widget(WTYPE_PROGRESS)
{
    m_value = 0;
}

// -----------------------------------------------------------------------------

void ProgressBarWidget::add()
{
    rect<s32> widget_size = rect<s32>(m_x, m_y, m_x + m_w, m_y + m_h);
    stringw&  message = m_text;
    m_element = GUIEngine::getGUIEnv()->addButton(widget_size, m_parent, getNewID(), message.c_str(), L"");
    
    m_id = m_element->getID();
    m_element->setTabOrder(m_id);
    m_element->setTabGroup(false);
}
// -----------------------------------------------------------------------------

void ProgressBarWidget::setValue(int value)
{
    m_value = value;
    setLabel(std::string(StringUtils::toString(value) + "\%").c_str());
}
// -----------------------------------------------------------------------------

void ProgressBarWidget::setLabel(irr::core::stringw label)
{
    m_element->setText( label.c_str() );
    m_text = label;
}

// -----------------------------------------------------------------------------

