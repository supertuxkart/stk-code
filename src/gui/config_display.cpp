//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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

#include "config_display.hpp"
#include "widget_manager.hpp"
#include "user_config.hpp"
#include "menu_manager.hpp"
#include "sdldrv.hpp"
#include "translation.hpp"

#include <algorithm>

#if defined(WIN32) && !defined(__CYGWIN__)
#  define snprintf _snprintf
#endif

enum WidgetTokens {
    WTOK_TITLE,

    WTOK_FULLSCREEN,
    
    WTOK_INCR_RES,
    
    WTOK_DECR_RES,
    
    WTOK_CURRENT_RES,
    
    WTOK_APPLY_RES,

    WTOK_EMPTY,
    
    WTOK_EMPTY1,
    
    WTOK_EMPTY2,

    WTOK_QUIT
};

ConfigDisplay::ConfigDisplay()
{  
    const bool SHOW_RECT = true;
    const bool SHOW_TEXT = true;
    widget_manager->set_initial_rect_state(SHOW_RECT, WGT_AREA_ALL, WGT_TRANS_BLACK);
    widget_manager->set_initial_text_state(SHOW_TEXT, "", WGT_FNT_MED );

    widget_manager->insert_column();
    widget_manager->add_wgt( WTOK_TITLE, 40, 7);
    widget_manager->set_wgt_text( WTOK_TITLE, _("Display Settings"));

    widget_manager->set_initial_activation_state(true);
    widget_manager->add_wgt( WTOK_FULLSCREEN, 40, 7);
    if(user_config->m_fullscreen)
    {
        widget_manager->set_wgt_text( WTOK_FULLSCREEN, _("Window mode"));
    }
    else
    {
        widget_manager->set_wgt_text( WTOK_FULLSCREEN, _("Fullscreen mode"));
    }
    
    widget_manager->add_wgt( WTOK_EMPTY, 40, 2);
    widget_manager->deactivate_wgt( WTOK_EMPTY );
    widget_manager->hide_wgt_rect( WTOK_EMPTY );
    widget_manager->hide_wgt_text( WTOK_EMPTY );
    
    widget_manager->add_wgt( WTOK_CURRENT_RES, 40, 7);
    widget_manager->set_wgt_text( WTOK_CURRENT_RES, _("Current: ****x****"));
    
    widget_manager->add_wgt( WTOK_INCR_RES, 40, 7);
    widget_manager->set_wgt_text( WTOK_INCR_RES, _("Increase Resolution"));
        
    widget_manager->add_wgt( WTOK_DECR_RES, 40, 7);
    widget_manager->set_wgt_text( WTOK_DECR_RES, ("Decrease Resolution"));
       
       widget_manager->add_wgt( WTOK_EMPTY2, 40, 2);
    widget_manager->deactivate_wgt( WTOK_EMPTY2 );
    widget_manager->hide_wgt_rect( WTOK_EMPTY2 );
    widget_manager->hide_wgt_text( WTOK_EMPTY2 );
    
    widget_manager->add_wgt( WTOK_APPLY_RES, 40, 7);
    widget_manager->set_wgt_text( WTOK_APPLY_RES, ("Apply "));
        
    widget_manager->add_wgt( WTOK_EMPTY1, 40, 7);
    widget_manager->deactivate_wgt( WTOK_EMPTY1 );
    widget_manager->hide_wgt_rect( WTOK_EMPTY1 );
    widget_manager->hide_wgt_text( WTOK_EMPTY1 );
    
    widget_manager->add_wgt( WTOK_QUIT, 40, 7);
    widget_manager->set_wgt_text( WTOK_QUIT, _("Press <ESC> to go back"));
    widget_manager->set_wgt_text_size( WTOK_QUIT, WGT_FNT_SML );

    widget_manager->layout( WGT_AREA_ALL );
    
    //get current resolution and set wgt txt
    getScreenModes();
    if (m_sizes_index == -1)
    { 
        snprintf (m_resolution, MAX_MESSAGE_LENGTH, "Current: %dx%d", user_config->m_width, user_config->m_height);
        widget_manager->set_wgt_text(WTOK_CURRENT_RES, m_resolution);
    }
    else
    {
        snprintf(m_resolution, MAX_MESSAGE_LENGTH, "Current: %dx%d",m_sizes[m_sizes_index].first,m_sizes[m_sizes_index].second);
        widget_manager->set_wgt_text(WTOK_CURRENT_RES, m_resolution);
    }
}

//-----------------------------------------------------------------------------
ConfigDisplay::~ConfigDisplay()
{
    widget_manager->reset();
}

//-----------------------------------------------------------------------------
void ConfigDisplay::select()
{
    switch ( widget_manager->get_selected_wgt())
    {
    case WTOK_FULLSCREEN:
        drv_toggleFullscreen();
        if(user_config->m_fullscreen)
        {
            widget_manager->set_wgt_text( WTOK_FULLSCREEN, _("Window mode"));
           }
        else
        {
            widget_manager->set_wgt_text( WTOK_FULLSCREEN, _("Fullscreen mode"));
        }
        widget_manager->layout();
        break;
    case WTOK_INCR_RES:
        m_sizes_index = std::min(m_sizes_size-1,m_sizes_index+1);
        snprintf(m_resolution, MAX_MESSAGE_LENGTH, "Apply %dx%d",m_sizes[m_sizes_index].first,m_sizes[m_sizes_index].second);
        widget_manager->set_wgt_text(WTOK_APPLY_RES, m_resolution);
        break;
      case WTOK_DECR_RES:
        m_sizes_index = std::max(0,m_sizes_index-1);
        snprintf(m_resolution, MAX_MESSAGE_LENGTH, "Apply %dx%d",m_sizes[m_sizes_index].first,m_sizes[m_sizes_index].second);
        widget_manager->set_wgt_text(WTOK_APPLY_RES, m_resolution);
        break;
      case WTOK_APPLY_RES:
        changeResolution(m_sizes[m_sizes_index].first,m_sizes[m_sizes_index].second);
        snprintf (m_resolution, MAX_MESSAGE_LENGTH, "Current: %dx%d", user_config->m_width, user_config->m_height);
        widget_manager->set_wgt_text(WTOK_CURRENT_RES, m_resolution);
        
        break;
   
    case WTOK_QUIT:
        menu_manager->popMenu();
        break;
    default: break;
    }
}

//-----------------------------------------------------------------------------
void ConfigDisplay::changeResolution(int width, int height)
{
    user_config->m_width = width;
    user_config->m_height = height;
                 
    setVideoMode();
        
    widget_manager->layout();
        
    glViewport(0,0,user_config->m_width, user_config->m_height);
    glScissor(0,0,user_config->m_width, user_config->m_height);
}

//-----------------------------------------------------------------------------
//This gets the available screen resolutions available on the hardware and 
//populates a vector with them.
void ConfigDisplay::getScreenModes()
{
    if (m_sizes.empty()) //has this data been collected before
    {
        SDL_Rect **modes = SDL_ListModes(NULL, SDL_OPENGL | SDL_FULLSCREEN | SDL_HWSURFACE );
        
        //Check if any modes are available
            if (!modes)
        {
            std::cerr << "No Screen Modes available" <<std::endl;
        }
           else if (modes == (SDL_Rect **)-1) 
        {
            //This means all modes are available..Shouldn't happen.
            std::cerr << "All modes available" << std::endl;
        }
        else
        {
            for (int i = 0; modes[i]; ++i)
            m_sizes.push_back (std::pair <int, int> (modes[i]->w, modes[i]->h));
        
            //Sort the entries
            sort (m_sizes.begin(), m_sizes.end());
            
            //Prevent use of very small resolutions
            const int minRes = 640;
            m_sizes_size = (int)m_sizes.size();
            
            for (int i = m_sizes_size-1; i >= 0; --i)
            {
                if (m_sizes[i].first < minRes) //find largest width less than minRes
                {
                    m_sizes.erase(m_sizes.begin(),m_sizes.begin()+i+1); //remove all resolutions prior
                    break;
                }
            }
        }
    }// m_sizes.empty()
    
    // search m_sizes for the current resolution
    m_sizes_index = -1;
    m_sizes_size = (int)m_sizes.size();
    for (int i = 0; i < m_sizes_size; i++)
    {
        if (m_sizes[i].first == user_config->m_width 
         && m_sizes[i].second == user_config->m_height)
        {
            m_sizes_index = i;
            break;
        }
    }
     
    
}
