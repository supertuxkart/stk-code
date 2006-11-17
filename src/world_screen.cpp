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

#include <SDL/SDL.h>

#include "world.hpp"
#include "widget_set.hpp"
#include "world_screen.hpp"
#include "sound_manager.hpp"
#include "camera.hpp"
#include "config.hpp"
#include "track_manager.hpp"
#include "track.hpp"
#include "gui/menu_manager.hpp"
#include "history.hpp"

WorldScreen* WorldScreen::m_current_ = 0;

WorldScreen::WorldScreen(const RaceSetup& raceSetup)
{
    // the constructor assigns this object to the global
    // variable world. Admittedly a bit ugly, but simplifies
    // handling of objects which get created in the constructor
    // and need world to be defined.
    new World(raceSetup);

    m_current_ = this;

    for(int i = 0; i < raceSetup.getNumPlayers(); ++i)
        m_cameras.push_back(new Camera(raceSetup.getNumPlayers(), i));
    m_fclock.reset();
    m_fclock.setMaxDelta(1.0);
    m_frame_clock.reset();
    m_frame_clock.setMaxDelta(100000.0);
    m_frame_count = 0;
}

//-----------------------------------------------------------------------------
WorldScreen::~WorldScreen()
{
    for (Cameras::iterator i = m_cameras.begin(); i != m_cameras.end(); ++i)
        delete *i;

    if(current() == this)
    {
        delete world;
        world = 0;
    }
}

//-----------------------------------------------------------------------------
void WorldScreen::update()
{
    m_fclock.update();

    if ( ! widgetSet -> get_paused () )
    {
        world->update(m_fclock.getDeltaTime());
    }

    for (Cameras::iterator i = m_cameras.begin(); i != m_cameras.end(); ++i)
        (*i)->update();

    draw();

    menu_manager->update();
    sound_manager->update() ;
    if(config->m_profile)
    {
        m_frame_count++;
        if (world->m_clock>config->m_profile)
        {
            // The actual timing for FPS has to be done with an external clock,
            // since world->m_clock might be modified by replaying a history file.
            m_frame_clock.update();
            printf("Number of frames: %d time %f, Average FPS: %f\n",
                   m_frame_count, m_frame_clock.getAbsTime(),
                   (float)m_frame_count/m_frame_clock.getAbsTime());
            if(!config->m_replay_history) history->Save();
            exit(-2);
        }
    }   // if m_profile

    SDL_GL_SwapBuffers() ;
}

//-----------------------------------------------------------------------------
void
WorldScreen::draw()
{
    const Track* TRACK = world->m_track;

    glEnable ( GL_DEPTH_TEST ) ;

    if (TRACK->useFog())
    {
        glEnable ( GL_FOG ) ;

        glFogf ( GL_FOG_DENSITY, TRACK->getFogDensity() ) ;
        glFogfv( GL_FOG_COLOR  , TRACK->getFogColor() ) ;
        glFogf ( GL_FOG_START  , TRACK->getFogStart() ) ;
        glFogf ( GL_FOG_END    , TRACK->getFogEnd() ) ;
        glFogi ( GL_FOG_MODE   , GL_EXP2   ) ;
        glHint ( GL_FOG_HINT   , GL_NICEST ) ;

        /* Clear the screen */
        glClearColor (TRACK->getFogColor()[0],
                      TRACK->getFogColor()[1],
                      TRACK->getFogColor()[2],
                      TRACK->getFogColor()[3]);
    }
    else
    {
        /* Clear the screen */
        glClearColor (TRACK->getSkyColor()[0],
                      TRACK->getSkyColor()[1],
                      TRACK->getSkyColor()[2],
                      TRACK->getSkyColor()[3]);
    }

    glClear      ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;

    for ( Cameras::iterator i = m_cameras.begin(); i != m_cameras.end(); ++i)
    {
        (*i) -> apply () ;
        world->draw() ;
    }

    if (TRACK->useFog())
    {
        glDisable ( GL_FOG ) ;
    }

    glViewport ( 0, 0, config->m_width, config->m_height ) ;
}

//-----------------------------------------------------------------------------
Camera*
WorldScreen::getCamera(int i) const
{
    if (i >= 0 && i < int(m_cameras.size()))
        return m_cameras[i];
    else
        return 0;
}

/* EOF */
