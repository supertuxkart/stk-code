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
#include "guiengine/widgets/progress_bar_widget.hpp"
#include "utils/string_utils.hpp"
#include <string.h>

#include <IGUIEnvironment.h>
#include <IGUIElement.h>
#include <IGUIButton.h>

using namespace GUIEngine;
using namespace irr::core;
using namespace irr;

// -----------------------------------------------------------------------------

ProgressBarWidget::ProgressBarWidget(bool show_label) : Widget(WTYPE_PROGRESS)
{
    // A progress bar consists of two parts, so need two containers
    m_next = new SkinWidgetContainer;

    m_value = 0;
    m_target_value = 0;
    m_previous_value = 0;
    m_show_label = show_label;
    setFocusable(false);
}   // ProgressBarWidget

// -----------------------------------------------------------------------------
ProgressBarWidget::~ProgressBarWidget()
{
    GUIEngine::needsUpdate.remove(this);
}   // ~ProgressBarWidget

// -----------------------------------------------------------------------------

void ProgressBarWidget::add()
{
    rect<s32> widget_size = rect<s32>(m_x, m_y, m_x + m_w, m_y + m_h);
    stringw&  message = m_text;
    m_element = GUIEngine::getGUIEnv()->addButton(widget_size, m_parent, 
                                                  getNewNoFocusID(),
                                                  message.c_str(), L"");

    m_id = m_element->getID();
    m_element->setTabStop(false);
    m_element->setTabGroup(false);

    /* Copied from model_view_widget.cpp
     FIXME: remove this unclean thing, I think irrlicht provides this feature:
     virtual void IGUIElement::OnPostRender (u32 timeMs)
     \brief animate the element and its children.
     */
    GUIEngine::needsUpdate.push_back(this);
    if (!m_is_visible)
        m_element->setVisible(false);
}    // add

// -----------------------------------------------------------------------------

void ProgressBarWidget::setValue(float value)
{
    m_value = value/100.0f;
    m_target_value = m_value;
    m_previous_value = m_value;
    if (m_show_label)
    {
        int percent = int(value);
        setLabel(std::string(StringUtils::toString(percent) + "%").c_str());
    }
}   // setValue

// -----------------------------------------------------------------------------

void ProgressBarWidget::moveValue(float value)
{
    m_previous_value = m_value;
    m_target_value = value/100.0f;
    if (m_show_label)
    {
        int percent = int(value);
        setLabel(std::string(StringUtils::toString(percent) + "%").c_str());
    }
}   // moveValue

// -----------------------------------------------------------------------------

void ProgressBarWidget::update(float delta)
{
    if (m_target_value != m_value)
    {
        // Compute current progress in the animation
        float cur = (m_value        - m_previous_value) 
                  / (m_target_value - m_previous_value);
        // Animation time: 1.0 seconds
        cur += delta * 10;
        if (cur > 1)
            cur = 1;
        m_value = m_previous_value 
                + cur * (m_target_value - m_previous_value);
    }
}   // update

// -----------------------------------------------------------------------------

void ProgressBarWidget::setLabel(irr::core::stringw label)
{
    m_element->setText( label.c_str() );
    m_text = label;
}   // setLabel

// -----------------------------------------------------------------------------
