//  $Id$
//
//  TuxKart - a fun racing game with go-kart
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


/*Some edits:
 *Commented out the func PlayerDriver::update.. seems unneeded and has 'issues' with
 *AutoDriver::update.
 */

#ifndef HEADER_PLAYERDRIVER_H
#define HEADER_PLAYERDRIVER_H

#include "joystick.h"
#include "Controller.h"
#include "sdldrv.h"

   class KartDriver;

/** PlayerDriver manages controll events from the player and moves
    them to the Kart */
   class PlayerDriver : public Controller
   {
   private:
      float    tscale ;
      float    rscale ;
   
   // physics debugging
      float *selected_property;
   
   public:
      PlayerDriver();
   
      void update(float);
      void incomingJoystick  (JoyInfo& ji);
      void incomingKeystroke ( const SDL_keysym& key );
   };

#endif

/* EOF */
