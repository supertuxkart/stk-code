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
#include <stdexcept>
CreditsMenu::CreditsMenu(){

  std::string filename;
  try {
    filename = loader->getPath("data/CREDITS");
    FILE *fd = fopen(filename.c_str(), "ra");
    char s[1024];
    while(fgets(s, 1023, fd)) {
      char *p = strdup(s);
      sl.push_back(p);
    }   // while
  } catch(std::runtime_error& e) {
    printf("Couldn't load '%s'\n",filename.c_str());
    sl.push_back("CREDIT file was not installed properly!!");
    sl.push_back("Please check 'data/CREDIT'!!");
  }
  
  setText(sl);
}   // CreditsMenu

// -----------------------------------------------------------------------------
CreditsMenu::~CreditsMenu() {
  while(sl.size()>0) {
    sl.pop_back();
  }
}   // ~CreditsMenu
	
/* EOF */
