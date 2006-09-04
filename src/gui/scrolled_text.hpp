//  $Id: credits_menu.hpp 694 2006-08-29 07:42:36Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
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

#ifndef HEADER_SCROLL_TEXT_HPP
#define HEADER_SCROLL_TEXT_HPP

#include <string>
#include <plib/fnt.h>
#include <vector>
#include "base_gui.hpp"
#include "player.hpp"

typedef std::vector<char*> StringList;

class ScrolledText: public BaseGUI {
protected:
  int         xLeft, xRight, yBottom, yTop;
  float       yPos, ySpeed;
  int         fontSize;
  StringList  sl;
  int         rect;
public:
                ScrolledText();
               ~ScrolledText();
          void setText      (StringList sl_);
  virtual void select       ();
  virtual void update       (float dt);
  virtual void keybd        (int key);
};

#endif
