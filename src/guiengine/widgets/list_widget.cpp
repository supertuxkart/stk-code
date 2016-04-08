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

#include "guiengine/widgets/list_widget.hpp"

#include "guiengine/CGUISpriteBank.hpp"
#include "guiengine/engine.hpp"
#include "io/file_manager.hpp"

#include <IGUIElement.h>
#include <IGUISkin.h>
#include <IGUIEnvironment.h>
#include <IGUIFontBitmap.h>

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
    m_sort_desc = false;
    m_sort_default = true;
    m_sort_col = 0;
    m_sortable = true;
}

// -----------------------------------------------------------------------------

void ListWidget::setIcons(STKModifiedSpriteBank* icons, int size)
{
    m_use_icons = (icons != NULL);
    m_icons = icons;

    if (m_use_icons)
    {
        CGUISTKListBox* list = getIrrlichtElement<CGUISTKListBox>();
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

    IGUISkin * current_skin = GUIEngine::getGUIEnv()->getSkin();
    IGUIFont * current_font = GUIEngine::getGUIEnv()->getBuiltInFont();
    CGUISTKListBox * list_box = new CGUISTKListBox(
        GUIEngine::getGUIEnv(),
        m_parent ? m_parent : GUIEngine::getGUIEnv()->getRootGUIElement(),
        getNewID(),
        widget_size,
        true,
        true,
        false);

    if (current_skin && current_skin->getSpriteBank())
    {
            list_box->setSpriteBank(current_skin->getSpriteBank());
    }
    else if (current_font && current_font->getType() == EGFT_BITMAP)
    {
            list_box->setSpriteBank( ((IGUIFontBitmap*)current_font)->getSpriteBank());
    }

    list_box->drop();

    list_box->setAutoScrollEnabled(false);

    m_element = list_box;
    m_element->setTabOrder( list_box->getID() );

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

    CGUISTKListBox* list = getIrrlichtElement<CGUISTKListBox>();
    assert(list != NULL);

    list->clear();
}

// -----------------------------------------------------------------------------

void ListWidget::addItem(   const std::string& internal_name,
                            const irr::core::stringw &name,
                            const int icon,
                            bool center)
{
    // May only be called AFTER this widget has been add()ed
    assert(m_element != NULL);

    ListCell cell(name, icon, 1, center);
    ListItem newItem;
    newItem.m_internal_name = internal_name;
    newItem.m_contents.push_back(cell);

    CGUISTKListBox* list = getIrrlichtElement<CGUISTKListBox>();
    assert(list != NULL);

    u32 itemID = list->addItem( newItem );
    if (m_use_icons)
    {
        list->setItemOverrideColor( itemID, gui::EGUI_LBC_ICON, video::SColor(255,255,255,255) );
        list->setItemOverrideColor( itemID, gui::EGUI_LBC_ICON_HIGHLIGHT, video::SColor(255,255,255,255) );
    }
    newItem.m_current_id = itemID;
}

// -----------------------------------------------------------------------------

void ListWidget::addItem(const std::string& internal_name,
                         const std::vector<ListCell>& contents)
{
    // May only be called AFTER this widget has been add()ed
    assert(m_element != NULL);

    ListItem newItem;
    newItem.m_internal_name = internal_name;
    for (unsigned int i = 0; i < contents.size(); i++)
    {
        newItem.m_contents.push_back(contents[i]);
    }

    CGUISTKListBox* list = getIrrlichtElement<CGUISTKListBox>();
    assert(list != NULL);

    u32 itemID = list->addItem( newItem );
    if (m_use_icons)
    {
        list->setItemOverrideColor( itemID, gui::EGUI_LBC_ICON, video::SColor(255,255,255,255) );
        list->setItemOverrideColor( itemID, gui::EGUI_LBC_ICON_HIGHLIGHT, video::SColor(255,255,255,255) );
    }
    newItem.m_current_id = itemID;
}

// -----------------------------------------------------------------------------
void ListWidget::renameCell(const int row_index, const int col_index, 
                            const irr::core::stringw &newName, const int icon)
{
    // May only be called AFTER this widget has been add()ed
    assert(m_element != NULL);

    CGUISTKListBox* list = getIrrlichtElement<CGUISTKListBox>();
    assert(list != NULL);

    list->setCell(row_index, col_index, newName.c_str(), icon);

    list->setItemOverrideColor( row_index, EGUI_LBC_TEXT          , video::SColor(255,0,0,0) );
    list->setItemOverrideColor( row_index, EGUI_LBC_TEXT_HIGHLIGHT, video::SColor(255,255,255,255) );
}

// -----------------------------------------------------------------------------
void ListWidget::renameItem(const int row_index,
                            const irr::core::stringw &newName, const int icon)
{
    renameCell(row_index, 0, newName, icon);
}

// -----------------------------------------------------------------------------
void ListWidget::renameItem(const std::string &internal_name, 
                            const irr::core::stringw &newName, const int icon)
{
    CGUISTKListBox* list = getIrrlichtElement<CGUISTKListBox>();
    assert(list != NULL);
    renameCell(list->getRowByInternalName(internal_name), 0, newName, icon);
}

// -----------------------------------------------------------------------------

std::string ListWidget::getSelectionInternalName()
{

    CGUISTKListBox* list = getIrrlichtElement<CGUISTKListBox>();
    assert(list != NULL);
    int selectionID = getSelectionID();
    if (selectionID == -1 || selectionID >= (int)list->getItemCount())
        return "";
    const CGUISTKListBox::ListItem& item = list->getItem(selectionID);
    return item.m_internal_name;
}

// -----------------------------------------------------------------------------
irr::core::stringw ListWidget::getSelectionLabel(const int cell) const
{
    const CGUISTKListBox* list = getIrrlichtElement<CGUISTKListBox>();
    assert(list != NULL);
    return list->getCellText( list->getSelected(), cell);
}

// -----------------------------------------------------------------------------

void ListWidget::selectItemWithLabel(const irr::core::stringw& name)
{
    CGUISTKListBox* list = getIrrlichtElement<CGUISTKListBox>();
    assert(list != NULL);
    return list->setSelectedByCellText( name.c_str() );
}

// -----------------------------------------------------------------------------

void ListWidget::unfocused(const int playerID, Widget* new_focus)
{
    CGUISTKListBox* list = getIrrlichtElement<CGUISTKListBox>();

    // remove selection when leaving list
    if (list != NULL && m_properties[PROP_KEEP_SELECTION] != "true")
        list->setSelected(-1);
}

// -----------------------------------------------------------------------------

int ListWidget::getSelectionID() const
{
    // May only be called AFTER this widget has been add()ed
    assert(m_element != NULL);

    return getIrrlichtElement<CGUISTKListBox>()->getSelected();
}

// -----------------------------------------------------------------------------

void ListWidget::setSelectionID(const int index)
{
    // May only be called AFTER this widget has been add()ed
    assert(m_element != NULL);

    CGUISTKListBox* irritem = getIrrlichtElement<CGUISTKListBox>();

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

    const int count = getIrrlichtElement<CGUISTKListBox>()->getItemCount();

    return count;
}

// -----------------------------------------------------------------------------

void ListWidget::elementRemoved()
{
    Widget::elementRemoved();

    for (unsigned int n=0; n<m_header_elements.size(); n++)
    {
        m_header_elements[n].elementRemoved();
        m_children.remove( m_header_elements.get(n) );
    }
    m_header_elements.clearAndDeleteAll();
    m_selected_column = NULL;
    m_sort_desc = false;
    m_sort_default = true;
}

// -----------------------------------------------------------------------------

void ListWidget::markItemRed(const int id, bool red)
{
    // May only be called AFTER this widget has been add()ed
    assert(m_element != NULL);

    CGUISTKListBox* irritem = getIrrlichtElement<CGUISTKListBox>();

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

void ListWidget::markItemBlue(const int id, bool blue)
{
    // May only be called AFTER this widget has been add()ed
    assert(m_element != NULL);

    CGUISTKListBox* irritem = getIrrlichtElement<CGUISTKListBox>();

    if (blue)
    {
        irritem->setItemOverrideColor( id, EGUI_LBC_TEXT,           video::SColor(255,0,0,255) );
        irritem->setItemOverrideColor( id, EGUI_LBC_TEXT_HIGHLIGHT, video::SColor(255,0,0,255) );
    }
    else
    {
        irritem->setItemOverrideColor( id, EGUI_LBC_TEXT,           video::SColor(255,0,0,0) );
        irritem->setItemOverrideColor( id, EGUI_LBC_TEXT_HIGHLIGHT, video::SColor(255,255,255,255) );
    }
}

// -----------------------------------------------------------------------------

EventPropagation ListWidget::transmitEvent(Widget* w,
                                           const std::string& originator,
                                           const int playerID)
{
    assert(m_magic_number == 0xCAFEC001);


    if (originator.find(m_properties[PROP_ID] + "_column_") != std::string::npos)
    {
        if (!m_sortable) return EVENT_BLOCK;

        if (m_sort_col != originator[(m_properties[PROP_ID] + "_column_").size()] - '0')
        {
            m_sort_desc = false;
            m_sort_default = false;
        }
        else
        {
            if (!m_sort_default) m_sort_desc = !m_sort_desc;
            m_sort_default = !m_sort_desc && !m_sort_default;
        }

        m_sort_col = originator[(m_properties[PROP_ID] + "_column_").size()] - '0';
        m_selected_column = m_header_elements.get(m_sort_col);

        /** \brief Allows sort icon to change depending on sort order **/

        /*
        for (int n=0; n<m_header_elements.size(); n++)
        {
            m_header_elements[n].getIrrlichtElement<IGUIButton>()->setPressed(false);
        }
        m_header_elements[col].getIrrlichtElement<IGUIButton>()->setPressed(true);
        */

        if (m_listener) m_listener->onColumnClicked(m_sort_col);

        return EVENT_BLOCK;
    }

    return EVENT_LET;
}

// -----------------------------------------------------------------------------
int ListWidget::getItemID(const std::string &internalName) const
{
    const CGUISTKListBox* list = getIrrlichtElement<CGUISTKListBox>();
    assert(list != NULL);
    return list->getRowByInternalName(internalName);
}
