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

#include "guiengine/widget.hpp"

#include <iostream>
#include <sstream>

#include "irrlicht.h"
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

#include "graphics/irr_driver.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/screen.hpp"
#include "io/file_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"


#ifndef round
# define round(x)  (floor(x+0.5f))
#endif

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
    
    x = -1;
    y = -1;
    w = -1;
    h = -1;
    id = -1;
    m_element = NULL;
    m_title_font = false;
    m_type = type;
    m_focusable = true;
    
    m_event_handler = NULL;
    m_show_bounding_box = false;
    m_parent = NULL;
    m_reserve_id = reserve_id;
    m_supports_multiplayer = false;
    
    m_tab_down_root = -1;
    m_tab_up_root = -1;
    
    for (int n=0; n<MAX_PLAYER_COUNT; n++)
    {
        m_player_focus[n] = false;
        m_selected[n] = false;
    }
    
    m_reserved_id     = -1;
    m_deactivated     = false;
    m_badges          = 0;
    
    // set a default value, derivates can override this as they wish
    m_check_inside_me = (m_type == WTYPE_DIV);
}

// -----------------------------------------------------------------------------

Widget::~Widget()
{
    assert(m_magic_number == 0xCAFEC001);
    
    // If any player focused this widget, unset that focus
    for (int n=0; n<MAX_PLAYER_COUNT; n++)
    {
        if (m_player_focus[n])
        {
            GUIEngine::focusNothingForPlayer(n);
        }
    }
    
    m_magic_number = 0xDEADBEEF;
}
    
// -----------------------------------------------------------------------------

void Widget::elementRemoved()
{
    assert(m_magic_number == 0xCAFEC001);
    
    m_element = NULL;
    
    // If any player focused this widget, unset that focus
    for (int n=0; n<MAX_PLAYER_COUNT; n++)
    {
        if (m_player_focus[n])
        {
            GUIEngine::focusNothingForPlayer(n);
        }
    }
    
}

// -----------------------------------------------------------------------------

void Widget::setActivated()
{    
    // even if this one is already active, do it anyway on purpose, maybe the
    // children widgets need to be updated
    m_deactivated = false;
    const int count = m_children.size();
    for (int n=0; n<count; n++)
    {
        m_children[n].setActivated();
    }
}

// -----------------------------------------------------------------------------

void Widget::setDeactivated()
{
    // even if this one is already inactive, do it anyway on purpose, maybe the
    // children widgets need to be updated
    m_deactivated = true;
    const int count = m_children.size();
    for (int n=0; n<count; n++)
    {
        m_children[n].setDeactivated();
    }
}

// -----------------------------------------------------------------------------
namespace GUIEngine
{
    // IDs must not start at 0, since it appears their GUI engine hardcodes some ID values
    const unsigned int focusableIdsBase = 100;
    const unsigned int unfocusableIdsBase = 1000;
    
    /** Used to assign irrLicht IDs to widgets dynamically */
    static unsigned int id_counter = focusableIdsBase;
    
    /** for items that can't be reached with keyboard navigation but can be clicked */
    static unsigned int id_counter_2 = unfocusableIdsBase;
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
    if (id >= unfocusableIdsBase) return false;
    else                          return true;
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
    if (GUIEngine::getFocusForPlayer(playerID) != NULL)
    {
        GUIEngine::getFocusForPlayer(playerID)->unfocused(playerID);
        GUIEngine::getFocusForPlayer(playerID)->m_player_focus[playerID] = false;
    }
    
    m_player_focus[playerID] = true;
    GUIEngine::Private::g_focus_for_player[playerID] = this;

    // Callback
    this->focused(playerID);
}
    
// -----------------------------------------------------------------------------

void Widget::unsetFocusForPlayer(const int playerID)
{
    assert(m_magic_number == 0xCAFEC001);
    
    if (m_player_focus[playerID]) this->unfocused(playerID);
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

/**
 * Receives as string the raw property value retrieved from XML file.
 * Will try to make sense of it, as an absolute value or a percentage.
 *
 * Return values :
 *     Will write to either absolute or percentage, depending on the case.
 *     Returns false if couldn't convert to either
 */
bool Widget::convertToCoord(std::string& x, int* absolute /* out */, int* percentage /* out */)
{    
    bool is_number;
    int i;
    std::istringstream myStream(x);
    is_number = (myStream >> i)!=0;

    if(!is_number) return false;

    if( x[x.size()-1] == '%' ) // percentage
    {
        *percentage = i;
        return true;
    }
    else // absolute number
    {
        *absolute = i;
        return true;
    }
}

// -----------------------------------------------------------------------------

void Widget::move(const int x, const int y, const int w, const int h)
{
    assert(m_magic_number == 0xCAFEC001);
    
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
    
    m_element->setRelativePosition( core::rect < s32 > (x, y, x+w, y+h) );
}

// -----------------------------------------------------------------------------
/**
 * Finds its x, y, w and h coords from what is specified in the XML properties.
 * Most notably, expands coords relative to parent and percentages.
 */
void Widget::readCoords(Widget* parent)
{
    assert(m_magic_number == 0xCAFEC001);
    
    /* determine widget position and size if not already done by sizers */
    std::string x       = m_properties[PROP_X];
    std::string y       = m_properties[PROP_Y];
    std::string width   = m_properties[PROP_WIDTH];
    std::string height  = m_properties[PROP_HEIGHT];

    /* retrieve parent size (or screen size if none). Will be useful for layout
       and especially for percentages. */
    unsigned int parent_w, parent_h, parent_x, parent_y;
    if(parent == NULL)
    {
        core::dimension2d<u32> frame_size = GUIEngine::getDriver()->getCurrentRenderTargetSize();
        parent_w = frame_size.Width;
        parent_h = frame_size.Height;
        parent_x = 0;
        parent_y = 0;
    }
    else
    {
        parent_w = parent->w;
        parent_h = parent->h;
        parent_x = parent->x;
        parent_y = parent->y;
    }

    // ---- try converting to number
    // x coord
    {
        int abs_x = -1, percent_x = -1;
        if(convertToCoord(x, &abs_x, &percent_x ))
        {
            if(abs_x > -1) this->x = parent_x + abs_x;
            else if(abs_x < -1) this->x = parent_x + (parent_w + abs_x);
            else if(percent_x > -1) this->x = parent_x + parent_w*percent_x/100;
        }
    }

    // y coord
    {
        int abs_y = -1, percent_y = -1;
        if(convertToCoord(y, &abs_y, &percent_y ))
        {
            if(abs_y > -1) this->y = parent_y + abs_y;
            else if(abs_y < -1) this->y = parent_y + (parent_h + abs_y);
            else if(percent_y > -1) this->y = parent_y + parent_h*percent_y/100;
        }
    }

    // ---- if this widget has an icon, get icon size. this can helpful determine its optimal size
    int texture_w = -1, texture_h = -1;

    if(m_properties[PROP_ICON].size() > 0)
    {
        ITexture* texture = irr_driver->getTexture(
                  (file_manager->getDataDir() + "/" + m_properties[PROP_ICON]).c_str());
        
        if(texture != NULL)
        {
            texture_w = texture->getSize().Width;
            texture_h = texture->getSize().Height;
        }
    }
    
    // ---- if this widget has a label, get text size. this can helpful determine its optimal size
    int label_w = -1, label_h = -1;
    if (m_text.size() > 0)
    {
        IGUIFont* font = (m_title_font ? GUIEngine::getTitleFont() : GUIEngine::getFont());
        core::dimension2d< u32 > dim = font->getDimension( m_text.c_str() );
        label_w = dim.Width + getWidthNeededAroundLabel();
        // FIXME - won't work with multiline labels. thus, for now, when multiple
        // lines are required, we need to specify a height explicitely
        label_h = dim.Height + getHeightNeededAroundLabel();
    }

    // ---- read dimension
    // width
    {
        int abs_w = -1, percent_w = -1;
        if(convertToCoord(width, &abs_w, &percent_w ))
        {
            if(abs_w > -1) this->w = abs_w;
            else if(percent_w > -1) this->w = (int)round(parent_w*percent_w/100.0);
        }
        else if(texture_w > -1) this->w = texture_w;
        else if(label_w > -1) this->w = label_w;
    }

    // height
    {
        int abs_h = -1, percent_h = -1;
        if(convertToCoord(height, &abs_h, &percent_h ))
        {
            if(abs_h > -1) this->h = abs_h;
            else if(percent_h > -1) this->h = parent_h*percent_h/100;
        }
        else if(texture_h > -1 && label_h > -1) this->h = texture_h + label_h; // label + icon
        else if(texture_h > -1) this->h = texture_h;
        else if(label_h > -1) this->h = label_h;
    }

    // ---- can't make widget bigger than parent
    if(this->h > (int)parent_h)
    {
        float ratio = (float)parent_h/this->h;

        this->w = (int)(this->w*ratio);
        this->h = (int)(this->h*ratio);
    }
    if(this->w > (int)parent_w)
    {
        // scale down while keeping aspect ratio (don't do this for text widgets though...)
        float ratio = (float)parent_w/this->w;

        this->w = (int)(this->w*ratio);
        if (m_type != WTYPE_LABEL) this->h = (int)(this->h*ratio);
    }

    // ------ check for given max size
    if(m_properties[PROP_MAX_WIDTH].size() > 0)
    {
        const int max_width = atoi( this->m_properties[PROP_MAX_WIDTH].c_str() );
        if(this->w > max_width) this->w = max_width;
    }

    if(m_properties[PROP_MAX_HEIGHT].size() > 0)
    {
        const int max_height = atoi( this->m_properties[PROP_MAX_HEIGHT].c_str() );
        if(this->h > max_height) this->h = max_height;
    }    
}

// -----------------------------------------------------------------------------

void Widget::setParent(IGUIElement* parent)
{
    assert(m_magic_number == 0xCAFEC001);
    m_parent = parent;
}

