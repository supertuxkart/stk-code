//  $Id: GrandPrixSetup.h,v 1.4 2004/08/25 00:21:23 grumbel Exp $
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

#ifndef HEADER_GRANDPRIXSETUP_H
#define HEADER_GRANDPRIXSETUP_H

#include <vector>
#include <string>
#include "CupData.h"

class GrandPrixSetup
{
public:
  struct Stat {
    Stat(const std::string& ident, int points_, int position_);

    /** ident of the kart (aka. .tkkf filename without .tkkf) */
    std::string ident;

    /** Number of Points optained in the GrandPrix so far */
    int points;
    
    /** Position of the kart in the next race (equal to the finishing
        position in the last race) */
    int position;
  };

  /** The number of races which alread have been completed */
  int race;

  /** The karts that participate in the GrandPrix and there point,
      position, etc. */
  std::vector<Stat> karts;

  GrandPrixSetup();
  ~GrandPrixSetup();

  void clear();
};

extern GrandPrixSetup grand_prix_setup;

#endif

/* EOF */
