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
    WTOK_CLEAR_BLACKLIST,

    WTOK_EMPTY,
    WTOK_EMPTY1,
    WTOK_EMPTY2,
    WTOK_EMPTY3,

    WTOK_QUIT
};

ConfigDisplay::ConfigDisplay()
{
    //This is commented because there is no point in checking the resolution
    //when we enter the display configuration every time; this should be done
    //at the start of the program and after the resolution confirm screen.
#if 0
    // The following code comes before the widget code because it may change
    // the fullscreen status, which is used in displaying the WTOK_FULLSCREEN
    // text.

    if( user_config->m_width != user_config->m_prev_width &&
        user_config->m_height != user_config->m_prev_height )
    {
        changeResolution( user_config->m_prev_width, user_config->m_prev_height, true );
    }
    else if( user_config->m_prev_windowed && user_config->m_fullscreen )
    {
        inputDriver->toggleFullscreen();
        user_config->m_prev_windowed = false;
        user_config->m_crashed = false;
        user_config->saveConfig();
    }
    else //no problems detected
    {
        user_config->m_crashed = false;
        user_config->saveConfig();
    }
#endif
    getScreenModes(); //Fill the vector m_sizes with possible resolutions

    m_curr_width = m_sizes[m_curr_res].first;
    m_curr_height = m_sizes[m_curr_res].second;
    
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
    if( isBlacklisted( m_curr_width, m_curr_height ))
    {
        if (!(user_config->m_fullscreen))
        {
//            widget_manager->setWgtText(WTOK_FULLSCREEN, _("Fullscreen Unavailable"));
            widget_manager->hideWgtText(WTOK_FULLSCREEN);
            widget_manager->hideWgtRect(WTOK_FULLSCREEN);
            widget_manager->deactivateWgt(WTOK_FULLSCREEN);
        }
        else
        {
            std::cerr << "Warning: current screen mode is blacklisted.\n";
        }
    }
    else
    {
        if(user_config->m_fullscreen)
        {
            widget_manager->setWgtText( WTOK_FULLSCREEN, _("Window mode"));
        }
        else
        {
            widget_manager->setWgtText( WTOK_FULLSCREEN, _("Fullscreen mode"));
        }
    }

    widget_manager->addWgt( WTOK_EMPTY, 40, 2);
    widget_manager->deactivateWgt( WTOK_EMPTY );
    widget_manager->hideWgtRect( WTOK_EMPTY );
    widget_manager->hideWgtText( WTOK_EMPTY );

    widget_manager->addWgt( WTOK_CURRENT_RES, 40, 7);
    char msg [MAX_MESSAGE_LENGTH];
    snprintf( msg, MAX_MESSAGE_LENGTH, _("Current: %dx%d"), m_curr_width, m_curr_height );
    widget_manager->setWgtText( WTOK_CURRENT_RES, msg );

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

    //Custom resolutions are disabled because, SDL *should* give the all the
    //the available resolutions (i.e. there is no point in using a custom
    //res), and there is the risk of users changing the configuration to a
    //setting that causes a crash.
#if 0
    if (m_curr_res == -1) //A custom res has been set previously that is not in list
    {
        snprintf (m_resolution, MAX_MESSAGE_LENGTH, _("Current: %dx%d"), user_config->m_width, user_config->m_height);
        widget_manager->setWgtText(WTOK_CURRENT_RES, m_resolution);
    }
    else // Find the current res from those in the list
    {}
#endif
}

//-----------------------------------------------------------------------------
ConfigDisplay::~ConfigDisplay()
{
    widget_manager->reset();
}

//-----------------------------------------------------------------------------
void ConfigDisplay::select()
{
    switch ( widget_manager->getSelectedWgt() )
    {
    case WTOK_FULLSCREEN:
        inputDriver->toggleFullscreen();
        if( user_config->m_fullscreen )
        {
            menu_manager->pushMenu( MENUID_RESOLUTION_CONFIRM_WIN );
        }
        else
        {
            //FIXME: maybe instead of 'Fullscreen mode' something like
            //'Switch to fullscreen mode' would be more user friendly?
            widget_manager->setWgtText(WTOK_FULLSCREEN, _("Fullscreen mode"));
        }
        changeApplyButton();
        break;

    case WTOK_INCR_RES:
        {
            const int NUM_RES = (int)m_sizes.size();
            m_curr_res = std::min(NUM_RES - 1, m_curr_res + 1);

            if ( user_config->m_fullscreen &&
                isBlacklisted( m_curr_width, m_curr_height ))
            {
                showBlacklistButtons();
            }
            else changeApplyButton();
        }
        break;

    case WTOK_DECR_RES:
        m_curr_res = std::max(0,m_curr_res-1);
        if ( user_config->m_fullscreen &&
            isBlacklisted( m_curr_width, m_curr_height ))
        {
            showBlacklistButtons();
        }
        else changeApplyButton();
        break;

    case WTOK_APPLY_RES:
        if (m_curr_width != m_sizes[m_curr_res].first ||
            m_curr_height != m_sizes[m_curr_res].second)
        {
            changeResolution(m_sizes[m_curr_res].first,m_sizes[m_curr_res].second/*, false*/);

            if (user_config->m_fullscreen)
            {
                menu_manager->pushMenu(MENUID_RESOLUTION_CONFIRM_FS);
            }
            else
            {
                char msg [MAX_MESSAGE_LENGTH];
                snprintf (msg, MAX_MESSAGE_LENGTH, "Current: %dx%d",
                user_config->m_width, user_config->m_height);
                widget_manager->setWgtText(WTOK_CURRENT_RES, msg);

                if ( isBlacklisted( user_config->m_width,
                    user_config->m_height ))
                {
                    //widget_manager->setWgtText(WTOK_FULLSCREEN, _("Fullscreen Unavailable"));
                    widget_manager->hideWgtText(WTOK_FULLSCREEN);
                    widget_manager->hideWgtRect(WTOK_FULLSCREEN);
                    widget_manager->deactivateWgt(WTOK_FULLSCREEN);
                }
                else
                {
                    widget_manager->setWgtText(WTOK_FULLSCREEN, _("Fullscreen mode"));
                    widget_manager->showWgtText(WTOK_FULLSCREEN);
                    widget_manager->showWgtRect(WTOK_FULLSCREEN);
                    widget_manager->activateWgt(WTOK_FULLSCREEN);
                }
                widget_manager->layout();

                user_config->m_prev_width = m_curr_width = user_config->m_width;
                user_config->m_prev_height = m_curr_height = user_config->m_height;
            }
        }
        break;

    case WTOK_CLEAR_BLACKLIST:
        {
            const int NUM_BLACKLISTED = (int)user_config->m_blacklist_res.size();
            int black_width, black_height = 0;
            int id = -1;

            for ( int i = 0; i < NUM_BLACKLISTED; ++i )
            {
                sscanf(user_config->m_blacklist_res[i].c_str(),
                       "%dx%d",& black_width, & black_height);

                if ( m_sizes[m_curr_res].first == black_width &&
                     m_sizes[m_curr_res].second == black_height )
                {
                    id = i;
                    break;
                }
            }

            if( id != -1 )
            {
                user_config->m_blacklist_res.erase(
                    user_config->m_blacklist_res.begin() + id );
            }
            else
            {
                std::cerr << "Warning: tried to erase a resolution that " <<
                    "is not blacklisted.\n";
            }
        }

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
void ConfigDisplay::changeResolution(int width, int height/*, bool reverse*/)
{
    if (/*!reverse && */user_config->m_fullscreen )
    {
        //store previous width and height
        user_config->m_prev_width = user_config->m_width;
        user_config->m_prev_height = user_config->m_height;
    }

    //change to new height and width
    user_config->m_width = width;
    user_config->m_height = height;

#if 0
    // if returning to prev res, change m_crashed to false as we didn't crash and save config
    if (reverse && user_config->m_fullscreen)
    {
        user_config->m_crashed = false;
        user_config->saveConfig();
    }
#endif

    if (/*!reverse && */user_config->m_fullscreen )
    {
        // Store settings in user config file in case new video mode
        // causes a crash
        user_config->m_crashed = true;
        user_config->saveConfig();
    }

    inputDriver->setVideoMode();
    glViewport(0,0,user_config->m_width, user_config->m_height);
}

/**This gets the available screen resolutions available on the hardware and
 * populates a vector with them.
 */
void ConfigDisplay::getScreenModes()
{
    SDL_Rect **modes = SDL_ListModes( NULL, SDL_OPENGL | SDL_FULLSCREEN | SDL_HWSURFACE );

    if (modes == NULL)
    {
        std::cerr << "No fullscreen modes available.\n";

        loadDefaultModes();

        //FIXME: blacklist all resolutions

#if 0
        //This isn't enabled because SDL_VideoInfo must be called before the
        //first SDL_SetVideoMode() call to get the desktop resolution.

        //Erase any resolutions from the list that are bigger than the
        //current desktop resolution
        const SDL_VideoInfo *VIDEO_INFO = SDL_GetVideoInfo();

        const int NUM_RES = m_sizes.size();
        for (int i = 0; i < NUM_RES; ++i)
        {
            if (m_sizes[i].first > VIDEO_INFO->current_w &&
                m_sizes[i].second > VIDEO_INFO->current_h)
            {
                m_sizes.erase( m_sizes.begin() + i, m_sizes.end() );
                break;
            }
        }
#endif
    }
    else if (modes == (SDL_Rect **)-1) //Any screen size can be used
    {
        loadDefaultModes();
    }
    else
    {
        //modes[i] is used as the breaking condition because that's how SDL's
        //docs use it in their examples.
        for (int i = 0; modes[i]; ++i)
        {
            m_sizes.push_back (std::pair <int, int> (modes[i]->w,
                modes[i]->h));
        }

        std::sort (m_sizes.begin(), m_sizes.end());

        //Prevent use of very small resolutions
        const int MIN_WIDTH = 640;
        const int MIN_HEIGHT = 480;
        const int NUM_RES = (int)m_sizes.size();

        for (int i = NUM_RES - 1; i >= 0; --i)
        {
            if ( m_sizes[i].first < MIN_WIDTH )
            {
                //Remove the resolutions with a width smaller than MIN_WIDTH
                m_sizes.erase( m_sizes.begin(), m_sizes.begin() + i + 1 );
                break;
            }
            else if ( m_sizes[i].first == MIN_WIDTH &&
                m_sizes[i].second < MIN_HEIGHT )
            {
                m_sizes.erase( m_sizes.begin(), m_sizes.begin() + i + 1 );
                break;
            }
        }
    }

    //Set the same resolution as the one in the config file; if it's not
    //found, set it to the lowest resolution available as a sane default.
    m_curr_res = -1;
    const int NUM_RES = (int)m_sizes.size();

    for (int i = 0; i < NUM_RES; ++i)
    {
        if (m_sizes[i].first == user_config->m_width
         && m_sizes[i].second == user_config->m_height)
        {
            m_curr_res = i;
            return;
        }
    }

    m_curr_res = 0;
}

//-----------------------------------------------------------------------------
void ConfigDisplay::changeApplyButton()
{
    // change Apply button text
    char msg [MAX_MESSAGE_LENGTH];
    snprintf(msg, MAX_MESSAGE_LENGTH, _("Apply %dx%d"),
      m_sizes[m_curr_res].first,m_sizes[m_curr_res].second);
    widget_manager->setWgtText(WTOK_APPLY_RES, msg);
    widget_manager->activateWgt(WTOK_APPLY_RES);

    // hide Remove from blacklist button
    widget_manager->hideWgtRect(WTOK_CLEAR_BLACKLIST);
    widget_manager->hideWgtText(WTOK_CLEAR_BLACKLIST);
    widget_manager->deactivateWgt(WTOK_CLEAR_BLACKLIST);
}

//-----------------------------------------------------------------------------
#if 0
int ConfigDisplay::isBlacklisted()
{
    int black_width, black_height = 0;
    for (int i = 0; i < m_blacklist_res_size; i++)
    {
        sscanf(user_config->m_blacklist_res[i].c_str(),
        "%dx%d",& black_width, & black_height);

        if (m_sizes[m_curr_res].first == black_width
            && m_sizes[m_curr_res].second == black_height)
            return i;
    }
    return -1;
}
#endif

//-----------------------------------------------------------------------------
bool ConfigDisplay::isBlacklisted(int width, int height)
{
    int black_width, black_height;
    const int NUM_BLACKLISTED = (int)user_config->m_blacklist_res.size();

    for (int i = 0; i < NUM_BLACKLISTED; ++i)
    {
        sscanf(user_config->m_blacklist_res[i].c_str(),
        "%dx%d", &black_width, &black_height );

        if (width == black_width && height == black_height) return true;
    }

    return false;

}

//-----------------------------------------------------------------------------
void ConfigDisplay::showBlacklistButtons()
{
    //change Apply button to Blacklisted button
    char msg [MAX_MESSAGE_LENGTH];
    snprintf(msg, MAX_MESSAGE_LENGTH, _("%dx%d Blacklisted"),
      m_sizes[m_curr_res].first,m_sizes[m_curr_res].second);
    widget_manager->setWgtText(WTOK_APPLY_RES, msg);
    widget_manager->deactivateWgt(WTOK_APPLY_RES);

    //show Remove from blacklist button
    widget_manager->showWgtRect( WTOK_CLEAR_BLACKLIST);
    widget_manager->showWgtText( WTOK_CLEAR_BLACKLIST);
    widget_manager->activateWgt( WTOK_CLEAR_BLACKLIST);
}

/** loadDefaultModes() populates our list of resolutios manually, sorted from
 *  smallest to biggest, first on the width, then the height. Useful when
 *  no fullscreen resolutions are available or when any resolution is
 *  available. The list of resolutions is taken from
 *  http://www.tamingthebeast.net/blog/web-development/screen-resolution-statistics-0907.htm
 */
void ConfigDisplay::loadDefaultModes()
{
    m_sizes.clear();

    m_sizes.push_back( std::pair <int, int> (800, 600) );
    m_sizes.push_back( std::pair <int, int> (1024, 768) );
    m_sizes.push_back( std::pair <int, int> (1152, 864) );
    m_sizes.push_back( std::pair <int, int> (1280, 768) );
    m_sizes.push_back( std::pair <int, int> (1280, 800) );
    m_sizes.push_back( std::pair <int, int> (1280, 960) );
    m_sizes.push_back( std::pair <int, int> (1280, 1024) );
    m_sizes.push_back( std::pair <int, int> (1440, 900) );
    m_sizes.push_back( std::pair <int, int> (1680, 1050) );
    m_sizes.push_back( std::pair <int, int> (1920, 1200) );
};

