//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 Marianne Gagnon
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

#include <sstream>

#include "irrlicht.h"
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

#include "graphics/irr_driver.hpp"
#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/widget.hpp"
#include "io/file_manager.hpp"
#include "utils/ptr_vector.hpp"
#include "utils/string_utils.hpp"

using namespace GUIEngine;

#ifndef round
# define round(x)  (floor(x+0.5f))
#endif

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
        fprintf(stderr, "[LayoutManager] WARNING: Invalid value '%s' found in XML file where integer was expected\n", val);
        return 0;
    }
}

// ----------------------------------------------------------------------------

bool LayoutManager::convertToCoord(std::string& x, int* absolute /* out */, int* percentage /* out */)
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


// ----------------------------------------------------------------------------

void LayoutManager::readCoords(Widget* self, AbstractTopLevelContainer* topLevelContainer, Widget* parent)
{    
    // determine widget position and size if not already done by sizers
    std::string x      = self->m_properties[PROP_X];
    std::string y      = self->m_properties[PROP_Y];
    std::string width  = self->m_properties[PROP_WIDTH];
    std::string height = self->m_properties[PROP_HEIGHT];
    
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
    
    // ---- try converting to number
    // x coord
    {
        int abs_x = -1, percent_x = -1;
        if(convertToCoord(x, &abs_x, &percent_x ))
        {
            if      (abs_x > -1)     self->m_x = parent_x + abs_x;
            else if (abs_x < -1)     self->m_x = parent_x + (parent_w + abs_x);
            else if (percent_x > -1) self->m_x = parent_x + parent_w*percent_x/100;
        }
    }
    
    // y coord
    {
        int abs_y = -1, percent_y = -1;
        if(convertToCoord(y, &abs_y, &percent_y ))
        {
            if      (abs_y > -1)     self->m_y = parent_y + abs_y;
            else if (abs_y < -1)     self->m_y = parent_y + (parent_h + abs_y);
            else if (percent_y > -1) self->m_y = parent_y + parent_h*percent_y/100;
        }
    }
    
    // ---- if this widget has an icon, get icon size. this can helpful determine its optimal size
    int texture_w = -1, texture_h = -1;
    
    if(self->m_properties[PROP_ICON].size() > 0)
    {
        ITexture* texture = irr_driver->getTexture((file_manager->getDataDir() + "/" +
                                                    self->m_properties[PROP_ICON]).c_str());
        
        if(texture != NULL)
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
    if (label_h == -1)
    {
        if (self->getType() == WTYPE_TEXTBOX ||
            self->getType() == WTYPE_BUTTON ||
            self->getType() == WTYPE_LABEL)
        {
            IGUIFont* font = (self->m_title_font ? GUIEngine::getTitleFont() : GUIEngine::getFont());

            // get text height, a text box, button or label is always as high as the text it could contain
            core::dimension2d< u32 > dim = font->getDimension( L"X" );
            label_h = dim.Height + self->getHeightNeededAroundLabel();
        }
    }
    
    // ---- read dimension
    // width
    {
        int abs_w = -1, percent_w = -1;
        if(convertToCoord(width, &abs_w, &percent_w ))
        {
            if      (abs_w > -1)     self->m_w = abs_w;
            else if (percent_w > -1) self->m_w = (int)round(parent_w*percent_w/100.0);
        }
        else if(texture_w > -1) self->m_w = texture_w;
        else if(label_w > -1)   self->m_w = label_w;
    }
    
    // height
    {
        int abs_h = -1, percent_h = -1;
        if (convertToCoord(height, &abs_h, &percent_h ))
        {
            if      (abs_h > -1)     self->m_h = abs_h;
            else if (percent_h > -1) self->m_h = parent_h*percent_h/100;
        }
        else if (texture_h > -1 && label_h > -1) self->m_h = texture_h + label_h; // label + icon
        else if (texture_h > -1)                 self->m_h = texture_h;
        else if (label_h > -1)                   self->m_h = label_h;
    }
    
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

void LayoutManager::calculateLayout(ptr_vector<Widget>& widgets, AbstractTopLevelContainer* topLevelContainer,
                                    Widget* parent)
{
    const unsigned short widgets_amount = widgets.size();
    
    // ----- read x/y/size parameters
    for (unsigned short n=0; n<widgets_amount; n++)
    {
        readCoords(widgets.get(n), topLevelContainer, parent);
    }//next widget        
    
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
            std::cerr << "Unknown layout name : " << layout_name.c_str() << std::endl;
            break;
        }
        
        const int w = parent->m_w, h = parent->m_h;
        
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
        
        // ---- lay widgets in row
        int x = parent->m_x;
        int y = parent->m_y;
        for (int n=0; n<widgets_amount; n++)
        {
            std::string prop = widgets[n].m_properties[ PROP_PROPORTION ];
            if(prop.size() != 0)
            {
                // proportional size
                int proportion = 1;
                std::istringstream myStream(prop);
                if (!(myStream >> proportion))
                {
                    std::cerr << "/!\\ Warning /!\\ : proportion  '" << prop.c_str()
                              << "' is not a number for widget " << widgets[n].m_properties[PROP_ID].c_str()
                              << std::endl;
                }
                
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
                        std::cerr  << "/!\\ Warning /!\\ : alignment  '" << align.c_str()
                                   <<  "' is unknown (widget '" << widgets[n].m_properties[PROP_ID].c_str()
                                   << "', in a horiozntal-row layout)\n";
                    }
                    
                    widgets[n].m_w = (int)(left_space*fraction);
                    if (widgets[n].m_properties[PROP_MAX_WIDTH].size() > 0)
                    {
                        const int max_width = atoi_p( widgets[n].m_properties[PROP_MAX_WIDTH].c_str() );
                        if (widgets[n].m_w > max_width) widgets[n].m_w = max_width;
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
                        std::cerr << "/!\\ Warning /!\\ : alignment  '" << align.c_str()
                                  <<  "' is unknown (widget '" << widgets[n].m_properties[PROP_ID].c_str()
                                  << "', in a vertical-row layout)\n";
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
                        std::cerr << "/!\\ Warning /!\\ : alignment  '" << align.c_str()
                                  << "' is unknown in widget " << widgets[n].m_properties[PROP_ID].c_str()
                                  << std::endl;
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
                        std::cerr << "/!\\ Warning /!\\ : alignment  '" << align.c_str()
                                  << "' is unknown in widget " << widgets[n].m_properties[PROP_ID].c_str() << std::endl;
                    }
                    widgets[n].m_y = y;
                    
                    y += widgets[n].m_h;
                }
            } // end if property or absolute size
            
        } // next widget
        
    } while(false);
    
    // ----- also deal with containers' children
    for(int n=0; n<widgets_amount; n++)
    {
        if(widgets[n].m_type == WTYPE_DIV) calculateLayout(widgets[n].m_children, topLevelContainer, &widgets[n]);
    }
}   // calculateLayout

