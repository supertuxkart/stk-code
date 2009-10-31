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

#include "guiengine/engine.hpp"
#include "guiengine/widgets/list_widget.hpp"
using namespace GUIEngine;
using namespace irr::core;
using namespace irr::gui;

ListWidget::ListWidget()
{
    m_type = WTYPE_LIST;
}
// -----------------------------------------------------------------------------
void ListWidget::add()
{
    rect<s32> widget_size = rect<s32>(x, y, x + w, y + h);
    
    m_element = GUIEngine::getGUIEnv()->addListBox (widget_size, m_parent, getNewID());
}
// -----------------------------------------------------------------------------
void ListWidget::clear()
{
    IGUIListBox* list = getIrrlichtElement<IGUIListBox>();
    assert(list != NULL);
    
    list->clear();
}
// -----------------------------------------------------------------------------
/* // Doesn't work, I would need to override CGUIListBox, but this class is private
bool ListWidget::OnEvent (const SEvent &event)
{
    // block input events, we will handle them (vertical navigation) ourselves
    if (event.EventType == EET_KEY_INPUT_EVENT ||
        event.EventType == EET_JOYSTICK_INPUT_EVENT ||
        event.EventType == EET_MOUSE_INPUT_EVENT)
    {
        return true;
    }
    else
    {
        return false;
    }
}
 */
// -----------------------------------------------------------------------------
void ListWidget::addItem(const char* item)
{
    IGUIListBox* list = getIrrlichtElement<IGUIListBox>();
    assert(list != NULL);
    list->addItem( stringw(item).c_str() );
}
// -----------------------------------------------------------------------------
int ListWidget::getSelection() const
{
    const IGUIListBox* list = getIrrlichtElement<IGUIListBox>();
    assert(list != NULL);
    return list->getSelected();
}
// -----------------------------------------------------------------------------
std::string ListWidget::getSelectionName() const
{
    const IGUIListBox* list = getIrrlichtElement<IGUIListBox>();
    assert(list != NULL);
    return stringc( list->getListItem( list->getSelected() ) ).c_str();
}

