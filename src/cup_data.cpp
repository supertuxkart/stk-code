//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2006 Ingo Ruhnke <grumbel@gmx.de>
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

#include <iostream>
#include <stdexcept>
#include "loader.hpp"
#include "lisp/parser.hpp"
#include "lisp/lisp.hpp"
#include "cup_data.hpp"

CupData::CupData(const char* filename)
{
  const lisp::Lisp* lisp = 0;
  try {
    lisp::Parser parser;
    lisp = parser.parse(loader->getPath(filename));
    
    lisp = lisp->getLisp("tuxkart-cup");
    if(!lisp)
        throw std::runtime_error("No tuxkart-cup node");
    
    lisp->get("name", name);
    lisp->getVector("tracks", tracks);
  } catch(std::exception& err) {
    std::cout << "Error while reading cup: " << err.what() << "\n";
  }

  delete lisp;
}

/* EOF */
