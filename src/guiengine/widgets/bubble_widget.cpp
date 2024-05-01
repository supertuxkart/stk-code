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

#include "font/font_manager.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/widgets/bubble_widget.hpp"
#include "online/link_helper.hpp"
#include <algorithm>

#include <GlyphLayout.h>
#include <IGUIStaticText.h>
#include <IGUIElement.h>
#include <IGUIEnvironment.h>
#include <IGUIFont.h>

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

rect<s32> BubbleWidget::getShrinkedSize()
{
    recti size = rect<s32>(m_x, m_y, m_x + m_w - BUBBLE_MARGIN_ON_RIGHT, m_y + m_h);
    size.LowerRightCorner.Y -= BOTTOM_MARGIN;
    return size;
}

// ----------------------------------------------------------------------------

void BubbleWidget::add()
{
    m_shrinked_size = getShrinkedSize();
    stringw message = getText();

    IGUIStaticText* irrwidget;
    irrwidget = GUIEngine::getGUIEnv()->addStaticText(message.c_str(), m_shrinked_size,
                                                      false, true /* word wrap */, m_parent,
                                                      (m_focusable ? getNewID() : getNewNoFocusID()));
    irrwidget->setTextRestrainedInside(false);
    irrwidget->setMouseCallback([this](irr::gui::IGUIStaticText* text,
                                       irr::SEvent::SMouseInput mouse)->bool
    {
        if (getText() != m_shrinked_text &&
            stringw(text->getText()) == m_shrinked_text)
        {
            return false;
        }
        return Online::LinkHelper::openURLIrrElement(text, mouse);
    });

    m_element = irrwidget;
    replaceText();
    m_id = m_element->getID();

    m_element->setTabOrder(m_id);
    m_element->setTabStop(true);
}

void BubbleWidget::resize()
{
    m_shrinked_size = getShrinkedSize();
    m_element->setRelativePosition(m_shrinked_size);
    updateForNewSize();
    updateSize();
}

void BubbleWidget::updateForNewSize()
{
    IGUIStaticText* irrwidget = static_cast<IGUIStaticText*>(m_element);
    // find expanded bubble size
    m_shrinked_text = getText();
    irrwidget->setText(m_shrinked_text);

start:
    IGUIFont* font = irrwidget->getActiveFont();
    int text_height = font->getHeightPerLine();
    bool has_newline = false;
#ifndef SERVER_ONLY
    int max_cluster = -1;
    auto& glyph_layouts = irrwidget->getGlyphLayouts();
    for (unsigned i = 0; i < glyph_layouts.size(); i++)
    {
        const GlyphLayout& glyph = glyph_layouts[i];
        if ((glyph.flags & GLF_NEWLINE) != 0)
        {
            has_newline = true;
            text_height += font->getHeightPerLine();
            if (text_height > m_shrinked_size.getHeight())
            {
                if (max_cluster != -1)
                    continue;
                for (unsigned idx = 0; idx < i; idx++)
                {
                    const GlyphLayout& gl = glyph_layouts[idx];
                    for (int c : gl.cluster)
                    {
                        if (c > max_cluster)
                            max_cluster = c;
                    }
                }
                while (max_cluster > 0 && LineBreakingRules::endSentence(
                    m_shrinked_text[max_cluster - 1]))
                    max_cluster--;
                m_shrinked_text = m_shrinked_text.subString(0, max_cluster) +
                    L"\u2026";
                irrwidget->setText(m_shrinked_text);
            }
        }
    }
#endif
    irrwidget->setText(getText());
    text_height = irrwidget->getTextHeight();

    m_expanded_size = m_shrinked_size;
    const int additionalNeededSize = std::max(0, text_height - m_shrinked_size.getHeight());

    if (additionalNeededSize > 0)
    {
        m_expanded_size.UpperLeftCorner.Y  -= additionalNeededSize/2 + 10;
        m_expanded_size.LowerRightCorner.Y += additionalNeededSize/2 + 10;
    }
    irrwidget->setText(m_shrinked_text);
    text_height = irrwidget->getTextHeight();
    if (has_newline && text_height > m_shrinked_size.getHeight())
        goto start;
}

void BubbleWidget::replaceText()
{
    IGUIStaticText* irrwidget = (IGUIStaticText*) m_element;
    // Take border into account for line breaking (happens in setText)
    irrwidget->setDrawBorder(true);
    irrwidget->setNotClipped(true);

    EGUI_ALIGNMENT align = EGUIA_UPPERLEFT;
    if      (m_properties[PROP_TEXT_ALIGN] == "center") align = EGUIA_CENTER;
    else if (m_properties[PROP_TEXT_ALIGN] == "right")  align = EGUIA_LOWERRIGHT;

    EGUI_ALIGNMENT valign = EGUIA_CENTER;
    if (m_properties[PROP_TEXT_VALIGN] == "top") valign = EGUIA_UPPERLEFT;
    if (m_properties[PROP_TEXT_VALIGN] == "bottom") valign = EGUIA_LOWERRIGHT;

    updateForNewSize();
    irrwidget->setTextAlignment( align, valign );
}

void BubbleWidget::setText(const irr::core::stringw &s)
{
    Widget::setText(s);
    if (m_element != NULL)
    {
        //If add() has already been called (and thus m_element is set) we need to replace the text.
        replaceText();
    }
}

void BubbleWidget::updateSize()
{
    stringw real_text = getText();
    if (m_shrinked_text == real_text)
        return;
    core::rect<s32> currsize = m_shrinked_size;

    const int y1_top    = m_shrinked_size.UpperLeftCorner.Y;
    const int y1_bottom = m_shrinked_size.LowerRightCorner.Y;

    const int y2_top    = m_expanded_size.UpperLeftCorner.Y;
    const int y2_bottom = m_expanded_size.LowerRightCorner.Y;

    currsize.UpperLeftCorner.Y  = (int)(y1_top + (y2_top - y1_top)*m_zoom);
    currsize.LowerRightCorner.Y = (int)(y1_bottom
                                        +(y2_bottom - y1_bottom)*m_zoom);

    m_element->setRelativePosition(currsize);

    IGUIStaticText* irrwidget = static_cast<IGUIStaticText*>(m_element);
    if (m_zoom > 0.5f)
        irrwidget->setText(real_text);
    else
        irrwidget->setText(m_shrinked_text);
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
