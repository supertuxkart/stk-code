//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008 Paul Elms
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

#include <iostream>
#include <sstream>

#include "display_res_confirm.hpp"
#include "menu_manager.hpp"
#include "widget_manager.hpp"
#include "translation.hpp"
#include "user_config.hpp"


#if defined(WIN32) && !defined(__CYGWIN__)
#  define snprintf _snprintf
#endif

enum WidgetTokens
{
    WTOK_TITLE,
    
    WTOK_APPLY_RES,
    
    WTOK_EMPTY,
    
    WTOK_EMPTY1,

    WTOK_QUIT
};

DisplayResConfirm::DisplayResConfirm()
{   
	m_counter = 5; // Number of seconds in which to confirm
    
    const bool SHOW_RECT = true;
    const bool SHOW_TEXT = true;
    widget_manager->setInitialRectState(SHOW_RECT, WGT_AREA_ALL, WGT_TRANS_BLACK);
    widget_manager->setInitialTextState(SHOW_TEXT, "", WGT_FNT_MED,
        WGT_FONT_GUI );

    widget_manager->insertColumn();
    widget_manager->addWgt( WTOK_TITLE, 70, 7);
    widget_manager->setWgtText( WTOK_TITLE, _("Confirm Resolution Within 5 Seconds"));

    widget_manager->setInitialActivationState(true);
        
    widget_manager->addWgt( WTOK_EMPTY, 40, 2);
    widget_manager->deactivateWgt( WTOK_EMPTY );
    widget_manager->hideWgtRect( WTOK_EMPTY );
    widget_manager->hideWgtText( WTOK_EMPTY );
    
    widget_manager->addWgt( WTOK_APPLY_RES, 40, 7);
    widget_manager->setWgtText( WTOK_APPLY_RES, _("Confirm Resolution"));
        
    widget_manager->addWgt( WTOK_EMPTY1, 40, 2);
    widget_manager->deactivateWgt( WTOK_EMPTY1 );
    widget_manager->hideWgtRect( WTOK_EMPTY1 );
    widget_manager->hideWgtText( WTOK_EMPTY1 );
        
    widget_manager->addWgt( WTOK_QUIT, 40, 7);
    widget_manager->setWgtText( WTOK_QUIT, _("Press <ESC> to Cancel"));
    widget_manager->setWgtTextSize( WTOK_QUIT, WGT_FNT_SML );

    widget_manager->layout( WGT_AREA_ALL );
    
    m_timer = SDL_AddTimer(1000,timeout,NULL);
    if (m_timer == NULL)
    	std::cerr << "Warning: Timer could not be initialised!" << std::endl;
    
}
    
//-----------------------------------------------------------------------------
DisplayResConfirm::~DisplayResConfirm()
{
    widget_manager->reset();
}

//-----------------------------------------------------------------------------
void DisplayResConfirm::select()
{
    switch ( widget_manager->getSelectedWgt())
    {
	case WTOK_APPLY_RES:
		//set prev resolution to current values to confirm change
		user_config->m_prev_width = user_config->m_width;
		user_config->m_prev_height = user_config->m_height;
		
		// if changing to fullscreen then it has now been confirmed
		// so we need to change m_prev_windowed to confirm the change
		if (user_config->m_fullscreen && user_config->m_prev_windowed)
			user_config->m_prev_windowed = false;
		
		SDL_RemoveTimer(m_timer);
        menu_manager->popMenu();
        break;
    case WTOK_QUIT:
    	SDL_RemoveTimer(m_timer);
    	menu_manager->popMenu();
        break;
    default: break;
    }
}

//-----------------------------------------------------------------------------
void DisplayResConfirm::countdown()
{
	if (m_counter > 1)
	{
		m_counter--;
		snprintf(m_count, MAX_MESSAGE_LENGTH, _("Confirm Resolution Within %d Seconds"), m_counter);
    	widget_manager->setWgtText(WTOK_TITLE, m_count);
	}
	else
	{
		SDL_RemoveTimer(m_timer);
		
		// blacklist the resolution
    	std::ostringstream o;
    	o << user_config->m_width << "x" << user_config->m_height;
    	user_config->m_blacklist_res.push_back (o.str());
    	
		menu_manager->popMenu();
	}
}

//=============================================================================
Uint32 timeout(Uint32 interval, void *param)
{
		SDL_Event event;
        SDL_UserEvent userevent;

        userevent.type = SDL_USEREVENT;
        userevent.code = 0;
        userevent.data1 = NULL;
        userevent.data2 = NULL;

        event.type = SDL_USEREVENT;
        event.user = userevent;

        SDL_PushEvent(&event);
        
		return (interval); 
}

