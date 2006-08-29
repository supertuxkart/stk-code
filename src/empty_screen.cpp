//  $Id: empty_screen.cpp,v 1.1 2005/05/25 21:53:43 joh Exp $
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

#include <plib/pw.h>

#include "empty_screen.hpp"
#include "start_screen.hpp"
#include "screen_manager.hpp"
#include "gui/menu_manager.hpp"
#include "plibdrv.hpp"

EmptyScreen::EmptyScreen()
{
  pwSetCallbacks(keystroke, gui_mousefn, gui_motionfn, NULL, NULL);
}

EmptyScreen::~EmptyScreen()
{
}

void
EmptyScreen::update()
{
    pollEvents() ;
    menu_manager->update();

    pwSwapBuffers();
}

/* EOF */
