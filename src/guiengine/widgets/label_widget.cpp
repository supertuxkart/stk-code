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

#include <IGUIButton.h>
#include <IGUIEnvironment.h>
#include <IGUIStaticText.h>

#include <assert.h>

using namespace GUIEngine;
using namespace irr::core;
using namespace irr::gui;
using namespace irr;

// ----------------------------------------------------------------------------

LabelWidget::LabelWidget(LabelType type) : Widget(WTYPE_LABEL)
{
    m_type             = type;
    m_scroll_speed     = 0;
    m_scroll_progress  = 0;
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
    // Meaningless size. Will be resized later.
    rect<s32> init_rect = rect<s32>(0, 0, 1, 1); 
    
    const bool word_wrap = m_properties[PROP_WORD_WRAP] == "true";
    stringw message = getText();

    EGUI_ALIGNMENT align = EGUIA_UPPERLEFT;
    if      (m_properties[PROP_TEXT_ALIGN] == "center") align = EGUIA_CENTER;
    else if (m_properties[PROP_TEXT_ALIGN] == "right")  align = EGUIA_LOWERRIGHT;

    EGUI_ALIGNMENT valign = EGUIA_CENTER ;
    if (m_properties[PROP_TEXT_VALIGN] == "top") valign = EGUIA_UPPERLEFT;
    if (m_properties[PROP_TEXT_VALIGN] == "bottom") valign = EGUIA_LOWERRIGHT;

    IGUIStaticText* irrwidget;
    irrwidget = GUIEngine::getGUIEnv()->addStaticText(message.c_str(), init_rect,
                                                      false, word_wrap, m_parent, -1);

    irrwidget->setTextRestrainedInside(m_scroll_speed != 0);
    irrwidget->setMouseCallback(Online::LinkHelper::openURLIrrElement);
    irrwidget->setTextAlignment( align, valign );

    if (m_has_color)
    {
        irrwidget->setOverrideColor(m_color);
    }

    m_element = irrwidget;

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
    //irrwidget->setBackgroundColor( video::SColor(255,255,0,0) );
    //irrwidget->setDrawBackground(true);

    m_id = m_element->getID();

    m_element->setTabStop(false);
    m_element->setTabGroup(false);
    m_element->setNotClipped(m_scroll_speed <= 0);

    if (!m_is_visible)
        m_element->setVisible(false);
    
    resize();
}   // add

// ----------------------------------------------------------------------------

void LabelWidget::resize()
{
    assert(m_element);

    int fwidth = 0;
    stringw message = getText();

    if(m_type == TITLE)
        fwidth = GUIEngine::getTitleFont()->getDimension(message.c_str()).Width;
    else if(m_type == SMALL_TITLE)
        fwidth = GUIEngine::getSmallTitleFont()->getDimension(message.c_str()).Width;
    else if(m_type == TINY_TITLE)
        fwidth = GUIEngine::getTinyTitleFont()->getDimension(message.c_str()).Width;
    else 
        fwidth = GUIEngine::getFont()->getDimension(message.c_str()).Width;

    int offset = 0, real_width = fwidth;
    if (!m_expand_if_needed || fwidth < m_w)
    {
        real_width = m_w;
    }
    if (m_scroll_speed > 0)
    {
        offset = -m_scroll_progress * (real_width + fwidth) + real_width;
    }

    core::rect<s32> rect = core::rect < s32 > (m_x + offset, m_y, m_x + real_width, m_y + m_h);

    m_element->setRelativePosition(rect);
}

// ----------------------------------------------------------------------------

void LabelWidget::setText(const core::stringw& text, bool expandIfNeeded)
{
    m_scroll_progress = 0.0f;
    m_expand_if_needed = expandIfNeeded;

    Widget::setText(text);
}   // setText

// ----------------------------------------------------------------------------
/** Needs to be called to update scrolling.
 *  \param dt Time step size.
 */
void LabelWidget::update(float dt)
{
    if (m_scroll_speed != 0)
    {
        m_scroll_progress += dt * m_scroll_speed;
        resize();
    }
}   // update
// ----------------------------------------------------------------------------

bool LabelWidget::scrolledOff() const
{
    // This method may only be called after this widget has been add()ed
    assert(m_element != NULL);
    return m_scroll_progress >= 1.0f;
}

// ----------------------------------------------------------------------------

void LabelWidget::setScrollSpeed(float speed)
{
    m_scroll_speed = speed;
}   // setScrollSpeed

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
