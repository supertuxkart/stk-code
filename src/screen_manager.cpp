//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmx.de>
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

#include <assert.h>
#include "sdldrv.hpp"
#include "screen_manager.hpp"
#include "screen.hpp"

ScreenManager* screen_manager = 0;

ScreenManager::ScreenManager() : m_do_abort(false)
{
    m_current_screen = 0;
    m_next_screen    = 0;
}  // ScreenManager

//-----------------------------------------------------------------------------
ScreenManager::~ScreenManager()
{
    delete m_next_screen;
    delete m_current_screen;
}   // ~ScreenManager

//-----------------------------------------------------------------------------
void ScreenManager::setScreen(Screen* screen)
{
    assert(m_next_screen == 0);
    m_next_screen = screen;
}   // setScreen

//-----------------------------------------------------------------------------
void ScreenManager::run()
{
    while(!m_do_abort)
    {
        // Run input processing and all that.
        drv_loop();

        // Now the screen may have changed and
        // needs to be updated.
        if (m_next_screen)
        {
            delete m_current_screen;
            m_current_screen = m_next_screen;
            m_next_screen    = 0;
        }   // if next_screen

        if (m_current_screen) m_current_screen->update();
    }  // while !m_do_abort
}   // run

//-----------------------------------------------------------------------------
void ScreenManager::abort()
{
    m_do_abort = true;
}

/* EOF */
