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

enum WidgetTokens
{
    WTOK_TITLE,

    WTOK_FULLSCREEN,
    
    WTOK_INCR_RES,
    
    WTOK_DECR_RES,
    
    WTOK_CURRENT_RES,
    
    WTOK_APPLY_RES,

    WTOK_EMPTY,
    
    WTOK_EMPTY1,
    
    WTOK_EMPTY2,
    
    WTOK_EMPTY3,

    WTOK_QUIT,
    
    WTOK_CLEAR_BLACKLIST
};

ConfigDisplay::ConfigDisplay()
{  
    const bool SHOW_RECT = true;
    const bool SHOW_TEXT = true;
    widget_manager->setInitialRectState(SHOW_RECT, WGT_AREA_ALL, WGT_TRANS_BLACK);
    widget_manager->setInitialTextState(SHOW_TEXT, "", WGT_FNT_MED,
        WGT_FONT_GUI );

    widget_manager->insertColumn();
    widget_manager->addWgt( WTOK_TITLE, 40, 7);
    widget_manager->setWgtText( WTOK_TITLE, _("Display Settings"));

    widget_manager->setInitialActivationState(true);
    widget_manager->addWgt( WTOK_FULLSCREEN, 40, 7);
    if(user_config->m_fullscreen)
    {
        widget_manager->setWgtText( WTOK_FULLSCREEN, _("Window mode"));
    }
    else
    {
        widget_manager->setWgtText( WTOK_FULLSCREEN, _("Fullscreen mode"));
    }
    
    widget_manager->addWgt( WTOK_EMPTY, 40, 2);
    widget_manager->deactivateWgt( WTOK_EMPTY );
    widget_manager->hideWgtRect( WTOK_EMPTY );
    widget_manager->hideWgtText( WTOK_EMPTY );
    
    widget_manager->addWgt( WTOK_CURRENT_RES, 40, 7);
    widget_manager->setWgtText( WTOK_CURRENT_RES, _("Current: ****x****"));
    
    widget_manager->addWgt( WTOK_INCR_RES, 40, 7);
    widget_manager->setWgtText( WTOK_INCR_RES, _("Increase Resolution"));
        
    widget_manager->addWgt( WTOK_DECR_RES, 40, 7);
    widget_manager->setWgtText( WTOK_DECR_RES, _("Decrease Resolution"));
       
    widget_manager->addWgt( WTOK_EMPTY2, 40, 2);
    widget_manager->deactivateWgt( WTOK_EMPTY2 );
    widget_manager->hideWgtRect( WTOK_EMPTY2 );
    widget_manager->hideWgtText( WTOK_EMPTY2 );
    
    widget_manager->addWgt( WTOK_APPLY_RES, 40, 7);
    widget_manager->setWgtText( WTOK_APPLY_RES, _("Apply "));
    
    if (!user_config->m_blacklist_res.empty())
    {
    	widget_manager->addWgt( WTOK_EMPTY3, 40, 2);
    	widget_manager->deactivateWgt( WTOK_EMPTY3 );
    	widget_manager->hideWgtRect( WTOK_EMPTY3 );
    	widget_manager->hideWgtText( WTOK_EMPTY3 );
    
    	widget_manager->addWgt( WTOK_CLEAR_BLACKLIST, 55, 7);
    	widget_manager->setWgtText( WTOK_CLEAR_BLACKLIST, _("Clear Resolution Blacklist"));
    }
        
    widget_manager->addWgt( WTOK_EMPTY1, 40, 7);
    widget_manager->deactivateWgt( WTOK_EMPTY1 );
    widget_manager->hideWgtRect( WTOK_EMPTY1 );
    widget_manager->hideWgtText( WTOK_EMPTY1 );
    
    widget_manager->addWgt( WTOK_QUIT, 40, 7);
    widget_manager->setWgtText( WTOK_QUIT, _("Press <ESC> to go back"));
    widget_manager->setWgtTextSize( WTOK_QUIT, WGT_FNT_SML );

	//if prev resolution different to current res then a resolution change has been rejected
	if (user_config->m_width != user_config->m_prev_width 
	    && user_config->m_height != user_config->m_prev_height)
	  	{
	  		changeResolution(user_config->m_prev_width,
	  		user_config->m_prev_height,true);
	  	}
    
    widget_manager->layout( WGT_AREA_ALL );
    
    //get current resolution and set wgt txt
    getScreenModes();  //Populate a list with possible resolutions
    if (m_sizes_index == -1) //A custom res has been set previously that is not in list
    { 
        snprintf (m_resolution, MAX_MESSAGE_LENGTH, _("Current: %dx%d"), user_config->m_width, user_config->m_height);
        widget_manager->setWgtText(WTOK_CURRENT_RES, m_resolution);
    }
    else // Find the current res from those in the list
    {
        m_curr_width = m_sizes[m_sizes_index].first;
        m_curr_height = m_sizes[m_sizes_index].second;
        snprintf(m_resolution, MAX_MESSAGE_LENGTH, _("Current: %dx%d"),m_curr_width,m_curr_height);
        widget_manager->setWgtText(WTOK_CURRENT_RES, m_resolution);
    }
    
    // Set crashed flag to false (needed after confirming change to fullscreen)
    user_config->m_crashed = false;  //if got here,then fullscreen change didn't crash STK 
    user_config->saveConfig();
}

//-----------------------------------------------------------------------------
ConfigDisplay::~ConfigDisplay()
{
    widget_manager->reset();
}

//-----------------------------------------------------------------------------
void ConfigDisplay::select()
{
    switch ( widget_manager->getSelectedWgt())
    {
    case WTOK_FULLSCREEN:
        drv_toggleFullscreen();
        if (user_config->m_fullscreen)
        	menu_manager->pushMenu(MENUID_DISPLAY_RES_CONFIRM);
        break;
    case WTOK_INCR_RES:
        m_sizes_index = std::min(m_sizes_size-1,m_sizes_index+1);
        snprintf(m_resolution, MAX_MESSAGE_LENGTH, _("Apply %dx%d"),m_sizes[m_sizes_index].first,m_sizes[m_sizes_index].second);
        widget_manager->setWgtText(WTOK_APPLY_RES, m_resolution);
        break;
    case WTOK_DECR_RES:
        m_sizes_index = std::max(0,m_sizes_index-1);
        snprintf(m_resolution, MAX_MESSAGE_LENGTH, _("Apply %dx%d"),m_sizes[m_sizes_index].first,m_sizes[m_sizes_index].second);
        widget_manager->setWgtText(WTOK_APPLY_RES, m_resolution);
        break;
    case WTOK_APPLY_RES:
    	if (m_curr_width != m_sizes[m_sizes_index].first 
    		|| m_curr_height != m_sizes[m_sizes_index].second)  //Only allow Apply if a new res has been selected
    	{
        	changeResolution(m_sizes[m_sizes_index].first,m_sizes[m_sizes_index].second);
        	
        	if (user_config->m_fullscreen)
        		menu_manager->pushMenu(MENUID_DISPLAY_RES_CONFIRM);
        	else
        	{
        		snprintf (m_resolution, MAX_MESSAGE_LENGTH, "Current: %dx%d",
        		user_config->m_width, user_config->m_height);
        		widget_manager->setWgtText(WTOK_CURRENT_RES, m_resolution);
        		widget_manager->layout();
        		// set prev_width and height values to current
        		user_config->m_prev_width = m_curr_width = user_config->m_width;
				user_config->m_prev_height = m_curr_height = user_config->m_height;
				  
        	}
        }
        break;
    case WTOK_CLEAR_BLACKLIST:
    	user_config->m_blacklist_res.clear();
    	widget_manager->hideWgtRect( WTOK_CLEAR_BLACKLIST );
    	widget_manager->hideWgtText( WTOK_CLEAR_BLACKLIST );
    	widget_manager->layout();
    	break;
    case WTOK_QUIT:
        menu_manager->popMenu();
        break;
    default: break;
    }
}

//-----------------------------------------------------------------------------
void ConfigDisplay::changeResolution(int width, int height, bool reverse)
{
    if (!reverse && user_config->m_fullscreen) //  don't store previous res if returning to it
    											// or if not in fullscreen mode
    {
    	//store previous width and height
    	user_config->m_prev_width = user_config->m_width;
    	user_config->m_prev_height = user_config->m_height;
    }
    
    //change to new height and width
    user_config->m_width = width;
    user_config->m_height = height;
    
    if (!reverse && user_config->m_fullscreen)
    {
    // Store settings in user config file in case new video mode
    // causes a crash
    	user_config->m_crashed = true; //set flag. 
    	user_config->saveConfig();
    }
                 
    setVideoMode();
        
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
        
    //reassess m_sizes_size
    m_sizes_size = (int)m_sizes.size();
            
    //Remove blacklisted resolutions from the list
    if (!user_config->m_blacklist_res.empty())
    {
        int blacklist_res_size = (int)user_config->m_blacklist_res.size();
        int black_width, black_height = 0;
        for (int i = 0; i < blacklist_res_size; i++)
        {
	        sscanf(user_config->m_blacklist_res[i].c_str(),
			"%dx%d",& black_width, & black_height);
			
			for (int i = m_sizes_size-1; i >=0; i--)
			{
				if (m_sizes[i].first == black_width 
					&& m_sizes[i].second == black_height)
					{
						m_sizes.erase(m_sizes.begin()+i);
					}
			}
		}
	}
        
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
