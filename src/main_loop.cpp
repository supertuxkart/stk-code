//  $Id: main_loop.cpp 855 2006-11-17 01:50:37Z coz $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmx.de>
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

#include "main_loop.hpp"

#include <assert.h>
#include "history.hpp"
#include "input/input_manager.hpp"
#include "material_manager.hpp"
#include "race_manager.hpp"
#include "audio/sound_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/scene.hpp"
#include "gui/engine.hpp"
#include "modes/world.hpp"
#include "user_config.hpp"
#include "network/network_manager.hpp"
#include "gui/state_manager.hpp"

MainLoop* main_loop = 0;

// FIXME hacky hacky FPS info
int minFPS  = 999;
int lastFPS =  -1;
int maxFPS  =   0;

MainLoop::MainLoop() :
m_abort(false),
m_frame_count(0)
{
    m_curr_time = 0;
    m_prev_time = 0;
}  // MainLoop

//-----------------------------------------------------------------------------
MainLoop::~MainLoop()
{
}   // ~MainLoop

//-----------------------------------------------------------------------------
/** Run the actual main loop.
 */
void MainLoop::run()
{
    IrrlichtDevice* device = irr_driver->getDevice();
    
    bool music_on = false;
    m_curr_time = device->getTimer()->getRealTime();
    float dt;
    while(!m_abort)
    {
        // input_manager->input();

        m_prev_time = m_curr_time;

        while( 1 )
        {
            m_curr_time = device->getTimer()->getRealTime();
            dt = (float)(m_curr_time - m_prev_time);
            
            // don't allow the game to run slower than a certain amount.
            // when the computer can't keep it up, slow down the shown time instead
            static const float max_elapsed_time = 3.0f*1.0f/60.0f*1000.0f; /* time 3 internal substeps take */
            if(dt > max_elapsed_time) dt=max_elapsed_time;
                                               
            // Throttle fps if more than maximum, which can reduce 
            // the noise the fan on a graphics card makes
            if( dt*user_config->m_max_fps < 1000.0f)
            {
                //SDL_Delay has a granularity of 10ms on most platforms, so
                //most likely when frames go faster than 125 frames, at times
                //it might limit the frames to even 55 frames. On some cases,
                //SDL_Delay(1) will just cause the program to give up the
                //rest of it's timeslice.
                // FIXME - implement with non-SDL code
                // SDL_Delay(1);
            }
            else break;
        }
        dt *= 0.001f;

        if (!music_on && !race_manager->raceIsActive())
        {
            sound_manager->stopMusic();   // stop potential 'left over' music from race
            sound_manager->startMusic(stk_config->m_title_music);
            music_on = true;
        }

        network_manager->update(dt);

        if (race_manager->raceIsActive())
        {
            // Busy wait if race_manager is active (i.e. creating of world is done)
            // till all clients have reached this state.
            if(network_manager->getState()==NetworkManager::NS_READY_SET_GO_BARRIER) continue;

            // Server: Send the current position and previous controls to all clients
            // Client: send current controls to server
            // But don't do this if the race is in finish phase (otherwise 
            // messages can be mixed up in the race manager)
            if(!race_manager->getWorld()->isFinishPhase())
                network_manager->sendUpdates();
            music_on = false; 
            if(user_config->m_profile) dt=1.0f/60.0f;
            // In the first call dt might be large (includes loading time),
            // which can cause the camera to significantly tilt
            stk_scene->draw(RaceManager::getWorld()->getPhase()==SETUP_PHASE ? 0.0f : dt);

            // Again, only receive updates if the race isn't over - once the
            // race results are displayed (i.e. game is in finish phase) 
            // messages must be handled by the normal update of the network 
            // manager
            if(!race_manager->getWorld()->isFinishPhase())
                network_manager->receiveUpdates();

            if ( RaceManager::getWorld()->getPhase() != LIMBO_PHASE)
            {
                history->update(dt);
                RaceManager::getWorld()->update(dt);

                if(user_config->m_profile>0)
                {
                    m_frame_count++;
                    if (RaceManager::getWorld()->getTime()>user_config->m_profile)
                    {
                        //FIXME: SDL_GetTicks() includes the loading time,
                        //so the FPS will be skewed for now.
                        printf("Number of frames: %d time %f, Average FPS: %f\n",
                               m_frame_count, device->getTimer()->getRealTime()*0.001,
                               (float)m_frame_count/(device->getTimer()->getRealTime()));
                        if(!history->replayHistory()) history->Save();
                        std::exit(-2);
                    }   // if profile finished
                }   // if m_profile
            }   // phase != limbo phase
        }   // if race is active
        else
        {
            glMatrixMode   ( GL_PROJECTION ) ;
            glLoadIdentity () ;
            glMatrixMode   ( GL_MODELVIEW ) ;
            glLoadIdentity () ;
            glDisable      ( GL_DEPTH_TEST ) ;
            glDisable      ( GL_LIGHTING   ) ;
            glDisable      ( GL_FOG        ) ;
            glDisable      ( GL_CULL_FACE  ) ;
            glDisable      ( GL_ALPHA_TEST ) ;
            glEnable       ( GL_TEXTURE_2D ) ;

            // On at least one platform the X server apparently gets overloaded
            // by the large texture, resulting in buffering of key events. This
            // results in the menu being very unresponsive/slow - it can sometimes
            // take (say) half a second before the menu reacts to a pressed key.
            // This is caused by X buffering the key events, delivering them
            // later (and sometimes even several at the same frame). This issue
            // could either be solved by a lazy drawing of the background picture
            // (i.e. draw the background only if something has changed) - which is
            // a lot of implementation work ... or by sleeping for a little while,
            // which apparently reduces the load for the X server, so that no
            // buffering is done --> all key events are handled in time.
        #if !defined(WIN32) && !defined(__CYGWIN__)
//                usleep(2000);
        #endif

            
            
            
            //Draw the splash screen
            /*
            if(menu_manager->isMainMenuActive())
                glBindTexture(GL_TEXTURE_2D, m_title_screen_texture);
            else 
                glBindTexture(GL_TEXTURE_2D, m_bg_texture);
            */
            glBegin ( GL_QUADS ) ;
            glColor3f   (1, 1, 1 ) ;
            glTexCoord2f(0, 0); glVertex2i(-1, -1);
            glTexCoord2f(1, 0); glVertex2i( 1, -1);
            glTexCoord2f(1, 1); glVertex2i( 1,  1);
            glTexCoord2f(0, 1); glVertex2i(-1,  1);
            glEnd () ;
        }

        sound_manager->update(dt);

        input_manager->update(dt);
        
        irr_driver->update(dt);

// FIXME hacky hacky FPS reporting
// it should be moved to the right place when on screen display is done
#if 0
        int fps = irr_driver->getDevice()->getVideoDriver()->getFPS();
        bool printFPS = false;
        // First reports seem to be always 1, so not useful
        if ((race_manager->raceIsActive()) && (fps > 1)) {
            // More than +-5 range is interesting to report (otherwise noise)
            if ((lastFPS+5 <= fps) || (lastFPS-5 >= fps)) {
                lastFPS = fps;
                printFPS = true;
            }
            // Min and max are worth updating any time they happen
            if (fps < minFPS) {
                minFPS = fps;
                printFPS = true;
            }
            if (fps > maxFPS) {
                maxFPS = fps;
                printFPS = true;
            }
        } // no else, or you get over 50 'printf ("FPS below 1!\n")' easily
        if (printFPS) printf("FPS %3d<%3d<%3d\n", minFPS, fps, maxFPS);
#endif
    }  // while !m_exit
}   // run

//-----------------------------------------------------------------------------
/** Set the abort flag, causing the mainloop to be left.
 */
void MainLoop::abort()
{
    m_abort = true;
}   // abort

/* EOF */
