//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
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

#ifndef HEADER_HERRING_H
#define HEADER_HERRING_H

#include <plib/sg.h>

class Kart;
class ssgTransform;
class ssgEntity;

// HE_RED ust be the first, HE_SILVER the last entry. See HerringManager
enum herringType { HE_RED, HE_GREEN, HE_GOLD, HE_SILVER };
 
// ----------------------------------------------------------------------------- 
class Herring {
private:
  herringType   type;         // Herring type
  bool          bEaten;       // true if herring  was eaten & is not displayed
  float         time_to_return;  // world->clock when an eaten herring reappears
  sgCoord       coord;        // Original coordinates, used mainly when 
                              // eaten herrings reappear.
  ssgTransform* root;         // The actual root of the herring
  ssgTransform* rotate;       // Just below root is a node only rotating
  float         rotation;     // Amount of rotation
 
 public:
              Herring   (herringType type, sgVec3* xyz, ssgEntity* model);
             ~Herring   ();
  void        update    (float delta);
  bool        wasEaten  ()            {return bEaten;}
  void        isEaten   ();
  herringType getType   ()            {return type;}
  int         hitKart   (Kart* kart );
  void        reset     ();
  ssgTransform* getRoot () const {return root;}
};   // class Herring

#endif
