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
    // The following code comes before the widget code because it may change 
    // the fullscreen status, which is used in displaying the WTOK_FULLSCREEN 
    // text.
      
    // if prev resolution different to current res then a resolution change 
    // has been rejected
	if (user_config->m_width != user_config->m_prev_width 
	    && user_config->m_height != user_config->m_prev_height)
	  	{
	  		changeResolution(user_config->m_prev_width,
	  		user_config->m_prev_height,true);
	  	}
	  	
    // if m_prev_windowed is true and m_fullscreen is true then a change to 
    // fullscreen has been rejected
    else if (user_config->m_prev_windowed && user_config->m_fullscreen)
    {
    	drv_toggleFullscreen();
    	user_config->m_prev_windowed = false;  //reset flags
    	user_config->m_crashed = false;
    	user_config->saveConfig();
    }
    else
    {
    	user_config->m_crashed = false;  //if we are here we didn't crash
    	user_config->saveConfig();
    }
    
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
    
    widget_manager->addWgt( WTOK_EMPTY3, 40, 2);
    widget_manager->deactivateWgt( WTOK_EMPTY3 );
    widget_manager->hideWgtRect( WTOK_EMPTY3 );
    widget_manager->hideWgtText( WTOK_EMPTY3 );
    
    widget_manager->addWgt( WTOK_CLEAR_BLACKLIST, 40, 7);
    widget_manager->setWgtText( WTOK_CLEAR_BLACKLIST, _("Clear from Blacklist"));
    widget_manager->deactivateWgt( WTOK_CLEAR_BLACKLIST);	
	widget_manager->hideWgtRect( WTOK_CLEAR_BLACKLIST);
	widget_manager->hideWgtText( WTOK_CLEAR_BLACKLIST);
    
        
    widget_manager->addWgt( WTOK_EMPTY1, 40, 7);
    widget_manager->deactivateWgt( WTOK_EMPTY1 );
    widget_manager->hideWgtRect( WTOK_EMPTY1 );
    widget_manager->hideWgtText( WTOK_EMPTY1 );
    
    widget_manager->addWgt( WTOK_QUIT, 40, 7);
    widget_manager->setWgtText( WTOK_QUIT, _("Press <ESC> to go back"));
    widget_manager->setWgtTextSize( WTOK_QUIT, WGT_FNT_SML );
    
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
    
    m_blacklist_res_size = 0; 
    if (!user_config->m_blacklist_res.empty())
        {
        	m_blacklist_res_size = (int)user_config->m_blacklist_res.size();
        	if (onBlacklist(m_curr_width,m_curr_height) >= 0)  // check current res against blacklist
			{
				// if in windowed mode make fullscreen unavailable
				if (!user_config->m_fullscreen)
    			{
    				widget_manager->setWgtText(WTOK_FULLSCREEN, _("Fullscreen Unavailable"));
					widget_manager->deactivateWgt(WTOK_FULLSCREEN);
    			}
    			else
    				showBlacklistButtons(); // change widgets to blacklisted mode
			}
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
    switch ( widget_manager->getSelectedWgt())
    {
    case WTOK_FULLSCREEN:
    	drv_toggleFullscreen();
        if (user_config->m_fullscreen)
       	{
       		user_config->m_prev_windowed = true;
       		menu_manager->pushMenu(MENUID_DISPLAY_RES_CONFIRM);
       	}
       	else
       		widget_manager->setWgtText(WTOK_FULLSCREEN, _("Fullscreen mode"));
       		changeApplyButton();
		break;
    case WTOK_INCR_RES:
        m_sizes_index = std::min(m_sizes_size-1,m_sizes_index+1);
        //if there is a blacklist and we are fullscreen, inform user about blacklisted res
        if (!user_config->m_blacklist_res.empty() && user_config->m_fullscreen)
        {
        	if (onBlacklist() >= 0)  // check new res against blacklist
			{
				showBlacklistButtons();  // change widgets to blacklisted mode 
			}
			else
				changeApplyButton();
        }
        else
        changeApplyButton(); // change text on Apply button
        break;
    case WTOK_DECR_RES:
        m_sizes_index = std::max(0,m_sizes_index-1);
        //if there is a blacklist and we are fullscreen, inform user about blacklisted res
        if (!user_config->m_blacklist_res.empty() && user_config->m_fullscreen)
		{	
			if (onBlacklist() >= 0)  // check new res against blacklist
			{
				showBlacklistButtons();  // change widgets to blacklisted mode 
			}
			else
				changeApplyButton();
		} 
        else
        changeApplyButton(); // change text on Apply button
        break;
    case WTOK_APPLY_RES:
    	if (m_curr_width != m_sizes[m_sizes_index].first 
    		|| m_curr_height != m_sizes[m_sizes_index].second)  //Only allow Apply if a new res has been selected
    	{
        	changeResolution(m_sizes[m_sizes_index].first,m_sizes[m_sizes_index].second);
        	
        	if (user_config->m_fullscreen)  // if in fullscreen seek confirmation
        		menu_manager->pushMenu(MENUID_DISPLAY_RES_CONFIRM);
        	else
        	{
        		snprintf (m_resolution, MAX_MESSAGE_LENGTH, "Current: %dx%d",
        		user_config->m_width, user_config->m_height);
        		widget_manager->setWgtText(WTOK_CURRENT_RES, m_resolution);
        		// Check new res against blacklist to determine if fullscreen is an option
        		if (!user_config->m_blacklist_res.empty())
        		{
        			if (onBlacklist(user_config->m_width, user_config->m_height) >= 0)
        			{
        				widget_manager->setWgtText(WTOK_FULLSCREEN, _("Fullscreen Unavailable"));
						widget_manager->deactivateWgt(WTOK_FULLSCREEN);
        			}
        			else
        			{
        				widget_manager->setWgtText(WTOK_FULLSCREEN, _("Fullscreen mode"));
						widget_manager->activateWgt(WTOK_FULLSCREEN);
        			}
        		}
        		widget_manager->layout();
        		// set prev_width and height values to current
        		user_config->m_prev_width = m_curr_width = user_config->m_width;
				user_config->m_prev_height = m_curr_height = user_config->m_height;
				  
        	}
        }
        break;
    case WTOK_CLEAR_BLACKLIST:
    	user_config->m_blacklist_res.erase(user_config->m_blacklist_res.begin()
    	 + onBlacklist());
    	
    	if (!user_config->m_blacklist_res.empty())
    		m_blacklist_res_size = (int)user_config->m_blacklist_res.size();
    	else
    	m_blacklist_res_size = 0;
    	 
    	changeApplyButton();    	
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
    
    // if returning to prev res, change m_crashed to false as we didn't crash and save config
    if (reverse && user_config->m_fullscreen)
    {
    	user_config->m_crashed = false;
    	user_config->saveConfig();
    }
    
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
    
        
    //reassess m_sizes_size
    m_sizes_size = (int)m_sizes.size();
        
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

//-----------------------------------------------------------------------------
void ConfigDisplay::changeApplyButton()
{
	// change Apply button text
	snprintf(m_resolution, MAX_MESSAGE_LENGTH, _("Apply %dx%d"),
	  m_sizes[m_sizes_index].first,m_sizes[m_sizes_index].second);
    widget_manager->setWgtText(WTOK_APPLY_RES, m_resolution); 
    widget_manager->activateWgt(WTOK_APPLY_RES); 
   
    // hide Remove from blacklist button
    widget_manager->hideWgtRect(WTOK_CLEAR_BLACKLIST);
    widget_manager->hideWgtText(WTOK_CLEAR_BLACKLIST);
    widget_manager->deactivateWgt(WTOK_CLEAR_BLACKLIST);
}

//-----------------------------------------------------------------------------
int ConfigDisplay::onBlacklist()
{
	int black_width, black_height = 0;
    for (int i = 0; i < m_blacklist_res_size; i++)
    {
	    sscanf(user_config->m_blacklist_res[i].c_str(),
		"%dx%d",& black_width, & black_height);
			
		if (m_sizes[m_sizes_index].first == black_width 
			&& m_sizes[m_sizes_index].second == black_height)
			return i;
	}
	return -1;
}

//-----------------------------------------------------------------------------
int ConfigDisplay::onBlacklist(int width, int height)
{
	int black_width, black_height = 0;
    for (int i = 0; i < m_blacklist_res_size; i++)
    {
	    sscanf(user_config->m_blacklist_res[i].c_str(),
		"%dx%d",& black_width, & black_height);
			
		if (width == black_width && height == black_height)
			return i;
	}
	return -1;

}

//-----------------------------------------------------------------------------
void ConfigDisplay::showBlacklistButtons()
{
	//change Apply button to Blacklisted button
	snprintf(m_resolution, MAX_MESSAGE_LENGTH, _("%dx%d Blacklisted"),
	  m_sizes[m_sizes_index].first,m_sizes[m_sizes_index].second);
    widget_manager->setWgtText(WTOK_APPLY_RES, m_resolution);
    widget_manager->deactivateWgt(WTOK_APPLY_RES);
    
    //show Remove from blacklist button
    widget_manager->showWgtRect( WTOK_CLEAR_BLACKLIST);
    widget_manager->showWgtText( WTOK_CLEAR_BLACKLIST);
    widget_manager->activateWgt( WTOK_CLEAR_BLACKLIST);
    	
}
