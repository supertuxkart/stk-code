//  $Id: ScreenManager.cxx,v 1.3 2005/08/19 20:49:27 joh Exp $
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
#include "screen_manager.hpp"
#include "screen.hpp"

ScreenManager* screen_manager = 0;

ScreenManager::ScreenManager() : do_abort(false) {
  current_screen = 0;
  next_screen    = 0;
}  // ScreenManager

// -----------------------------------------------------------------------------
ScreenManager::~ScreenManager() {
  delete next_screen;
  delete current_screen;
}   // ~ScreenManager

// -----------------------------------------------------------------------------
void ScreenManager::setScreen(Screen* screen) {
  assert(next_screen == 0);
  next_screen = screen;
}   // setScreen

// -----------------------------------------------------------------------------
void ScreenManager::run() {
  while(!do_abort) {
    if (current_screen) current_screen->update();

    if (next_screen) {
      delete current_screen;
      current_screen = next_screen;
      next_screen    = 0;
    }   // if next_screen
  }  // while !do_abort
}   // run

// -----------------------------------------------------------------------------
void ScreenManager::abort() {
  do_abort = true;
}

/* EOF */
