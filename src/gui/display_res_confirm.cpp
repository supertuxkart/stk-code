//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008 Paul Elms
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

#include <iostream>
#include <sstream>

#include "user_config.hpp"
#include "sdldrv.hpp"
#include "gui/display_res_confirm.hpp"
#include "gui/menu_manager.hpp"
#include "gui/widget_manager.hpp"
#include "utils/translation.hpp"


#if defined(WIN32) && !defined(__CYGWIN__)
#  define snprintf _snprintf
#endif

enum WidgetTokens
{
    WTOK_TITLE,
    WTOK_APPLY_RES,

    WTOK_QUIT
};


DisplayResConfirm::DisplayResConfirm( const bool FROM_WINDOW_ ) :
    FROM_WINDOW (FROM_WINDOW_)
{
    m_counter = 5; // Number of seconds in which to confirm

    widget_manager->switchOrder();
    widget_manager->addTitleWgt( WTOK_TITLE, 70, 7,
        _("Confirm Resolution Within 5 Seconds"));
    widget_manager->hideWgtRect(WTOK_TITLE);
    
    widget_manager->addEmptyWgt( WidgetManager::WGT_NONE, 40, 2);

    widget_manager->addTextButtonWgt( WTOK_APPLY_RES, 40, 7, _("Confirm Resolution") );

    widget_manager->addEmptyWgt( WidgetManager::WGT_NONE, 40, 2);

    widget_manager->addTextButtonWgt( WTOK_QUIT, 40, 7, _("Press <ESC> to Cancel") );
    widget_manager->setWgtTextSize( WTOK_QUIT, WGT_FNT_SML );

    widget_manager->layout( WGT_AREA_ALL );

    m_timer = SDL_AddTimer(1000,timeout,NULL);
    if (m_timer == NULL)
    {
        std::cerr << "Warning: Timer could not be initialised!\n";
    }
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
        user_config->m_prev_width = user_config->m_width;
        user_config->m_prev_height = user_config->m_height;

        SDL_RemoveTimer(m_timer);
        menu_manager->popMenu();
        break;
    case WTOK_QUIT:
        SDL_RemoveTimer(m_timer);
        if (FROM_WINDOW)
        {
            inputDriver->toggleFullscreen();
            user_config->m_crashed = false;
            user_config->saveConfig();
        }
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
        widget_manager->setWgtText( WTOK_TITLE, m_count );
    }
    else
    {
        SDL_RemoveTimer(m_timer);

        // blacklist the resolution
        std::ostringstream o;
        o << user_config->m_width << "x" << user_config->m_height;
        user_config->m_blacklist_res.push_back (o.str());

        if( FROM_WINDOW )
        {
            inputDriver->toggleFullscreen();
            user_config->m_prev_windowed = false;
            user_config->m_crashed = false;
            user_config->saveConfig();
        }


        menu_manager->popMenu();
    }
}
//-----------------------------------------------------------------------------
void DisplayResConfirm::handle(GameAction ga, int value)
{
    switch ( ga )
    {
    case GA_LEAVE:
        if (value)
            break;
		SDL_RemoveTimer(m_timer);
        if (FROM_WINDOW)
        {
            inputDriver->toggleFullscreen();
            user_config->m_crashed = false;
            user_config->saveConfig();
        }
        menu_manager->popMenu();
        break;

    default:
        BaseGUI::handle(ga, value);
        break;
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

