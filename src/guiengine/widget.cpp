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

#include "guiengine/widget.hpp"

#include <iostream>
#include <sstream>

#include <IGUIElement.h>

using namespace irr;
using namespace core;
using namespace video;
using namespace io;
using namespace gui;

#include "guiengine/engine.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/screen.hpp"
#include "io/file_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"
#include "utils/vs.hpp"

namespace GUIEngine
{

    static bool g_is_within_a_text_box = false;
    bool isWithinATextBox()
    {
        return g_is_within_a_text_box;
    }
    void setWithinATextBox(bool in)
    {
        g_is_within_a_text_box = in;
    }
}
using namespace GUIEngine;

// -----------------------------------------------------------------------------

Widget::Widget(WidgetType type, bool reserve_id)
{
    m_magic_number = 0xCAFEC001;

    m_x  = -1;
    m_y  = -1;
    m_w  = -1;
    m_h  = -1;
    m_id = -1;
    m_badge_x_shift         = 0;
    m_element               = NULL;
    m_title_font            = false;
    m_type                  = type;
    m_parent                = NULL;
    m_focusable             = true;
    m_bottom_bar            = false;
    m_top_bar               = false;
    m_event_handler         = NULL;
    m_reserve_id            = reserve_id;
    m_show_bounding_box     = false;
    m_supports_multiplayer  = false;
    m_is_bounding_box_round = false;
    m_has_tooltip           = false;

    m_absolute_x = m_absolute_y = m_absolute_w = m_absolute_h = -1;
    m_relative_x = m_relative_y = m_relative_w = m_relative_h = -1;
    m_absolute_reverse_x = m_absolute_reverse_y = -1;


    m_tab_down_root = -1;
    m_tab_up_root = -1;

    for (unsigned int n=0; n<MAX_PLAYER_COUNT; n++)
    {
        m_player_focus[n] = false;
        m_selected[n] = false;
    }

    m_reserved_id     = -1;
    m_deactivated     = false;
    m_is_visible      = true;
    m_badges          = 0;

    // set a default value, derivates can override this as they wish
    m_check_inside_me = (m_type == WTYPE_DIV);
}

// -----------------------------------------------------------------------------

Widget::~Widget()
{
    assert(m_magic_number == 0xCAFEC001);

    // If any player focused this widget, unset that focus
    for (unsigned int n=0; n<MAX_PLAYER_COUNT; n++)
    {
        if (m_player_focus[n])
        {
            GUIEngine::focusNothingForPlayer(n);
        }
    }

    m_magic_number = 0xDEADBEEF;
}

// -----------------------------------------------------------------------------
void Widget::setText(const wchar_t *s)
{
    m_text = s;
    if(m_element)
        m_element->setText(s);
}   // setText

// -----------------------------------------------------------------------------

void Widget::elementRemoved()
{
    assert(m_magic_number == 0xCAFEC001);

    m_element = NULL;
    m_is_visible = true;

    // If any player focused this widget, unset that focus
    for (unsigned int n=0; n<MAX_PLAYER_COUNT; n++)
    {
        if (m_player_focus[n])
        {
            GUIEngine::focusNothingForPlayer(n);
        }
    }

}

// -----------------------------------------------------------------------------

void Widget::setActive(bool active)
{
    // even if this one is already active, do it anyway on purpose, maybe the
    // children widgets need to be updated
    m_deactivated = !active;
    const int count = m_children.size();
    for (int n=0; n<count; n++)
    {
        m_children[n].setActive(active);
    }
}

// -----------------------------------------------------------------------------

bool Widget::deleteChild(const char* id)
{
    const int count = m_children.size();
    for (int n=0; n<count; n++)
    {
        if (m_children[n].m_properties[PROP_ID] == id)
        {
            m_children.erase(n);
            return true;
        }
    }
    return false;
}

// -----------------------------------------------------------------------------
namespace GUIEngine
{
    // IDs must not start at 0, since it appears their GUI engine hardcodes some ID values
    const unsigned int FOCUSABLE_IDS_BASE = 100;
    const unsigned int UNFOCUSABLE_IDS_BASE = 1000;

    /** Used to assign irrLicht IDs to widgets dynamically */
    static unsigned int id_counter = FOCUSABLE_IDS_BASE;

    /** for items that can't be reached with keyboard navigation but can be clicked */
    static unsigned int id_counter_2 = UNFOCUSABLE_IDS_BASE;
}

int Widget::getNewID()
{
    return id_counter++;
}
int Widget::getNewNoFocusID()
{
    return id_counter_2++;
}


bool Widget::isFocusableId(const int id)
{
    if (id < 0) return false;

    if ((unsigned int)id >= UNFOCUSABLE_IDS_BASE) return false;
    else                                        return true;
}

// -----------------------------------------------------------------------------

/** When switching to a new screen, this function will be called to reset ID counters
 * (so we start again from ID 0, and don't grow to big numbers) */
void Widget::resetIDCounters()
{
    id_counter = 100;
    id_counter_2 = 1000;
}

// -----------------------------------------------------------------------------

void Widget::add()
{
    assert(m_magic_number == 0xCAFEC001);
    if (m_reserve_id)
    {
        m_reserved_id = getNewID();
    }
}

// -----------------------------------------------------------------------------
/**
  * \param playerID ID of the player you want to set/unset focus for, starting from 0
  * Since the code tracks focus from main player, this will most likely be used only
  * for additionnal players
  */
void Widget::setFocusForPlayer(const int playerID)
{
    assert(m_magic_number == 0xCAFEC001);

    // Unset focus flag on previous widget that had focus
    Widget* previous_focus = GUIEngine::getFocusForPlayer(playerID);
    if (previous_focus != NULL)
    {
        previous_focus->unfocused(playerID, this);
        previous_focus->m_player_focus[playerID] = false;
    }

    m_player_focus[playerID] = true;
    GUIEngine::Private::g_focus_for_player[playerID] = this;

    // Callback
    this->focused(playerID);

    Screen* screen = GUIEngine::getCurrentScreen();
    if(screen)
        screen->onFocusChanged(previous_focus, this, playerID);
}

// -----------------------------------------------------------------------------

void Widget::unsetFocusForPlayer(const int playerID)
{
    assert(m_magic_number == 0xCAFEC001);

    if (m_player_focus[playerID]) this->unfocused(playerID, NULL);
    m_player_focus[playerID] = false;
}

// -----------------------------------------------------------------------------

/**
 * \param playerID ID of the player you want to set/unset focus for, starting from 0
 */
bool Widget::isFocusedForPlayer(const int playerID)
{
    assert(m_magic_number == 0xCAFEC001);

    return m_player_focus[playerID];
}

// -----------------------------------------------------------------------------

void Widget::move(const int x, const int y, const int w, const int h)
{
    assert(m_magic_number == 0xCAFEC001);

    m_x = x;
    m_y = y;
    m_w = w;
    m_h = h;

    if (m_element != NULL)
        m_element->setRelativePosition( core::rect < s32 > (x, y, x+w, y+h) );
}

// -----------------------------------------------------------------------------

void Widget::setParent(IGUIElement* parent)
{
    assert(m_magic_number == 0xCAFEC001);
    m_parent = parent;
}

// -----------------------------------------------------------------------------

bool Widget::isVisible() const
{
    if (m_element != NULL)
    {
        assert(m_element->isVisible() == m_is_visible);
    }
    return m_is_visible;
}

// -----------------------------------------------------------------------------

bool Widget::isActivated() const
{
    if (isVisible())
        return !m_deactivated;
    return false;
}

// -----------------------------------------------------------------------------

void Widget::setVisible(bool visible)
{
    if (m_element != NULL)
    {
        m_element->setVisible(visible);
    }
    m_is_visible = visible;

    const int childrenCount = m_children.size();
    for (int n=0; n<childrenCount; n++)
    {
        m_children[n].setVisible(visible);
    }
}

// -----------------------------------------------------------------------------

void Widget::moveIrrlichtElement()
{
    if (m_element != NULL)
    {
        m_element->setRelativePosition( irr::core::recti(irr::core::position2di(m_x, m_y),
                                                         irr::core::dimension2di(m_w, m_h) ) );
    }
}
