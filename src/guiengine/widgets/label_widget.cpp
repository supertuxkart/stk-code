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
#include "utils/translation.hpp"

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

LabelWidget::LabelWidget(bool title, bool bright) : Widget(WTYPE_LABEL)
{
    m_title_font   = title;
    m_scroll_speed = 0;
    m_scroll_offset = 0;
    m_bright = bright;

    if (m_bright)
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
    EGUI_ALIGNMENT valign = EGUIA_CENTER ; //TODO: make label v-align configurable through XML file?

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
    irrwidget->setRightToLeft(translations->isRTLText(message));

    m_element = irrwidget;
    irrwidget->setTextAlignment( align, valign );

    if (m_has_color)
    {
        irrwidget->setOverrideColor(m_color);
    }

    if (m_title_font)
    {
        irrwidget->setOverrideColor( video::SColor(255,255,255,255) );
        irrwidget->setOverrideFont( GUIEngine::getTitleFont() );
    }
    //irrwidget->setBackgroundColor( video::SColor(255,255,0,0) );
    //irrwidget->setDrawBackground(true);

    m_id = m_element->getID();

    m_element->setTabStop(false);
    m_element->setTabGroup(false);

    if (m_scroll_speed <= 0)
        m_element->setNotClipped(true);
}   // add

// ----------------------------------------------------------------------------

void LabelWidget::setText(const wchar_t *text, bool expandIfNeeded)
{
    m_scroll_offset = 0;

    if (expandIfNeeded)
    {
        assert(m_element != NULL);
        const int fwidth = (m_title_font ? GUIEngine::getTitleFont() : GUIEngine::getFont())->getDimension(text).Width;
        core::rect<s32> rect = m_element->getRelativePosition();

        if (rect.getWidth() < fwidth)
        {
            rect.LowerRightCorner.X = rect.UpperLeftCorner.X + fwidth;
            m_element->setRelativePosition(rect);
            m_element->updateAbsolutePosition();
        }
    }

    if (m_scroll_speed > 0)
        m_scroll_offset = (float)m_w;

    Widget::setText(text);
    if (m_element)
        getIrrlichtElement<IGUIStaticText>()->setRightToLeft(translations->isRTLText(getText()));
}   // setText

// ----------------------------------------------------------------------------
/** Needs to be called to update scrolling.
 *  \param dt Time step size.
 */
void LabelWidget::update(float dt)
{
    if (m_scroll_speed != 0)
    {
        m_scroll_offset -= dt*m_scroll_speed*5.0f;
        m_element->setRelativePosition( core::position2di( /*m_x +*/ (int)m_scroll_offset,
                                                           /*m_y*/ 0 ) );
    }
}   // update
// ----------------------------------------------------------------------------

bool LabelWidget::scrolledOff() const
{
    // This method may only be called after this widget has been add()ed
    assert(m_element != NULL);
    return m_scroll_offset <= -m_element->getAbsolutePosition().getWidth();
}

// ----------------------------------------------------------------------------

void LabelWidget::setScrollSpeed(float speed)
{
    m_scroll_speed  = speed;
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
    if (m_bright)
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
