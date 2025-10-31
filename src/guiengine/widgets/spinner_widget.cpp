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

#include <iostream>

#include "graphics/irr_driver.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "io/file_manager.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/log.hpp"

#include <IGUIElement.h>
#include <IGUIEnvironment.h>
#include <IGUIButton.h>
#include <IGUIImage.h>

#include "../../../lib/irrlicht/source/Irrlicht/CGUIButton.h"
#include <IGUIStaticText.h>
#include <ITexture.h>

using namespace GUIEngine;
using namespace irr::core;
using namespace irr::gui;
using namespace irr::video;

// -----------------------------------------------------------------------------

class SpinnerIrrElement : public CGUIButton
{
private:
    bool m_pressed_moving;

    SpinnerWidget* m_spinner;
public:
    SpinnerIrrElement(const core::rect<s32>& rectangle, IGUIElement* parent,
                      s32 id, SpinnerWidget* spinner)
    : CGUIButton(GUIEngine::getGUIEnv(), parent ?
                 parent : dynamic_cast<IGUIElement*>(GUIEngine::getGUIEnv()),
                 id, rectangle)
    {
        m_pressed_moving = false;
        m_spinner = spinner;
        setText(L"");
        // Parent or GUI environment grabbed it
        drop();
    }
    virtual bool OnEvent(const SEvent& event)
    {
        bool ret = CGUIButton::OnEvent(event);
        if (ret && event.EventType == EET_MOUSE_INPUT_EVENT)
        {
            if (!m_spinner->isActivated())
                return ret;
            if (event.MouseInput.Event == EMIE_LMOUSE_PRESSED_DOWN)
                m_pressed_moving = true;
            else if (event.MouseInput.Event == EMIE_LMOUSE_LEFT_UP)
            {
                m_pressed_moving = false;
                // clearSelected so SpinnerWidget::activateSelectedButton
                // is not called in transmitEvent
                m_spinner->clearSelected();
            }
            if (m_pressed_moving &&
                (event.MouseInput.Event == EMIE_LMOUSE_PRESSED_DOWN ||
                event.MouseInput.Event == EMIE_MOUSE_MOVED))
            {
                m_spinner->onPressed(event.MouseInput.X, event.MouseInput.Y);
                m_spinner->doValueUpdatedCallback();
            }
        }
        return ret;
    }
};

SpinnerWidget::SpinnerWidget(const bool gauge) : Widget(WTYPE_SPINNER)
{
    m_gauge = gauge;
    m_listener = NULL;
    m_graphical = false;
    m_check_inside_me = true; //FIXME: not sure this is necessary
    m_supports_multiplayer = true;
    m_value = -1;
    m_badge_x_shift = 0;
    m_use_background_color=false;
    m_spinner_widget_player_id=PLAYER_ID_GAME_MASTER;
    m_min = 0;
    m_max = 999;
    m_step = 1.0;
    m_left_selected = false;
    m_right_selected = false;
    m_incorrect       = false;
    m_red_mark_widget = NULL;
    m_left_arrow = rect<s32>(0, 0, m_h, m_h);
}

// ------------------------------------------------------------------------
/** Add a red mark on the spinner to mean "invalid choice" */
void SpinnerWidget::markAsIncorrect()
{
    if (m_incorrect) return; // already flagged as incorrect

    m_incorrect = true;

    irr::video::ITexture* texture = irr_driver->getTexture(FileManager::GUI_ICON,
                                                           "red_mark.png"   );
    const int mark_size = m_h * 4 / 5;
    const int mark_x = m_w - m_h * 19 / 10;
    const int mark_y = m_h / 10;
    core::recti red_mark_area(mark_x, mark_y, mark_x + mark_size,
                              mark_y + mark_size);
    m_red_mark_widget = GUIEngine::getGUIEnv()->addImage( red_mark_area,
                        /* parent */ m_element );
    m_red_mark_widget->setImage(texture);
    m_red_mark_widget->setScaleImage(true);
    m_red_mark_widget->setTabStop(false);
    m_red_mark_widget->setUseAlphaChannel(true);
} // markAsIncorrect

// ------------------------------------------------------------------------
/** Remove any red mark set with 'markAsIncorrect' */
void SpinnerWidget::markAsCorrect()
{
    if (m_incorrect)
    {
        m_red_mark_widget->remove();
        m_red_mark_widget = NULL;
        m_incorrect = false;
    }
} // markAsCorrect

// -----------------------------------------------------------------------------
void SpinnerWidget::setRange(float min, float max, float step)
{
    clearLabels();
    setStep(step);
    setMin(min/step);
    setMax(max/step);
}

// -----------------------------------------------------------------------------

void SpinnerWidget::add()
{
    // retrieve min and max values
    std::string min_s = m_properties[PROP_MIN_VALUE];
    std::string max_s = m_properties[PROP_MAX_VALUE];

    m_wrap_around = (m_properties[PROP_WRAP_AROUND] == "true");
    m_color_slider = (m_properties[PROP_COLOR_SLIDER] == "true");

    if (min_s.size() > 0)
    {
        if (!StringUtils::parseString<int>(min_s, &m_min))
        {
            Log::warn("invalid value for spinner widget minimum value : %s", min_s.c_str());
        }
    }

    if (max_s.size() > 0)
    {
        if (!StringUtils::parseString<int>(max_s, &m_max))
        {
            Log::warn("invalid value for spinner widget maximum value : %s", max_s.c_str());
        }
    }

    if (m_value == -1)
    {
        m_value = (m_min + m_max)/2;
    }

    // create sub-widgets if they don't already exist
    if (m_children.size() == 0)
    {
        std::string& icon = m_properties[PROP_ICON];
        m_graphical = icon.size()>0;

        //FIXME: unclean to create "fake" button/label/icon widgets!!
        m_children.push_back( new Widget(WTYPE_BUTTON) );
        m_children.push_back( new Widget(m_graphical ? WTYPE_ICON_BUTTON : WTYPE_LABEL) );
        m_children.push_back( new Widget(WTYPE_BUTTON) );
    }

    int widgetID;

    if (m_reserved_id != -1)
    {
        widgetID = m_reserved_id;
    }
    else
    {
        widgetID = getNewID();
    }

    // To be resized later
    rect<s32> widget_size = rect<s32>(0, 0, 1, 1);
    IGUIButton * btn = new SpinnerIrrElement(widget_size, m_parent, widgetID, this);
    m_element = btn;

    m_element->setTabOrder( m_element->getID() );

    // left arrow
    IGUIButton * left_arrow = GUIEngine::getGUIEnv()->addButton(widget_size, btn, getNewNoFocusID(), L" ");
    m_children[0].m_element = left_arrow;
    left_arrow->setTabStop(false);
    m_children[0].m_event_handler = this;
    m_children[0].m_properties[PROP_ID] = "left";
    m_children[0].m_id = m_children[0].m_element->getID();

    // label
    if (m_graphical)
    {
        ITexture* texture = getTexture();
        assert(texture != NULL);

        IGUIImage * subbtn = GUIEngine::getGUIEnv()->addImage(widget_size, btn, getNewNoFocusID());
        m_children[1].m_element = subbtn;
        m_children[1].m_id = subbtn->getID();
        m_children[1].m_event_handler = this;
        m_children[1].m_properties[PROP_ID] = "spinnerbody";
        subbtn->setUseAlphaChannel(true);

        subbtn->setImage(texture);
        //subbtn->setScaleImage(true);
    }
    else
    {
        stringw text = stringw(m_value);
        IGUIStaticText* label = GUIEngine::getGUIEnv()->addStaticText(text.c_str(), widget_size,
                                                                      false /* border */, true /* word wrap */,
                                                                      btn, getNewNoFocusID());
        m_children[1].m_element = label;
        m_children[1].m_event_handler = this;
        m_children[1].m_id = label->getID();
        m_children[1].m_properties[PROP_ID] = "spinnerbody";
        label->setTextAlignment(EGUIA_CENTER, EGUIA_CENTER);
        label->setTabStop(false);
        label->setNotClipped(true);

        if (m_labels.size() > 0)
        {
            label->setText(m_labels[m_value].c_str() );
        }
    }


    // right arrow
    IGUIButton * right_arrow = GUIEngine::getGUIEnv()->addButton(widget_size, btn, getNewNoFocusID(), L"  ");
    right_arrow->setTabStop(false);
    m_children[2].m_element = right_arrow;
    m_children[2].m_event_handler = this;
    m_children[2].m_properties[PROP_ID] = "right";
    m_children[2].m_id = m_children[2].m_element->getID();

    // refresh display
    resize();
    setValue(m_value);
}
// -----------------------------------------------------------------------------

ITexture* SpinnerWidget::getTexture()
{
    assert(m_graphical);
    std::string s = StringUtils::insertValues(m_properties[PROP_ICON], m_value);
    std::string imagefile = file_manager->searchTexture(s);
    ITexture* texture = irr_driver->getTexture(imagefile);
    return texture;
}

// -----------------------------------------------------------------------------

void SpinnerWidget::resize()
{
    Widget::resize();

    rect<s32> subsize_left_arrow = rect<s32>(0 ,0, m_h, m_h);
    m_left_arrow = subsize_left_arrow;
    m_children[0].m_element->setRelativePosition(subsize_left_arrow);
    m_badge_x_shift = subsize_left_arrow.getWidth();

    if (m_graphical)
    {
        ITexture* texture = getTexture();
        assert(texture != NULL);

        const int texture_width = texture->getSize().Width;
        const int free_h_space = m_w-m_h*2-texture_width; // to center image

        rect<s32> subsize_label = rect<s32>(m_h+free_h_space/2, 0, m_w-m_h+free_h_space/2, m_h);
        m_children[1].m_element->setRelativePosition(subsize_label);
    }
    else
    {
        resizeLabel();
    }

    rect<s32> subsize_right_arrow = rect<s32>(m_w-m_h, 0, m_w, m_h);
    m_children[2].m_element->setRelativePosition(subsize_right_arrow);
} // resize

// -----------------------------------------------------------------------------

/** Pick the appropriate font size to display the current spinner label */
void SpinnerWidget::resizeLabel()
{
    if (m_graphical) // Don't proceed further if this spinner doesn't use labels
        return;

    rect<s32> subsize_label = rect<s32>(m_h, 0, m_w - m_h, m_h);
    IGUIStaticText* label = static_cast<IGUIStaticText*>(m_children[1].m_element);
    label->setRelativePosition(subsize_label);
    if ( (m_h < GUIEngine::getFontHeight()) ||
         ((int)GUIEngine::getFont()->getDimension(label->getText()).Width > (m_w - 2 * m_h)) )
    {
        label->setOverrideFont(GUIEngine::getSmallFont());
    }
    else
    {
        label->setOverrideFont(NULL);
    }
} // resizeLabel

// -----------------------------------------------------------------------------

EventPropagation SpinnerWidget::rightPressed(const int playerID)
{
    //Log::info("SpinnerWidget", "Right pressed");

    // if widget is deactivated, do nothing
    if (m_deactivated) return EVENT_BLOCK;

    // if right arrow is selected, let event handler move to next widget
    if (m_right_selected)
        return EVENT_BLOCK;
    else
        setSelectedButton(/* right*/ true);

    return EVENT_BLOCK_BUT_HANDLED;
} // rightPressed

// -----------------------------------------------------------------------------

EventPropagation SpinnerWidget::leftPressed(const int playerID)
{
    //Log::info("SpinnerWidget", "Left pressed");

    // if widget is deactivated, do nothing
    if (m_deactivated) return EVENT_BLOCK;

    // if left arrow is selected, let event handler move to next widget
    if (m_left_selected)
        return EVENT_BLOCK;
    else
        setSelectedButton(/* right*/ false);

    return EVENT_BLOCK_BUT_HANDLED;
} // leftPressed

void SpinnerWidget::activateSelectedButton()
{
    if (m_right_selected)
    {
        if (m_value+1 <= m_max)
        {
            setValue(m_value+1);
        }
        else if (m_wrap_around)
        {
            setValue(m_min);
        }
    }
    else if (m_left_selected)
    {
        if (m_value-1 >= m_min)
        {
            setValue(m_value-1);
        }
        else if (m_wrap_around)
        {
            setValue(m_max);
        }
    }

    // Update the label font size if needed
    resizeLabel();
} // activateSelectedButton

// -----------------------------------------------------------------------------

EventPropagation SpinnerWidget::transmitEvent(Widget* w,
                                              const std::string& originator,
                                              const int playerID)
{
    assert(m_magic_number == 0xCAFEC001);

    // if widget is deactivated, do nothing
    if (m_deactivated) return EVENT_BLOCK;

    if (originator == "left")
    {
        m_left_selected = true;
        m_right_selected = false;
        activateSelectedButton();
    }
    else if (originator == "right")
    {
        m_left_selected = false;
        m_right_selected = true;
        activateSelectedButton();
    }
    else if (originator == "spinnerbody" || originator == m_properties[PROP_ID])
    {
        if (m_listener != NULL)
        {
            if (m_listener->onSpinnerConfirmed() == EVENT_BLOCK)
            {
                return EVENT_BLOCK;
            }
        }
    }


    if (m_element != NULL) setFocusForPlayer( playerID );
    return EVENT_LET;
}

// -----------------------------------------------------------------------------

void SpinnerWidget::clearLabels()
{
    m_labels.clear();
}

// -----------------------------------------------------------------------------

void SpinnerWidget::addLabel(stringw label)
{
    m_labels.push_back(label);
    m_min = 0;
    m_max = (int)m_labels.size()-1;

    if (m_element != NULL) setValue(0);
}

// -----------------------------------------------------------------------------

void SpinnerWidget::setValue(const int new_value)
{
    m_value = new_value;
    m_custom_text = "";

    if (m_graphical)
    {
        std::string s = StringUtils::insertValues(m_properties[PROP_ICON], m_value);
        std::string imagefile = file_manager->searchTexture(s);
        ((IGUIImage*)(m_children[1].m_element))->setImage(irr_driver->getTexture(imagefile));
    }
    else if (m_labels.size() > 0 && m_children.size() > 0)
    {
        assert(new_value >= 0);
        assert(new_value < (int)m_labels.size());

        m_children[1].m_element->setText(m_labels[new_value].c_str());
        resizeLabel();
    }
    else if (m_text.size() > 0 && m_children.size() > 0)
    {
        stringw text = StringUtils::insertValues(m_text.c_str(), m_value);
        m_children[1].m_element->setText( text.c_str() );
    }
    else if (m_children.size() > 0)
    {
        if (m_step == 1.0)
        {
            m_children[1].m_element->setText( stringw(m_value).c_str() );
        }
        else
        {
            std::wstringstream ws;
            ws << (m_value*m_step);
            std::wstring text = ws.str();
            m_children[1].m_element->setText( text.c_str() );
        }
    }
}

// -----------------------------------------------------------------------------

stringw SpinnerWidget::getStringValue() const
{
    if (m_labels.size() > 0)
    {
        return stringw(m_labels[m_value].c_str()).c_str();
    }
    else if (m_text.size() > 0)
    {
        stringw text = StringUtils::insertValues(m_text.c_str(), m_value);
        return text;
    }
    // This can happen if the spinner has not been initialized yet.
    return "";
}

// -----------------------------------------------------------------------------

void SpinnerWidget::setValue(irr::core::stringw new_value)
{
    const int size = (int)m_labels.size();
    for (int n=0; n<size; n++)
    {
        if (m_labels[n] == new_value)
        {
            setValue(n);
            resizeLabel();
            return;
        }
    }

    Log::fatal("SpinnerWidget::setValue", "Cannot find element named '%s'",
        irr::core::stringc(new_value.c_str()).c_str());
}

// -----------------------------------------------------------------------------

void SpinnerWidget::setActive(bool active)
{
    Widget::setActive(active);

    if (active)
    {
        setText(L"");
        if (m_custom_text.empty())
        {
            setValue(getValue()); // Update the display
        }
        else
        {
            setCustomText(m_custom_text);
        }
    }
}   // setActive

// -----------------------------------------------------------------------------
void SpinnerWidget::setCustomText(const core::stringw& text)
{
    m_custom_text = text;
    if (m_children.size() > 0)
    {
        m_children[1].m_element->setText(text.c_str());
    }
}

// -----------------------------------------------------------------------------

void SpinnerWidget::onPressed(int x, int y)
{
    if (m_children[1].m_deactivated || 
        m_children[1].m_properties[PROP_ID] != "spinnerbody"  || 
        !isGauge()) 
    { 
        return;
    }

    core::position2di mouse_position(x, y);
    core::recti body_rect 
        = m_children[1].getIrrlichtElement()->getAbsolutePosition();

    if (body_rect.isPointInside(mouse_position))
    {
        float exact_hover = (float)((mouse_position.X -
            body_rect.UpperLeftCorner.X) /
            (float)body_rect.getWidth()) * (m_max-m_min);

        float new_value_f = ((exact_hover * (m_max - m_min)) /
            (m_max - m_min)) + m_min;
        int new_value = (int)roundf(new_value_f);

        if (new_value > m_max) new_value = m_max;
        if (new_value < m_min) new_value = m_min;

        setValue(new_value);
    }
}

// -----------------------------------------------------------------------------
