//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 Marianne Gagnon
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

#include "guiengine/layout_manager.hpp"

#include <iostream>

#include <IrrlichtDevice.h>
#include <IGUIFont.h>
#include <ITexture.h>

#include "graphics/irr_driver.hpp"
#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/skin.hpp"
#include "guiengine/widget.hpp"
#include "io/file_manager.hpp"
#include "utils/ptr_vector.hpp"
#include "utils/string_utils.hpp"
#include "utils/vs.hpp"

using namespace irr;
using namespace gui;
using namespace video;
using namespace GUIEngine;

/** Like atoi, but on error prints an error message to stderr */
int atoi_p(const char* val)
{
    int i;
    if (StringUtils::parseString<int>(val, &i))
    {
        return i;
    }
    else
    {
        Log::warn("LayoutManager", "Invalid value '%s' found in XML file where integer was expected.", val);
        return 0;
    }
}

// ----------------------------------------------------------------------------

bool LayoutManager::convertToCoord(std::string& x, int* absolute /* out */, int* percentage /* out */)
{
    int i = 0;
    if (!StringUtils::fromString<int>(x, i /* out */)) return false;

    if( x[x.size()-1] == '%' ) // percentage
    {
        *percentage = i;
        return true;
    }
    else if( x[x.size()-1] == 'f' ) // font height
    {
        *absolute = i * GUIEngine::getFontHeight();
    	return true;
    }
    else // absolute number
    {
        *absolute = i;
        return true;
    }
}


// ----------------------------------------------------------------------------

void LayoutManager::readCoords(Widget* self)
{
    // determine widget position and size if not already done by sizers
    std::string x      = self->m_properties[PROP_X];
    std::string y      = self->m_properties[PROP_Y];
    std::string width  = self->m_properties[PROP_WIDTH];
    std::string height = self->m_properties[PROP_HEIGHT];

    /*
    // retrieve parent size (or screen size if none). Will be useful for layout
    // and especially for percentages.
    unsigned int parent_w, parent_h, parent_x, parent_y;
    if(parent == NULL)
    {
        //core::dimension2d<u32> frame_size = GUIEngine::getDriver()->getCurrentRenderTargetSize();
        //parent_w = frame_size.Width;
        //parent_h = frame_size.Height;
        parent_w = topLevelContainer->getWidth();
        parent_h = topLevelContainer->getHeight();
        parent_x = 0;
        parent_y = 0;
    }
    else
    {
        parent_w = parent->m_w;
        parent_h = parent->m_h;
        parent_x = parent->m_x;
        parent_y = parent->m_y;
    }
    */

    // ---- try converting to number
    // x coord
    {
        int abs_x = 0x7FFFFFFF, percent_x = 0x7FFFFFFF;
        if (convertToCoord(x, &abs_x, &percent_x ))
        {
            if      (abs_x >= 0 && abs_x != 0x7FFFFFFF)
            {
                self->m_absolute_x = abs_x;
            }
            else if (abs_x < 0)
            {
                self->m_absolute_reverse_x = abs(abs_x);
            }
            else if (percent_x >= 0 && percent_x != 0x7FFFFFFF)
            {
                self->m_relative_x = (float)percent_x;
            }
        }
    }

    // y coord
    {
        int abs_y = 0x7FFFFFFF, percent_y = 0x7FFFFFFF;
        if (convertToCoord(y, &abs_y, &percent_y ))
        {
            if      (abs_y >= 0 && abs_y != 0x7FFFFFFF)
            {
                self->m_absolute_y = abs_y;
            }
            else if (abs_y < 0)
            {
                self->m_absolute_reverse_y = abs(abs_y);
            }
            else if (percent_y >= 0 && percent_y != 0x7FFFFFFF)
            {
                self->m_relative_y = (float)percent_y;
            }
        }
    }

    // ---- if this widget has an icon, get icon size. this can helpful determine its optimal size
    int texture_w = -1, texture_h = -1;

    if (self->m_properties[PROP_ICON].size() > 0)
    {
        // PROP_ICON includes paths (e.g. gui/icons/logo.png)
        ITexture* texture = irr_driver->getTexture(
            GUIEngine::getSkin()->getThemedIcon(self->m_properties[PROP_ICON]));

        if (texture != NULL)
        {
            texture_w = texture->getSize().Width;
            texture_h = texture->getSize().Height;
        }
    }

    // ---- if this widget has a label, get text size. this can helpful determine its optimal size
    int label_w = -1, label_h = -1;
    if (self->m_text.size() > 0)
    {
        IGUIFont* font = (self->m_title_font ? GUIEngine::getTitleFont() : GUIEngine::getFont());
        core::dimension2d< u32 > dim = font->getDimension( self->m_text.c_str() );
        label_w = dim.Width + self->getWidthNeededAroundLabel();
        // FIXME - won't work with multiline labels. thus, for now, when multiple
        // lines are required, we need to specify a height explicitely
        label_h = dim.Height + self->getHeightNeededAroundLabel();
    }
    else if (self->getType() == WTYPE_CHECKBOX)
    {
        // User text height to guess checkbox size
        IGUIFont* font = (self->m_title_font ? GUIEngine::getTitleFont() : GUIEngine::getFont());
        core::dimension2d< u32 > dim = font->getDimension( L"X" );
        label_h = dim.Height + self->getHeightNeededAroundLabel();
        label_w = label_h; // a checkbox is square
    }

    if (label_h == -1)
    {
        if (self->getType() == WTYPE_TEXTBOX ||
            self->getType() == WTYPE_BUTTON  ||
            self->getType() == WTYPE_LABEL   ||
            self->getType() == WTYPE_SPINNER)
        {
            IGUIFont* font = (self->m_title_font ? GUIEngine::getTitleFont() : GUIEngine::getFont());

            // get text height, a text box, button or label is always as high as the text it could contain
            core::dimension2d< u32 > dim = font->getDimension( L"X" );
            label_h = dim.Height + self->getHeightNeededAroundLabel();
        }
    }

    // ---- if this widget has children, get their size size. this can helpful determine its optimal size
    if (self->m_properties[PROP_WIDTH] == "fit" || self->m_properties[PROP_HEIGHT] == "fit")
    {
        bool is_vertical_row = (self->m_properties[PROP_LAYOUT] == "vertical-row");
        bool is_horizontal_row = (self->m_properties[PROP_LAYOUT] == "horizontal-row");

        int child_max_width = -1, child_max_height = -1;
        int total_width = 0, total_height = 0;

        for (unsigned int child=0; child<self->m_children.size(); child++)
        {
            if (self->m_children[child].m_absolute_w > -1)
            {
                if (is_horizontal_row)
                {
                    total_width += self->m_children[child].m_absolute_w ;
                }
                else
                {
                    if (child_max_width == -1 || self->m_children[child].m_absolute_w > child_max_width)
                    {
                        child_max_width = self->m_children[child].m_absolute_w;
                    }
                }
            }

            if (self->m_children[child].m_absolute_h > -1)
            {
                if (is_vertical_row)
                {
                    total_height += self->m_children[child].m_absolute_h;
                }
                else
                {
                    if (child_max_height == -1 || self->m_children[child].m_absolute_h > child_max_height)
                    {
                        child_max_height = self->m_children[child].m_absolute_h;
                    }
                }
            }
        }

        //Add padding to <box> elements
        if (self->getType() == WTYPE_DIV && self->m_show_bounding_box)
        {
            int padding = GUIEngine::getFontHeight() / 2;
            if (self->m_properties[PROP_DIV_PADDING].length() > 0)
                padding = atoi(self->m_properties[PROP_DIV_PADDING].c_str());
            child_max_height += padding * 2;
            total_height += padding * 2;
            total_width += padding * 2;
            child_max_width += padding * 2;
        }

        if (self->m_properties[PROP_WIDTH] == "fit")
        {
            self->m_absolute_w = (is_horizontal_row ? total_width : child_max_width);
        }
        if (self->m_properties[PROP_HEIGHT] == "fit")
        {
            self->m_absolute_h = (is_vertical_row ? total_height : child_max_height);
        }
    }

    // ---- read dimension
    // width
    {
        int abs_w = -1, percent_w = -1;
        if (width == "font") self->m_absolute_w = GUIEngine::getFontHeight();
        else if (convertToCoord(width, &abs_w, &percent_w ))
        {
            if      (abs_w > -1)     self->m_absolute_w = abs_w;
            else if (percent_w > -1) self->m_relative_w = (float)percent_w;
        }
        else if(texture_w > -1) self->m_absolute_w = texture_w;
        else if(label_w > -1)   self->m_absolute_w = label_w;
    }

    // height
    {
        int abs_h = -1, percent_h = -1;
        if (height == "font") self->m_absolute_h = GUIEngine::getFontHeight();
        else if (convertToCoord(height, &abs_h, &percent_h ))
        {
            if      (abs_h > -1)     self->m_absolute_h = abs_h;
            else if (percent_h > -1) self->m_relative_h = (float)percent_h;
        }
        else if (texture_h > -1 && label_h > -1) self->m_absolute_h = texture_h + label_h; // label + icon
        else if (texture_h > -1)                 self->m_absolute_h = texture_h;
        else if (label_h > -1)                   self->m_absolute_h = label_h;
    }

    if (self->getType() != WTYPE_RIBBON) // Ribbons have their own handling
    {
        // Set vertical inner padding
        self->m_absolute_h += self->m_absolute_h * SkinConfig::getVerticalInnerPadding(self->getType(), self);
//        self->m_relative_h += self->m_relative_h * SkinConfig::getVerticalInnerPadding(self->getType(), self);

        // Set horizontal inner padding
        self->m_absolute_w += self->m_absolute_w * SkinConfig::getHorizontalInnerPadding(self->getType(), self);
//        self->m_relative_w += self->m_relative_w * SkinConfig::getHorizontalInnerPadding(self->getType(), self);
    }

}

// ----------------------------------------------------------------------------

void LayoutManager::applyCoords(Widget* self, AbstractTopLevelContainer* topLevelContainer, Widget* parent)
{
    // retrieve parent size (or screen size if none). Will be useful for layout
    // and especially for percentages.
    unsigned int parent_w, parent_h, parent_x, parent_y;
    if(parent == NULL)
    {
        //core::dimension2d<u32> frame_size = GUIEngine::getDriver()->getCurrentRenderTargetSize();
        //parent_w = frame_size.Width;
        //parent_h = frame_size.Height;
        parent_w = topLevelContainer->getWidth();
        int left_padding = irr_driver->getDevice()->getLeftPadding();
        int right_padding = irr_driver->getDevice()->getRightPadding();
        if (!topLevelContainer->enableScreenPadding())
            left_padding = right_padding = 0;
        int total_padding = left_padding + right_padding;
        if (parent_w - total_padding > 0)
            parent_w -= total_padding;
        parent_h = topLevelContainer->getHeight();
        parent_x = 0;
        if (left_padding > 0)
            parent_x = left_padding;
        parent_y = 0;
    }
    else
    {
        parent_w = parent->m_w;
        parent_h = parent->m_h;
        parent_x = parent->m_x;
        parent_y = parent->m_y;
    }
    
    if (parent != NULL && parent->getType() == WTYPE_DIV && parent->m_show_bounding_box)
    {
        int padding = GUIEngine::getFontHeight() / 2;
        if (parent->m_properties[PROP_DIV_PADDING].length() > 0)
            padding = atoi(parent->m_properties[PROP_DIV_PADDING].c_str());
            
        parent_x += padding;
        parent_y += padding;
        parent_w -= padding*2;
        parent_h -= padding*2;
    }
    
    if      (self->m_absolute_x > -1)         self->m_x = parent_x + self->m_absolute_x;
    else if (self->m_absolute_reverse_x > -1) self->m_x = parent_x + (parent_w - self->m_absolute_reverse_x);
    else if (self->m_relative_x > -1)         self->m_x = (int)(parent_x + parent_w*self->m_relative_x/100);

    if      (self->m_absolute_y > -1)         self->m_y = parent_y + self->m_absolute_y;
    else if (self->m_absolute_reverse_y > -1) self->m_y = parent_y + (parent_h - self->m_absolute_reverse_y);
    else if (self->m_relative_y > -1)         self->m_y = (int)(parent_y + parent_h*self->m_relative_y/100);

    if (self->m_absolute_w > -1)      self->m_w = self->m_absolute_w;
    else if (self->m_relative_w > -1) self->m_w = (int)roundf(parent_w*self->m_relative_w/100.0f);

    if (self->m_absolute_h > -1)      self->m_h = self->m_absolute_h;
    else if (self->m_relative_h > -1) self->m_h = (int)roundf(parent_h*self->m_relative_h/100.0f);

    // ---- can't make widget bigger than parent
    if (self->m_h > (int)parent_h)
    {
        float ratio = (float)parent_h / self->m_h;

        self->m_w = (int)(self->m_w*ratio);
        self->m_h = (int)(self->m_h*ratio);
    }
    if (self->m_w > (int)parent_w)
    {
        // scale down while keeping aspect ratio (don't do this for text widgets though...)
        float ratio = (float)parent_w / self->m_w;

        self->m_w = (int)(self->m_w * ratio);

        // FIXME: ugly to hardcode widgets types here
        if (self->m_type != WTYPE_LABEL && self->m_type != WTYPE_BUBBLE)
        {
            self->m_h = (int)(self->m_h * ratio);
        }
    }

    // ------ check for given max size
    if (self->m_properties[PROP_MAX_WIDTH].size() > 0)
    {
        const int max_width = atoi_p( self->m_properties[PROP_MAX_WIDTH].c_str() );
        if (self->m_w > max_width) self->m_w = max_width;
    }

    if (self->m_properties[PROP_MAX_HEIGHT].size() > 0)
    {
        const int max_height = atoi_p( self->m_properties[PROP_MAX_HEIGHT].c_str() );
        if (self->m_h > max_height) self->m_h = max_height;
    }
}

// ----------------------------------------------------------------------------

void LayoutManager::recursivelyReadCoords(PtrVector<Widget>& widgets)
{
    const unsigned short widgets_amount = widgets.size();

    // ----- deal with containers' children
    for (int n=0; n<widgets_amount; n++)
    {
        if (widgets[n].m_type == WTYPE_DIV) recursivelyReadCoords(widgets[n].m_children);
    }

    // ----- read x/y/size parameters
    for (unsigned short n=0; n<widgets_amount; n++)
    {
        readCoords(widgets.get(n));
    }//next widget
}

// ----------------------------------------------------------------------------

void LayoutManager::calculateLayout(PtrVector<Widget>& widgets, AbstractTopLevelContainer* topLevelContainer)
{
    recursivelyReadCoords(widgets);
    doCalculateLayout(widgets, topLevelContainer, NULL);
}

// ----------------------------------------------------------------------------

void LayoutManager::doCalculateLayout(PtrVector<Widget>& widgets, AbstractTopLevelContainer* topLevelContainer,
                                      Widget* parent)
{
    const unsigned short widgets_amount = widgets.size();

    for (int n=0; n<widgets_amount; n++)
    {
        applyCoords(widgets.get(n), topLevelContainer, parent);
    }

    // ----- manage 'layout's if relevant
    do // i'm using 'while false' here just to be able to 'break' ...
    {
        if(parent == NULL) break;

        std::string layout_name = parent->m_properties[PROP_LAYOUT];
        if(layout_name.size() < 1) break;

        bool horizontal = false;

        if (!strcmp("horizontal-row", layout_name.c_str()))
            horizontal = true;
        else if(!strcmp("vertical-row", layout_name.c_str()))
            horizontal = false;
        else
        {
            Log::error("LayoutManager::doCalculateLayout", "Unknown layout name: %s", layout_name.c_str());
            break;
        }

        int x = parent->m_x;
        int y = parent->m_y;
        int w = parent->m_w;
        int h = parent->m_h;

        if (parent != NULL && parent->getType() == WTYPE_DIV && parent->m_show_bounding_box)
        {
            int padding = GUIEngine::getFontHeight() / 2;
            if (parent->m_properties[PROP_DIV_PADDING].length() > 0)
                padding = atoi(parent->m_properties[PROP_DIV_PADDING].c_str());
            
            x += padding;
            y += padding;
            w -= padding*2;
            h -= padding*2;
        }
    
        // find space left after placing all absolutely-sized widgets in a row
        // (the space left will be divided between remaining widgets later)
        int left_space = (horizontal ? w : h);
        unsigned short total_proportion = 0;
        for(int n=0; n<widgets_amount; n++)
        {
            // relatively-sized widget
            std::string prop = widgets[n].m_properties[ PROP_PROPORTION ];
            if(prop.size() != 0)
            {
                total_proportion += atoi_p( prop.c_str() );
                continue;
            }

            // absolutely-sized widgets
            left_space -= (horizontal ? widgets[n].m_w : widgets[n].m_h);
        } // next widget

        if (left_space < 0)
        {
            Log::warn("LayoutManager", "Statically sized widgets took all the place!!");
            left_space = 0;
        }

        // ---- lay widgets in row
        for (int n=0; n<widgets_amount; n++)
        {
            std::string prop = widgets[n].m_properties[ PROP_PROPORTION ];
            if(prop.size() != 0)
            {
                // proportional size
                int proportion = 1;
                std::istringstream myStream(prop);
                if (!(myStream >> proportion))
                    Log::warn("LayoutManager::doCalculateLayout",
                        "Proportion '%s' is not a number for widget %s", prop.c_str(),
                        widgets[n].m_properties[PROP_ID].c_str());

                const float fraction = (float)proportion/(float)total_proportion;

                if (horizontal)
                {
                    widgets[n].m_x = x;

                    std::string align = widgets[n].m_properties[ PROP_ALIGN ];
                    if (align.size() < 1)
                    {
                        std::string prop_y = widgets[n].m_properties[ PROP_Y ];
                        if (prop_y.size() > 0)
                        {
                            if (prop_y[ prop_y.size()-1 ] == '%')
                            {
                                prop_y = prop_y.substr(0, prop_y.size() - 1);
                                widgets[n].m_y = (int)(y + atoi_p(prop_y.c_str())/100.0f * h);
                            }
                            else if(prop_y[ prop_y.size()-1 ] == 'f')
                            {
                                widgets[n].m_y = y + atoi_p(prop_y.c_str()) * GUIEngine::getFontHeight();
                            }
                            else
                            {
                                widgets[n].m_y = y + atoi_p(prop_y.c_str());
                            }
                        }
                        else
                        {
                            widgets[n].m_y = y;
                        }
                    }
                    else if (align == "top")
                    {
                        widgets[n].m_y = y;
                    }
                    else if (align == "center")
                    {
                        widgets[n].m_y = y + h/2 - widgets[n].m_h/2;
                    }
                    else if (align == "bottom")
                    {
                        widgets[n].m_y = y + h - widgets[n].m_h;
                    }
                    else
                    {
                        Log::warn("LayoutManager::doCalculateLayout",
                            "Alignment '%s' is unknown (widget '%s', in a horiozntal-row layout)",
                            align.c_str(), widgets[n].m_properties[PROP_ID].c_str());
                    }

                    widgets[n].m_w = (int)(left_space*fraction);
                    if (widgets[n].m_properties[PROP_MAX_WIDTH].size() > 0)
                    {
                        const int max_width = atoi_p( widgets[n].m_properties[PROP_MAX_WIDTH].c_str() );
                        if (widgets[n].m_w > max_width) widgets[n].m_w = max_width;
                    }

                    if (widgets[n].m_w <= 0)
                    {
                        Log::warn("LayoutManager", "Widget '%s' has a width of %i (left_space = %i, "
                                  "fraction = %f, max_width = %s)", widgets[n].m_properties[PROP_ID].c_str(),
                                  widgets[n].m_w, left_space, fraction, widgets[n].m_properties[PROP_MAX_WIDTH].c_str());
                        widgets[n].m_w = 1;
                    }

                    x += widgets[n].m_w;
                }
                else
                {
                    widgets[n].m_h = (int)(left_space*fraction);

                    if (widgets[n].m_properties[PROP_MAX_HEIGHT].size() > 0)
                    {
                        const int max_height = atoi_p( widgets[n].m_properties[PROP_MAX_HEIGHT].c_str() );
                        if (widgets[n].m_h > max_height) widgets[n].m_h = max_height;
                    }

                    if (widgets[n].m_h <= 0)
                    {
                        Log::warn("LayoutManager", "Widget '%s' has a height of %i (left_space = %i, "
                                  "fraction = %f, max_width = %s)\n", widgets[n].m_properties[PROP_ID].c_str(),
                                  widgets[n].m_h, left_space, fraction, widgets[n].m_properties[PROP_MAX_WIDTH].c_str());
                        widgets[n].m_h = 1;
                    }

                    std::string align = widgets[n].m_properties[ PROP_ALIGN ];
                    if (align.size() < 1)
                    {
                        std::string prop_x = widgets[n].m_properties[ PROP_X ];
                        if (prop_x.size() > 0)
                        {
                            if (prop_x[ prop_x.size()-1 ] == '%')
                            {
                                prop_x = prop_x.substr(0, prop_x.size() - 1);
                                widgets[n].m_x = (int)(x + atoi_p(prop_x.c_str())/100.0f * w);
                            }
                            else if(prop_x[ prop_x.size()-1 ] == 'f')
                            {
                                widgets[n].m_x = x + atoi_p(prop_x.c_str()) * GUIEngine::getFontHeight();
                            }
                            else
                            {
                                widgets[n].m_x = x + atoi_p(prop_x.c_str());
                            }
                        }
                        else
                        {
                            widgets[n].m_x = x;
                        }
                    }
                    else if (align == "left")
                    {
                        widgets[n].m_x = x;
                    }
                    else if (align == "center")
                    {
                        widgets[n].m_x = x + w/2 - widgets[n].m_w/2;
                    }
                    else if (align == "right")
                    {
                        widgets[n].m_x = x + w - widgets[n].m_w;
                    }
                    else
                    {
                        Log::warn("LayoutManager::doCalculateLayout",
                            "Alignment '%s' is unknown (widget '%s', in a vertical-row layout)",
                            align.c_str(), widgets[n].m_properties[PROP_ID].c_str());
                    }
                    widgets[n].m_y = y;

                    y += widgets[n].m_h;
                }
            }
            else
            {
                // absolute size

                if (horizontal)
                {
                    widgets[n].m_x = x;

                    std::string align = widgets[n].m_properties[ PROP_ALIGN ];

                    if (align.size() < 1)
                    {
                        std::string prop_y = widgets[n].m_properties[ PROP_Y ];

                        if (prop_y.size() > 0)
                        {
                            if (prop_y[ prop_y.size()-1 ] == '%')
                            {
                                prop_y = prop_y.substr(0, prop_y.size() - 1);
                                widgets[n].m_y = (int)(y + atoi_p(prop_y.c_str())/100.0f * h);
                            }
                            else if(prop_y[ prop_y.size()-1 ] == 'f')
                            {
                                widgets[n].m_y = y + atoi_p(prop_y.c_str()) * GUIEngine::getFontHeight();
                            }
                            else
                            {
                                widgets[n].m_y = y + atoi_p(prop_y.c_str());
                            }
                        }
                        else
                        {
                            widgets[n].m_y = y;
                        }
                    }
                    else if (align == "top")
                    {
                        widgets[n].m_y = y;
                    }
                    else if (align == "center")
                    {
                        widgets[n].m_y = y + h/2 - widgets[n].m_h/2;
                    }
                    else if (align == "bottom")
                    {
                        widgets[n].m_y = y + h - widgets[n].m_h;
                    }
                    else
                    {
                        Log::warn("LayoutManager::doCalculateLayout",
                            "Alignment '%s' is unknown in widget '%s'",
                            align.c_str(), widgets[n].m_properties[PROP_ID].c_str());
                    }

                    x += widgets[n].m_w;
                }
                else
                {
                    std::string align = widgets[n].m_properties[ PROP_ALIGN ];

                    if (align.size() < 1)
                    {
                        std::string prop_x = widgets[n].m_properties[ PROP_X ];

                        if (prop_x.size() > 0)
                        {
                            if (prop_x[ prop_x.size()-1 ] == '%')
                            {
                                prop_x = prop_x.substr(0, prop_x.size() - 1);
                                widgets[n].m_x = (int)(x + atoi_p(prop_x.c_str())/100.0f * w);
                            }
                            else if(prop_x[ prop_x.size()-1 ] == 'f')
                            {
                                widgets[n].m_x = x + atoi_p(prop_x.c_str()) * GUIEngine::getFontHeight();
                            }
                            else
                            {
                                widgets[n].m_x = x + atoi_p(prop_x.c_str());
                            }
                        }
                        else
                        {
                            widgets[n].m_x = x;
                        }
                    }
                    else if (align == "left")
                    {
                        widgets[n].m_x = x;
                    }
                    else if (align == "center")
                    {
                        widgets[n].m_x = x + w/2 - widgets[n].m_w/2;
                    }
                    else if (align == "right")
                    {
                        widgets[n].m_x = x + w - widgets[n].m_w;
                    }
                    else
                    {
                        Log::warn("LayoutManager::doCalculateLayout",
                            "Alignment '%s' is unknown in widget '%s'",
                            align.c_str(), widgets[n].m_properties[PROP_ID].c_str());
                    }
                    widgets[n].m_y = y;

                    y += widgets[n].m_h;
                }
            } // end if property or absolute size

        } // next widget

    } while(false);

    // ----- also deal with containers' children
    for (int n=0; n<widgets_amount; n++)
    {
        if (widgets[n].m_type == WTYPE_DIV)
        {
            doCalculateLayout(widgets[n].m_children, topLevelContainer, &widgets[n]);
        }
    }
}   // calculateLayout

