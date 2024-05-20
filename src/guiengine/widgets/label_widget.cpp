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

#include "guiengine/widgets/label_widget.hpp"

#include "guiengine/engine.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/skin.hpp"
#include "online/link_helper.hpp"

#include <IGUIElement.h>
#include <IGUIEnvironment.h>
#include <IGUIStaticText.h>
#include <IGUIButton.h>

#include <assert.h>

using namespace GUIEngine;
using namespace irr::core;
using namespace irr::gui;
using namespace irr;

// ----------------------------------------------------------------------------

LabelWidget::LabelWidget(LabelType type) : Widget(WTYPE_LABEL)
{
    m_type   = type;
    m_scroll_speed = 0;
    m_per_character_size = 0;
    m_scroll_offset = 0;
    m_expand_if_needed = false;

    if (m_type == BRIGHT)
    {
        m_has_color = true;
        m_color = Skin::getColor("brighttext::neutral");
    }
    else
        m_has_color = false;

    setFocusable(false);
}   // LabelWidget

// ----------------------------------------------------------------------------
/** Adds the stk widget to the irrlicht widget set.
 */
void LabelWidget::add()
{
    rect<s32> widget_size = rect<s32>(m_x, m_y, m_x + m_w, m_y + m_h);
    const bool word_wrap = m_properties[PROP_WORD_WRAP] == "true";
    stringw message = getText();

    EGUI_ALIGNMENT align = EGUIA_UPPERLEFT;
    if      (m_properties[PROP_TEXT_ALIGN] == "center") align = EGUIA_CENTER;
    else if (m_properties[PROP_TEXT_ALIGN] == "right")  align = EGUIA_LOWERRIGHT;

    EGUI_ALIGNMENT valign = EGUIA_CENTER ;
    if (m_properties[PROP_TEXT_VALIGN] == "top") valign = EGUIA_UPPERLEFT;
    if (m_properties[PROP_TEXT_VALIGN] == "bottom") valign = EGUIA_LOWERRIGHT;

    IGUIStaticText* irrwidget;
    if (m_scroll_speed != 0)
    {
        IGUIElement* container = GUIEngine::getGUIEnv()->addButton(widget_size, m_parent, -1);
        core::rect<s32> r(core::position2di(0,0), widget_size.getSize());
        irrwidget = GUIEngine::getGUIEnv()->addStaticText(message.c_str(), r,
                                                          false, word_wrap, /*m_parent*/ container, -1);
    }
    else
    {
        irrwidget = GUIEngine::getGUIEnv()->addStaticText(message.c_str(), widget_size,
                                                          false, word_wrap, m_parent, -1);
        irrwidget->setTextRestrainedInside(false);
    }

    irrwidget->setMouseCallback(Online::LinkHelper::openURLIrrElement);
    m_element = irrwidget;
    irrwidget->setTextAlignment( align, valign );

    if (m_has_color)
    {
        irrwidget->setOverrideColor(m_color);
    }

    if (m_type == TITLE)
    {
        irrwidget->setOverrideColor( video::SColor(255,255,255,255) );
        irrwidget->setOverrideFont( GUIEngine::getTitleFont() );
    }
    else if (m_type == SMALL_TITLE)
    {
        irrwidget->setOverrideColor( video::SColor(255,255,255,255) );
        irrwidget->setOverrideFont( GUIEngine::getSmallTitleFont() );
    }
    else if (m_type == TINY_TITLE)
    {
        irrwidget->setOverrideColor( video::SColor(255,255,255,255) );
        irrwidget->setOverrideFont( GUIEngine::getTinyTitleFont() );
    }
    m_per_character_size = getCurrentFont()->getDimension(L"X").Height;

    //irrwidget->setBackgroundColor( video::SColor(255,255,0,0) );
    //irrwidget->setDrawBackground(true);

    m_id = m_element->getID();

    m_element->setTabStop(false);
    m_element->setTabGroup(false);

    if (m_scroll_speed <= 0)
        m_element->setNotClipped(true);

    if (!m_is_visible)
        m_element->setVisible(false);
}   // add

// ----------------------------------------------------------------------------

void LabelWidget::setText(const core::stringw& text, bool expandIfNeeded)
{
    m_scroll_offset = 0;
    m_expand_if_needed = expandIfNeeded;
    updateExpandedText(text);
    Widget::setText(text);
}   // setText

// ----------------------------------------------------------------------------
irr::gui::IGUIFont* LabelWidget::getCurrentFont() const
{
    IGUIFont* font = static_cast<IGUIStaticText*>(m_element)->getOverrideFont();
    if (!font)
        font = GUIEngine::getFont();
    return font;
}   // getCurrentFont

// ----------------------------------------------------------------------------
void LabelWidget::updateExpandedText(const irr::core::stringw& text)
{
    if (m_expand_if_needed)
    {
        assert(m_element != NULL);
        int fwidth = getCurrentFont()->getDimension(text.c_str()).Width;
        core::rect<s32> rect = m_element->getRelativePosition();

        if (rect.getWidth() < fwidth)
        {
            rect.LowerRightCorner.X = rect.UpperLeftCorner.X + fwidth;
            m_element->setRelativePosition(rect);
            m_element->updateAbsolutePosition();
        }
    }
}   // updateExpandedText

// ----------------------------------------------------------------------------
/** Needs to be called to update scrolling.
 *  \param dt Time step size.
 */
void LabelWidget::update(float dt)
{
    if (m_scroll_speed != 0)
    {
        m_scroll_offset -= dt * m_scroll_speed * 5.0f;
        int offset = (float)m_w + m_scroll_offset * m_per_character_size;
        m_element->setRelativePosition( core::position2di( /*m_x +*/ offset,
                                                           /*m_y*/ 0 ) );
    }
}   // update
// ----------------------------------------------------------------------------

bool LabelWidget::scrolledOff() const
{
    // This method may only be called after this widget has been add()ed
    assert(m_element != NULL);
    return (float)m_w + m_scroll_offset * m_per_character_size <=
        -1.0f * (float)m_element->getAbsolutePosition().getWidth();
}

// ----------------------------------------------------------------------------

void LabelWidget::setScrollSpeed(float speed)
{
    m_scroll_speed  = speed;
}   // setScrollSpeed

// ----------------------------------------------------------------------------

void LabelWidget::resize()
{
    m_per_character_size = getCurrentFont()->getDimension(L"X").Height;
    if (m_scroll_speed != 0)
    {
        rect<s32> widget_size = rect<s32>(m_x, m_y, m_x + m_w, m_y + m_h);
        core::rect<s32> r(core::position2di(0, 0), widget_size.getSize());
        m_element->getParent()->setRelativePosition(widget_size);
        m_element->setRelativePosition(r);
        updateExpandedText(m_text);
    }
    else
        Widget::resize();
}   // resize

// ----------------------------------------------------------------------------

void LabelWidget::setColor(const irr::video::SColor& color)
{
    assert(m_element != NULL);
    m_color = color;
    m_has_color = true;
    ((IGUIStaticText*)m_element)->setOverrideColor(m_color);
}
// ----------------------------------------------------------------------------

void LabelWidget::setErrorColor()
{
    setColor(irr::video::SColor(255, 255, 0, 0));
}

// ----------------------------------------------------------------------------

void LabelWidget::setDefaultColor()
{
    if (m_type == BRIGHT)
    {
        setColor(Skin::getColor("brighttext::neutral"));
    }
    else
    {
        if(m_has_color)
        {
            assert(m_element != NULL);
            m_has_color = false;
            ((IGUIStaticText*)m_element)->enableOverrideColor(false);
        }
    }
}
// ----------------------------------------------------------------------------
