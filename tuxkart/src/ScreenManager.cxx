//  $Id: ScreenManager.cxx,v 1.2 2004/08/11 11:36:40 grumbel Exp $
//
//  TuxKart - a fun racing game with go-kart
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

#include "ScreenManager.h"

ScreenManager* ScreenManager::current_ = 0;

ScreenManager::ScreenManager()
{
  current_ = this;
  current_screen = 0;
}

void
ScreenManager::set_screen(Screen* screen)
{
  delete current_screen;
  current_screen = screen;
}

void
ScreenManager::run()
{
  while(current_screen)
    {
      current_screen->update();
    }
}

void
ScreenManager::shutdown()
{
  delete current_screen;
  current_screen = 0;
}

/* EOF */
