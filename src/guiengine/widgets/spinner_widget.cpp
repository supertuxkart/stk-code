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

#include <iostream>

#include "graphics/irr_driver.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "io/file_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"
using namespace GUIEngine;
using namespace irr::core;
using namespace irr::gui;
using namespace irr::video;

// -----------------------------------------------------------------------------

SpinnerWidget::SpinnerWidget(const bool gauge) : Widget(WTYPE_SPINNER)
{
    m_gauge = gauge;
    
    m_listener = NULL;
    
    m_check_inside_me = true; //FIXME: not sure this is necessary
    m_supports_multiplayer = true;
    
    m_min = 0;
    m_max = 999;
}

// -----------------------------------------------------------------------------

void SpinnerWidget::add()
{
    // retrieve min and max values
    std::string min_s = m_properties[PROP_MIN_VALUE];
    std::string max_s = m_properties[PROP_MAX_VALUE];
    
    m_warp_around = (m_properties[PROP_WARP_AROUND] == "true");
    
    if (min_s.size() > 0)
    {
        int i;
        std::istringstream myStream(min_s);
        bool is_number = (myStream >> i)!=0;
        if (is_number)
        {
            m_min = i;
        }
        else
        {
            std::cerr << "WARNING : invalid value for spinner widget minimum value : '" << min_s << "'\n";
        }
    }
    
    if (max_s.size() > 0)
    {
        int i;
        std::istringstream myStream(max_s);
        bool is_number = (myStream >> i)!=0;
        if (is_number)
        {
            m_max = i;
        }
        else
        {
            std::cerr << "WARNING : invalid value for spinner widget maximal value : '" << max_s << "'\n";
        }
    }
    
    m_value = (m_min + m_max)/2;
    
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
    
    rect<s32> widget_size = rect<s32>(m_x, m_y, m_x + m_w, m_y + m_h);
    IGUIButton * btn = GUIEngine::getGUIEnv()->addButton(widget_size, m_parent, widgetID, L"");
    m_element = btn;
    
    m_element->setTabOrder( m_element->getID() );
    
    // left arrow
    rect<s32> subsize_left_arrow = rect<s32>(0 ,0, m_h, m_h);
    IGUIButton * left_arrow = GUIEngine::getGUIEnv()->addButton(subsize_left_arrow, btn, getNewNoFocusID(), L" ");
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
        
        const int texture_width = texture->getSize().Width;
        const int free_h_space = m_w - m_h*2 - texture_width; // to center image
        
        rect<s32> subsize_label = rect<s32>(m_h + free_h_space/2, 0, m_w - m_h+ free_h_space/2, m_h);
        IGUIImage * subbtn = GUIEngine::getGUIEnv()->addImage(subsize_label, btn, getNewNoFocusID());
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
        rect<s32> subsize_label = rect<s32>(m_h, 0, m_w - m_h, m_h);
        IGUIStaticText* label = GUIEngine::getGUIEnv()->addStaticText(stringw(m_value).c_str(), subsize_label,
                                                                      false /* border */, true /* word wrap */,
                                                                      btn, getNewNoFocusID());
        m_children[1].m_element = label;
        m_children[1].m_event_handler = this;
        m_children[1].m_id = label->getID();
        m_children[1].m_properties[PROP_ID] = "spinnerbody";
        label->setTextAlignment(EGUIA_CENTER, EGUIA_CENTER);
        label->setTabStop(false);
        label->setNotClipped(true);
    }
    
    
    // right arrow
    rect<s32> subsize_right_arrow = rect<s32>(m_w - m_h, 0, m_w, m_h);
    IGUIButton * right_arrow = GUIEngine::getGUIEnv()->addButton(subsize_right_arrow, btn, getNewNoFocusID(), L"  ");
    right_arrow->setTabStop(false);
    m_children[2].m_element = right_arrow;
    m_children[2].m_event_handler = this;
    m_children[2].m_properties[PROP_ID] = "right";
    m_children[2].m_id = m_children[2].m_element->getID();
    
    // refresh display
    setValue(m_value);
}

// -----------------------------------------------------------------------------

ITexture* SpinnerWidget::getTexture()
{
    assert(m_graphical);
    std::ostringstream icon_stream;
    icon_stream << file_manager->getDataDir() << "/" << m_properties[PROP_ICON];
    std::string imagefile = StringUtils::insertValues(icon_stream.str(), m_value);
    ITexture* texture = irr_driver->getTexture(imagefile);
    return texture;
}

// -----------------------------------------------------------------------------

void SpinnerWidget::move(const int x, const int y, const int w, const int h)
{
    Widget::move(x, y, w, h);
    
    rect<s32> subsize_left_arrow = rect<s32>(0 ,0, h, h);
    m_children[0].m_element->setRelativePosition(subsize_left_arrow);
    
    if (m_graphical)
    {
        ITexture* texture = getTexture();
        assert(texture != NULL);
        
        const int texture_width = texture->getSize().Width;
        const int free_h_space = w-h*2-texture_width; // to center image
        
        rect<s32> subsize_label = rect<s32>(h+free_h_space/2, 0, w-h+free_h_space/2, h);
        m_children[1].m_element->setRelativePosition(subsize_label);
    }
    else
    {
        rect<s32> subsize_label = rect<s32>(h, 0, w-h, h);
        m_children[1].m_element->setRelativePosition(subsize_label);
    }
    
    rect<s32> subsize_right_arrow = rect<s32>(w-h, 0, w, h);
    m_children[2].m_element->setRelativePosition(subsize_right_arrow);
}

// -----------------------------------------------------------------------------

EventPropagation SpinnerWidget::rightPressed(const int playerID)
{
    // if widget is deactivated, do nothing
    if (m_deactivated) return EVENT_BLOCK;
    
    //std::cout  << "Right pressed\n";
    if (m_value+1 <= m_max)
    {
        setValue(m_value+1);
    }
    else if (m_warp_around)
    {
        setValue(m_min);
    }
    
    //GUIEngine::transmitEvent( this, m_properties[PROP_ID], playerID );
    
    return EVENT_LET;
}

// -----------------------------------------------------------------------------

EventPropagation SpinnerWidget::leftPressed(const int playerID)
{
    // if widget is deactivated, do nothing
    if (m_deactivated) return EVENT_BLOCK;
    
    //std::cout  << "Left pressed\n";
    if (m_value-1 >= m_min)
    {
        setValue(m_value-1);
    }
    else if (m_warp_around)
    {
        setValue(m_max);
    }
    
    //GUIEngine::transmitEvent( this, m_properties[PROP_ID], playerID );
    
    return EVENT_LET;
}

// -----------------------------------------------------------------------------

EventPropagation SpinnerWidget::transmitEvent(Widget* w, std::string& originator, const int playerID)
{
    // if widget is deactivated, do nothing
    if (m_deactivated) return EVENT_BLOCK;
        
    if (originator == "left")
    {
        leftPressed(playerID);
    }
    else if (originator == "right")
    {
        rightPressed(playerID);
    }
    else if (originator == "spinnerbody" || originator == m_properties[PROP_ID])
    {
        if (m_listener != NULL)
        {
            m_listener->onSpinnerConfirmed();
        }
    }
    
    
    this->setFocusForPlayer( playerID );
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
    m_max = m_labels.size()-1;
    
    if (m_element != NULL) setValue(0);
}

// -----------------------------------------------------------------------------

void SpinnerWidget::setValue(const int new_value)
{
    m_value = new_value;
    
    if (m_graphical)
    {
        std::ostringstream icon;
        icon << file_manager->getDataDir() << "/"  << m_properties[PROP_ICON];
        std::string imagefile = StringUtils::insertValues(icon.str(), m_value);
        ((IGUIImage*)(m_children[1].m_element))->setImage(irr_driver->getTexture(imagefile));
    }
    else if (m_labels.size() > 0)
    {
        assert(new_value >= 0);
        assert(new_value < (int)m_labels.size());
        m_children[1].m_element->setText(m_labels[new_value].c_str() );
    }
    else if (m_text.size() > 0)
    {
        stringw text = StringUtils::insertValues(m_text.c_str(), m_value);
        m_children[1].m_element->setText( text.c_str() );
    }
    else
    {
        m_children[1].m_element->setText( stringw(m_value).c_str() );
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
    else
    {
        assert(false);
    }  
    /** To avoid compiler warnings about missing return statements. */
    return "";
}

// -----------------------------------------------------------------------------

void SpinnerWidget::setValue(irr::core::stringw new_value)
{
    const int size = m_labels.size();
    for (int n=0; n<size; n++)
    {
        if (m_labels[n] == new_value)
        {
            setValue(n);
            return;
        }
    }
    
    std::cerr << "ERROR [SpinnerWidget::setValue] : cannot find element named '"
              <<  irr::core::stringc(new_value.c_str()).c_str() << "'\n";
    assert(false);
}

// -----------------------------------------------------------------------------

