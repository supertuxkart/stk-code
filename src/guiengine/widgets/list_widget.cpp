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

#include "graphics/irr_driver.hpp"
#include "guiengine/CGUISpriteBank.h"
#include "guiengine/engine.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "io/file_manager.hpp"

#include <IGUIElement.h>
#include <IGUIEnvironment.h>
#include <IGUIListBox.h>

#include <sstream>

using namespace GUIEngine;
using namespace irr::core;
using namespace irr::gui;

// -----------------------------------------------------------------------------

ListWidget::ListWidget() : Widget(WTYPE_LIST)
{
    m_use_icons = false;
    m_icons = NULL;
    m_listener = NULL;
    m_selected_column = NULL;
}

// -----------------------------------------------------------------------------

void ListWidget::setIcons(STKModifiedSpriteBank* icons, int size)
{
    m_use_icons = (icons != NULL);
    m_icons = icons;
    
    if (m_use_icons)
    {
        IGUIListBox* list = getIrrlichtElement<IGUIListBox>();
        assert(list != NULL);
        
        list->setSpriteBank(m_icons);
        
        // determine needed height
        int item_height = 0;
        if (size > 0)
        {
            item_height = size;
        }
        else
        {
            const core::array< core::rect<s32> >& rects = m_icons->getPositions();
            const int count = rects.size();
            for (int n=0; n<count; n++)
            {
                const int h = rects[n].getHeight();
                if (h > item_height) item_height = h;
            }
        }
        
        if (item_height > 0)
        {
            list->setItemHeight( item_height );
        }
    }
    
}

// -----------------------------------------------------------------------------

void ListWidget::add()
{
    const int header_height = GUIEngine::getFontHeight() + 15;
    
    rect<s32> widget_size = (m_header.size() > 0 ? rect<s32>(m_x, m_y + header_height, m_x + m_w, m_y + m_h) :
                                                   rect<s32>(m_x, m_y, m_x + m_w, m_y + m_h) );
    
    IGUIListBox* list = GUIEngine::getGUIEnv()->addListBox (widget_size, m_parent, getNewID());
    list->setAutoScrollEnabled(false);
    
    m_element = list;
    m_element->setTabOrder( list->getID() );
    
    if (m_header.size() > 0)
    {
        //const int col_size = m_w / m_header.size();
        
        int proportion_total = 0;
        for (unsigned int n=0; n<m_header.size(); n++)
        {
            proportion_total += m_header[n].m_proportion;
        }
        
        int x = m_x;
        for (unsigned int n=0; n<m_header.size(); n++)
        {
            std::ostringstream name;
            name << m_properties[PROP_ID];
            name << "_column_";
            name << n;
            
            ButtonWidget* header = new ButtonWidget();
            
            header->m_reserved_id = getNewNoFocusID();
            
            header->m_y = m_y;
            header->m_h = header_height;
            
            header->m_x = x;
            header->m_w = (int)(m_w * float(m_header[n].m_proportion)
                                /float(proportion_total));
            
            x += header->m_w;
            
            header->setText( m_header[n].m_text );
            header->m_properties[PROP_ID] = name.str();
            
            header->add();
            header->m_event_handler = this;
            
            header->getIrrlichtElement()->setTabStop(false);
            
            m_children.push_back(header);
            m_header_elements.push_back(header);
        }
        
        m_check_inside_me = true;
    }
}

// -----------------------------------------------------------------------------

void ListWidget::clear()
{
    // May only be called AFTER this widget has been add()ed
    assert(m_element != NULL);
    
    IGUIListBox* list = getIrrlichtElement<IGUIListBox>();
    assert(list != NULL);
    
    list->clear();
    m_items.clear();
}

// -----------------------------------------------------------------------------

void ListWidget::addItem(const std::string& internalName, 
                         const irr::core::stringw& name, const int icon)
{
    // May only be called AFTER this widget has been add()ed
    assert(m_element != NULL);
    
    ListItem newItem;
    newItem.m_label = name;
    newItem.m_internal_name = internalName;
    
    IGUIListBox* list = getIrrlichtElement<IGUIListBox>();
    assert(list != NULL);
    
    if (m_use_icons && icon != -1)
    {
        u32 itemID = list->addItem( name.c_str(), icon );
        list->setItemOverrideColor( itemID, gui::EGUI_LBC_ICON, video::SColor(255,255,255,255) );
        list->setItemOverrideColor( itemID, gui::EGUI_LBC_ICON_HIGHLIGHT, video::SColor(255,255,255,255) );
        newItem.m_current_id = itemID;
    }
    else
    {
        newItem.m_current_id = list->addItem( name.c_str() );
    }
    m_items.push_back(newItem);
}

// -----------------------------------------------------------------------------

void ListWidget::renameItem(const int itemID, const irr::core::stringw newName, const int icon)
{
    // May only be called AFTER this widget has been add()ed
    assert(m_element != NULL);
    
    IGUIListBox* list = getIrrlichtElement<IGUIListBox>();
    assert(list != NULL);
    
    m_items[itemID].m_label = newName;
    list->setItem(itemID, newName.c_str(), icon);
    
    list->setItemOverrideColor( itemID, EGUI_LBC_TEXT          , video::SColor(255,0,0,0) );
    list->setItemOverrideColor( itemID, EGUI_LBC_TEXT_HIGHLIGHT, video::SColor(255,255,255,255) );
}

// -----------------------------------------------------------------------------

std::string ListWidget::getSelectionInternalName()
{
    if (getSelectionID() == -1) return "";
    return m_items[ getSelectionID() ].m_internal_name;

}

// -----------------------------------------------------------------------------

irr::core::stringw ListWidget::getSelectionLabel() const
{
    const IGUIListBox* list = getIrrlichtElement<IGUIListBox>();
    assert(list != NULL);
    return list->getListItem( list->getSelected() );
}

// -----------------------------------------------------------------------------

void ListWidget::selectItemWithLabel(const irr::core::stringw& name)
{
    IGUIListBox* list = getIrrlichtElement<IGUIListBox>();
    assert(list != NULL);
    return list->setSelected( name.c_str() );
}

// -----------------------------------------------------------------------------

void ListWidget::unfocused(const int playerID)
{
    IGUIListBox* list = getIrrlichtElement<IGUIListBox>();

    // remove selection when leaving list
    if (list != NULL) list->setSelected(-1);
}

// -----------------------------------------------------------------------------

int ListWidget::getSelectionID() const
{
    // May only be called AFTER this widget has been add()ed
    assert(m_element != NULL);
    
    return getIrrlichtElement<IGUIListBox>()->getSelected();
}

// -----------------------------------------------------------------------------

void ListWidget::setSelectionID(const int index)
{
    // May only be called AFTER this widget has been add()ed
    assert(m_element != NULL);
    
    IGUIListBox* irritem = getIrrlichtElement<IGUIListBox>();
    
    // auto-scroll to item when selecting something, don't auto-scroll when selecting nothing
    if (index != -1)
    {
        irritem->setAutoScrollEnabled(true);
    }
    
    irritem->setSelected(index);
    
    if (index != -1)
    {
        irritem->setAutoScrollEnabled(false);
    }
}

// -----------------------------------------------------------------------------

int ListWidget::getItemCount() const
{
    // May only be called AFTER this widget has been add()ed
    assert(m_element != NULL);
    
    const int count = getIrrlichtElement<IGUIListBox>()->getItemCount();
    assert((int)m_items.size() == count);
    
    return count;
}

// -----------------------------------------------------------------------------

void ListWidget::elementRemoved()
{
    Widget::elementRemoved();
    m_items.clear();
    
    for (int n=0; n<m_header_elements.size(); n++)
    {
        m_header_elements[n].elementRemoved();
        m_children.remove( m_header_elements.get(n) );
    }
    m_header_elements.clearAndDeleteAll();
    m_selected_column = NULL;
}

// -----------------------------------------------------------------------------

int ListWidget::getItemID(const std::string internalName) const
{
    const int count = m_items.size();
    
    for (int i=0; i<count; i++)
    {
        if (m_items[i].m_internal_name == internalName) return i;
    }
    
    return -1;
}

// -----------------------------------------------------------------------------

void ListWidget::markItemRed(const int id, bool red)
{
    // May only be called AFTER this widget has been add()ed
    assert(m_element != NULL);
    
    IGUIListBox* irritem = getIrrlichtElement<IGUIListBox>();
    
    if (red)
    {
        irritem->setItemOverrideColor( id, EGUI_LBC_TEXT,           video::SColor(255,255,0,0) );
        irritem->setItemOverrideColor( id, EGUI_LBC_TEXT_HIGHLIGHT, video::SColor(255,255,0,0) );
    }
    else
    {
        irritem->setItemOverrideColor( id, EGUI_LBC_TEXT,           video::SColor(255,0,0,0) );
        irritem->setItemOverrideColor( id, EGUI_LBC_TEXT_HIGHLIGHT, video::SColor(255,255,255,255) );
    }
}

// -----------------------------------------------------------------------------

EventPropagation ListWidget::transmitEvent(Widget* w, std::string& originator, const int playerID)
{
    if (originator.find(m_properties[PROP_ID] + "_column_") != std::string::npos)
    {        
        int col = originator[ (m_properties[PROP_ID] + "_column_").size() ] - '0';

        m_selected_column = m_header_elements.get(col);
        /*
        for (int n=0; n<m_header_elements.size(); n++)
        {
            m_header_elements[n].getIrrlichtElement<IGUIButton>()->setPressed(false);
        }
        m_header_elements[col].getIrrlichtElement<IGUIButton>()->setPressed(true);
        */
        
        if (m_listener) m_listener->onColumnClicked(col);
        
        return EVENT_BLOCK;
    }
    
    return EVENT_LET;
}
