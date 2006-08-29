//  $Id: physics_parameter.hpp,v 1.2 2005/07/14 15:43:44 joh Exp $
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

#ifndef HEADER_PHYSICSPARAMETER_H
#define HEADER_PHYSICSPARAMETER_H

#include "kart_properties.hpp"
#include "lisp/lisp.hpp"

class PhysicsParameters : public KartProperties {
 public:
  float anvilWeight;           // Additional kart weight if anvil is attached
  float anvilSpeedFactor;      // To decrease speed once when attached
  float parachuteFriction;     // Increased air friction when parachute
  float magnetRangeSQ;         // Squared range for magnets
  float magnetMinRangeSQ;      // Squared minimum range for magnets
  float jumpImpulse;           // percentage of gravity when jumping

       PhysicsParameters() : KartProperties() {};
  void init_defaults    ();
  void getAllData       (const lisp::Lisp* lisp);
  void load             (const char* filename, 
			 char *node="physics");

};   // PhysicsParameters

extern PhysicsParameters* physicsParameters;
#endif
