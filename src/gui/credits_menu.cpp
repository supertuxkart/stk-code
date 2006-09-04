//  $Id$
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

#include "credits_menu.hpp"
#include "loader.hpp"

CreditsMenu::CreditsMenu(){

  std::string filename = loader->getPath("data/CREDITS");
  FILE *fd = fopen(filename.c_str(), "r");
  
  char s[1024];
  while(fgets(s, 1023, fd)) {
    char *p = strdup(s);
    sl.push_back(p);
  }
  fclose(fd);
  setText(sl);
}   // CreditsMenu

// -----------------------------------------------------------------------------
CreditsMenu::~CreditsMenu() {
  while(sl.size()>0) {
    char*p=sl.back();
    sl.pop_back();
  }
}   // ~CreditsMenu
	
/* EOF */
