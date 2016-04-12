//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 Marianne Gagnon
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
#include "utils/translation.hpp"
#include <algorithm>

#include <IGUIStaticText.h>
#include <IGUIElement.h>
#include <IGUIEnvironment.h>

using namespace irr::core;
using namespace irr::gui;
using namespace irr;

using namespace GUIEngine;

const int BOTTOM_MARGIN = 10;

// ----------------------------------------------------------------------------

BubbleWidget::BubbleWidget() : Widget(WTYPE_BUBBLE)
{
    m_zoom = 0.0f;
}

// ----------------------------------------------------------------------------

void BubbleWidget::add()
{
    m_shrinked_size = rect<s32>(m_x, m_y, m_x + m_w - BUBBLE_MARGIN_ON_RIGHT, m_y + m_h);
    stringw message = getText();

    m_shrinked_size.LowerRightCorner.Y -= BOTTOM_MARGIN;

    IGUIStaticText* irrwidget;
    irrwidget = GUIEngine::getGUIEnv()->addStaticText(message.c_str(), m_shrinked_size,
                                                      false, true /* word wrap */, m_parent,
                                                      (m_focusable ? getNewID() : getNewNoFocusID()));
    irrwidget->setTextRestrainedInside(false);
    irrwidget->setRightToLeft(translations->isRTLText(message));

    m_element = irrwidget;
    replaceText();
    m_id = m_element->getID();

    m_element->setTabOrder(m_id);
    m_element->setTabStop(true);

    m_element->setNotClipped(true);
    irrwidget->setDrawBorder(true);
}

void BubbleWidget::replaceText()
{
    IGUIStaticText* irrwidget = (IGUIStaticText*) m_element;
    stringw message = getText();

    EGUI_ALIGNMENT align = EGUIA_UPPERLEFT;
    if      (m_properties[PROP_TEXT_ALIGN] == "center") align = EGUIA_CENTER;
    else if (m_properties[PROP_TEXT_ALIGN] == "right")  align = EGUIA_LOWERRIGHT;
    else if (translations->isRTLText(message))          align = EGUIA_LOWERRIGHT;

    EGUI_ALIGNMENT valign = EGUIA_CENTER ; //TODO: make label v-align configurable through XML file?

    // find expanded bubble size
    int text_height = irrwidget->getTextHeight();

    m_expanded_size = m_shrinked_size;
    const int additionalNeededSize = std::max(0, text_height - m_shrinked_size.getHeight());

    if (additionalNeededSize > 0)
    {
        m_expanded_size.UpperLeftCorner.Y  -= additionalNeededSize/2 + 10;
        m_expanded_size.LowerRightCorner.Y += additionalNeededSize/2 + 10;

        // reduce text to fit in the available space if it's too long
        if (translations->isRTLText(message))
        {
            while (text_height > m_shrinked_size.getHeight() && message.size() > 10)
            {
                message = core::stringw(L"...") + message.subString(10, message.size() - 10);
                irrwidget->setText(message.c_str());
                text_height = irrwidget->getTextHeight();
            }
        }
        else
        {
            while (text_height > m_shrinked_size.getHeight() && message.size() > 10)
            {
                message = message.subString(0, message.size() - 10) + "...";
                irrwidget->setText(message.c_str());
                text_height = irrwidget->getTextHeight();
            }
        }
    }
    m_shrinked_text = message;
    irrwidget->setTextAlignment( align, valign );
}

void BubbleWidget::setText(const irr::core::stringw &s)
{
    Widget::setText(s);
    if (m_element != NULL)
    {
        //If add() has already been called (and thus m_element is set) we need to replace the text.
        replaceText();
        getIrrlichtElement<IGUIStaticText>()->setRightToLeft(translations->isRTLText(getText()));
    }
}

void BubbleWidget::updateSize()
{
    core::rect<s32> currsize = m_shrinked_size;

    const int y1_top    = m_shrinked_size.UpperLeftCorner.Y;
    const int y1_bottom = m_shrinked_size.LowerRightCorner.Y;

    const int y2_top    = m_expanded_size.UpperLeftCorner.Y;
    const int y2_bottom = m_expanded_size.LowerRightCorner.Y;

    currsize.UpperLeftCorner.Y  = (int)(y1_top + (y2_top - y1_top)*m_zoom);
    currsize.LowerRightCorner.Y = (int)(y1_bottom
                                        +(y2_bottom - y1_bottom)*m_zoom);

    m_element->setRelativePosition(currsize);

    if (m_zoom > 0.5f)
    {
        getIrrlichtElement<IGUIStaticText>()->setText(getText().c_str());
    }
    else
    {
        getIrrlichtElement<IGUIStaticText>()->setText(m_shrinked_text.c_str());
    }
}

// ----------------------------------------------------------------------------

EventPropagation BubbleWidget::focused(const int playerID)
{
    if (m_element != NULL)
    {
        // bring element to top (with a hack because irrlicht does not appear to offer a built-in way to do this)
        m_element->grab();

        IGUIElement* parent = m_parent;
        if (parent == NULL) parent = GUIEngine::getGUIEnv()->getRootGUIElement();

        parent->removeChild(m_element);
        parent->addChild(m_element);
        m_element->drop();
    }
    return EVENT_LET;
}

// ----------------------------------------------------------------------------
/*
void BubbleWidget::unfocused(const int playerID, Widget* new_focus)
{
    if (m_element != NULL)
    {
        IGUIStaticText* widget = getIrrlichtElement<IGUIStaticText>();
        //widget->setDrawBorder(false);
        widget->setRelativePosition(m_shrinked_size);
        widget->setText(m_shrinked_text.c_str());
    }
}
*/
// ----------------------------------------------------------------------------
