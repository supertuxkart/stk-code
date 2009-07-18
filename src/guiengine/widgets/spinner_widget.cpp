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

SpinnerWidget::SpinnerWidget(const bool gauge)
{
    m_gauge = gauge;
    m_type = WTYPE_SPINNER;
    
}

void SpinnerWidget::add()
{
    // retrieve min and max values
    std::string min_s = m_properties[PROP_MIN_VALUE];
    std::string max_s = m_properties[PROP_MAX_VALUE];
    
    {
        int i;
        std::istringstream myStream(min_s);
        bool is_number = (myStream >> i)!=0;
        if(is_number) m_min = i;
        else m_min = 0;
    }
    {
        int i;
        std::istringstream myStream(max_s);
        bool is_number = (myStream >> i)!=0;
        if(is_number) m_max = i;
        else m_max = 10;
    }
    
    m_value = (m_min + m_max)/2;
    
    // create sub-widgets if they don't already exist
    if(m_children.size() == 0)
    {
        std::string& icon = m_properties[PROP_ICON];
        m_graphical = icon.size()>0;
        
        m_children.push_back( new Widget() );
        m_children.push_back( new Widget() );
        m_children.push_back( new Widget() );
    }
    
    rect<s32> widget_size = rect<s32>(x, y, x + w, y + h);
    IGUIButton * btn = GUIEngine::getGUIEnv()->addButton(widget_size, m_parent, getNewID(), L"");
    m_element = btn;
    
    m_element->setTabOrder( m_element->getID() );
    
    // left arrow
    rect<s32> subsize_left_arrow = rect<s32>(0 ,0, h, h);
    IGUIButton * left_arrow = GUIEngine::getGUIEnv()->addButton(subsize_left_arrow, btn, getNewNoFocusID(), L" ");
    m_children[0].m_element = left_arrow;
    m_children[0].m_type = WTYPE_BUTTON;
    left_arrow->setTabStop(false);
    m_children[0].m_event_handler = this;
    m_children[0].m_properties[PROP_ID] = "left";
    m_children[0].id = m_children[0].m_element->getID();
    
    // label
    if(m_graphical)
    {
        std::ostringstream icon_stream;
        icon_stream << file_manager->getDataDir() << "/" << m_properties[PROP_ICON];
        std::string imagefile = StringUtils::insert_values(icon_stream.str(), m_value);
        ITexture* texture = irr_driver->getTexture(imagefile);
        assert(texture != NULL);
        
        const int texture_width = texture->getSize().Width;
        const int free_h_space = w-h*2-texture_width; // to center image
        
        rect<s32> subsize_label = rect<s32>(h+free_h_space/2, 0, w-h+free_h_space/2, h);
        //IGUIButton* subbtn = GUIEngine::getGUIEnv()->addButton(subsize_label, btn, ++id_counter_2, L"");
        IGUIImage * subbtn = GUIEngine::getGUIEnv()->addImage(subsize_label, btn, getNewNoFocusID());
        m_children[1].m_element = subbtn;
        m_children[1].m_type = WTYPE_ICON_BUTTON;
        m_children[1].id = subbtn->getID();
        subbtn->setUseAlphaChannel(true);
        
        subbtn->setImage(texture);
        //subbtn->setScaleImage(true);
    }
    else
    {
        rect<s32> subsize_label = rect<s32>(h, 0, w-h, h);
        IGUIStaticText* label = GUIEngine::getGUIEnv()->addStaticText(stringw(m_value).c_str(), subsize_label,
                                                                      false /* border */, true /* word wrap */,
                                                                      btn, getNewNoFocusID());
        m_children[1].m_element = label;
        m_children[1].m_type = WTYPE_LABEL;
        m_children[1].id = label->getID();
        label->setTextAlignment(EGUIA_CENTER, EGUIA_CENTER);
        label->setTabStop(false);
        label->setNotClipped(true);
    }
    
    
    // right arrow
    rect<s32> subsize_right_arrow = rect<s32>(w-h, 0, w, h);
    IGUIButton * right_arrow = GUIEngine::getGUIEnv()->addButton(subsize_right_arrow, btn, getNewNoFocusID(), L"  ");
    right_arrow->setTabStop(false);
    m_children[2].m_element = right_arrow;
    m_children[2].m_type = WTYPE_BUTTON;
    m_children[2].m_event_handler = this;
    m_children[2].m_properties[PROP_ID] = "right";
    m_children[2].id = m_children[2].m_element->getID();
}
void SpinnerWidget::move(const int x, const int y, const int w, const int h)
{
    Widget::move(x, y, w, h);
    
    rect<s32> subsize_left_arrow = rect<s32>(0 ,0, h, h);
    m_children[0].m_element->setRelativePosition(subsize_left_arrow);
    
    if(m_graphical)
    {
        // FIXME : code duplicated from add()
        std::ostringstream icon_stream;
        icon_stream << file_manager->getDataDir() << "/" << m_properties[PROP_ICON];
        std::string imagefile = StringUtils::insert_values(icon_stream.str(), m_value);
        ITexture* texture = irr_driver->getTexture(imagefile);
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
bool SpinnerWidget::rightPressed()
{
    if(m_value+1 <= m_max) setValue(m_value+1);
    return true;
}
// -----------------------------------------------------------------------------
bool SpinnerWidget::leftPressed()
{
    if(m_value-1 >= m_min) setValue(m_value-1);
    return true;
}
// -----------------------------------------------------------------------------
bool SpinnerWidget::transmitEvent(Widget* w, std::string& originator)
{
    if(originator == "left") leftPressed();
    else if(originator == "right") rightPressed();
    
    GUIEngine::getGUIEnv()->setFocus(m_element);
    return true;
}
// -----------------------------------------------------------------------------
void SpinnerWidget::addLabel(std::string label)
{
    m_labels.push_back(label);
    m_min = 0;
    m_max = m_labels.size()-1;
    setValue(0);
}
// -----------------------------------------------------------------------------
void SpinnerWidget::setValue(const int new_value)
{
    m_value = new_value;
    
    if(m_graphical)
    {
        std::ostringstream icon;
        icon << file_manager->getDataDir() << "/"  << m_properties[PROP_ICON];
        std::string imagefile = StringUtils::insert_values(icon.str(), m_value);
        //((IGUIButton*)(m_children[1].m_element))->setImage(GUIEngine::getDriver()->getTexture(imagefile));
        ((IGUIImage*)(m_children[1].m_element))->setImage(irr_driver->getTexture(imagefile));
    }
    else if(m_labels.size() > 0)
    {
        m_children[1].m_element->setText( stringw(m_labels[new_value].c_str()).c_str() );
    }
    else if(m_properties[PROP_TEXT].size() > 0)
    {
        std::string text = StringUtils::insert_values(_(m_properties[PROP_TEXT].c_str()), m_value);
        m_children[1].m_element->setText( stringw(text.c_str()).c_str() );
    }
    else
    {
        m_children[1].m_element->setText( stringw(m_value).c_str() );
    }
}
