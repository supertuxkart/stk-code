//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 Marianne Gagnon
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
#include "guiengine/widgets/bubble_widget.hpp"
#include <irrlicht.h>
using namespace irr::core;
using namespace irr::gui;

using namespace GUIEngine;

// ----------------------------------------------------------------------------

BubbleWidget::BubbleWidget() : Widget(WTYPE_BUBBLE)
{
    m_zoom = 0.0f;
}

// ----------------------------------------------------------------------------

void BubbleWidget::add()
{
    m_shrinked_size = rect<s32>(m_x, m_y, m_x + m_w, m_y + m_h);
    stringw message = getText();
    
    EGUI_ALIGNMENT align = EGUIA_UPPERLEFT;
    if      (m_properties[PROP_TEXT_ALIGN] == "center") align = EGUIA_CENTER;
    else if (m_properties[PROP_TEXT_ALIGN] == "right")  align = EGUIA_LOWERRIGHT;
    EGUI_ALIGNMENT valign = EGUIA_CENTER ; //TODO: make label v-align configurable through XML file?
    
    IGUIStaticText* irrwidget;
    irrwidget = GUIEngine::getGUIEnv()->addStaticText(message.c_str(), m_shrinked_size,
                                                      false, true /* word wrap */, m_parent, getNewID());
        
    // find expanded bubble size
    int text_height = irrwidget->getTextHeight();

    m_expanded_size = m_shrinked_size;
    const int additionalNeededSize = std::max(0, text_height - m_shrinked_size.getHeight());
    m_expanded_size.UpperLeftCorner.Y  -= additionalNeededSize/2;
    m_expanded_size.LowerRightCorner.Y += additionalNeededSize/2;

    // reduce text to fit in the available space if it's too long
    while (text_height > m_shrinked_size.getHeight())
    {
        message = message.subString(0, message.size() - 10) + "...";
        irrwidget->setText(message.c_str());
        text_height = irrwidget->getTextHeight();
    }
    m_shrinked_text = message;
    
    m_element = irrwidget;
    irrwidget->setTextAlignment( align, valign );
    
    m_id = m_element->getID();
    
    m_element->setTabOrder(m_id);
    m_element->setTabStop(true);
    
    m_element->setNotClipped(true);
    irrwidget->setDrawBorder(true);
}

// ----------------------------------------------------------------------------

EventPropagation BubbleWidget::focused(const int playerID)
{
    if (m_element != NULL)
    {
        IGUIStaticText* widget = getIrrlichtElement<IGUIStaticText>();
        //widget->setDrawBorder(true);
        widget->setRelativePosition(m_expanded_size);
        widget->setText(getText().c_str());
    }
    return EVENT_LET;
}

// ----------------------------------------------------------------------------

void BubbleWidget::unfocused(const int playerID)
{
    if (m_element != NULL)
    {
        IGUIStaticText* widget = getIrrlichtElement<IGUIStaticText>();
        //widget->setDrawBorder(false);
        widget->setRelativePosition(m_shrinked_size);
        widget->setText(m_shrinked_text.c_str());
    }
}

// ----------------------------------------------------------------------------
