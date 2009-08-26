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
#include "guiengine/my_button.hpp"
#include "guiengine/screen.hpp"
#include "io/file_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"


#ifndef round
# define round(x)  (floor(x+0.5f))
#endif

namespace GUIEngine
{
    
// -----------------------------------------------------------------------------
static unsigned int id_counter = 0;
static unsigned int id_counter_2 = 1000; // for items that can't be reached with keyboard navigation but can be clicked

int Widget::getNewID()
{
    return id_counter++;
}
int Widget::getNewNoFocusID()
{
    return id_counter_2++;
}

// -----------------------------------------------------------------------------
Widget::Widget()
{
    x = -1;
    y = -1;
    w = -1;
    h = -1;
    id = -1;
    m_element = NULL;
    m_type = WTYPE_NONE;
    m_selected = false;
    m_event_handler = NULL;
    m_show_bounding_box = false;
    m_parent = NULL;
}
// -----------------------------------------------------------------------------
/** When switching to a new screen, this function will be called to reset ID counters
 * (so we start again from ID 0, and don't grow to big numbers) */
void Widget::resetIDCounters()
{
    id_counter = 0;
    id_counter_2 = 1000;
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
void Widget::move(const int x, const int y, const int w, const int h)
{
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
#ifdef IRR_SVN
        core::dimension2d<u32> frame_size = GUIEngine::getDriver()->getCurrentRenderTargetSize();
#else
        core::dimension2d<s32> frame_size = GUIEngine::getDriver()->getCurrentRenderTargetSize();
#endif
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
        ITexture* texture = GUIEngine::getDriver()->getTexture(
                                                               (file_manager->getDataDir() + "/" + m_properties[PROP_ICON]).c_str()
                                                                );
        if(texture != NULL)
        {
            texture_w = texture->getSize().Width;
            texture_h = texture->getSize().Height;
        }
    }

    // ---- if this widget has a label, get text length. this can helpful determine its optimal size
    int label_w = -1, label_h = -1;
    if(m_properties[PROP_TEXT].size() > 0)
    {
        IGUIFont* font = GUIEngine::getFont();
#ifdef IRR_SVN
        core::dimension2d< u32 > dim = font->getDimension( stringw(m_properties[PROP_TEXT].c_str()).c_str() );
#else
        core::dimension2d< s32 > dim = font->getDimension( stringw(m_properties[PROP_TEXT].c_str()).c_str() );
#endif
        label_w = dim.Width;
        // FIXME - won't work with multiline labels. thus, for now, when multiple
        // lines are required, we need to specify a height explicitely
        label_h = dim.Height;
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
        else if(texture_h > -1 && label_h > -1) this->h = texture_h + label_h;
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
        float ratio = (float)parent_w/this->w;

        this->w = (int)(this->w*ratio);
        this->h = (int)(this->h*ratio);
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

void Widget::setParent(IGUIElement* parent)
{
    m_parent = parent;
}

}

