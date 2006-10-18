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

#include "world.hpp"
#include "herring.hpp"
#include "kart.hpp"

// =============================================================================
Herring::Herring(herringType _type, sgVec3* xyz, ssgEntity* model) 
{
  sgSetVec3(coord.hpr, 0.0f, 0.0f, 0.0f);

  sgCopyVec3(coord.xyz, *xyz);
  root   = new ssgTransform();
  root->ref();
  root->setTransform(&coord);

  rotate = new ssgTransform();
  rotate->ref();
  rotate->addKid(model);
  root->addKid(rotate);
  world->addToScene(root);

  type           = _type;
  bEaten         = FALSE;
  rotation       = 0.0f;
  time_to_return = 0.0f;  // not strictly necessary, see isEaten()
}   // Herring

// -----------------------------------------------------------------------------
Herring::~Herring() 
{
  ssgDeRefDelete(root);
  ssgDeRefDelete(rotate);
}   // ~Herring

// -----------------------------------------------------------------------------
void Herring::reset() 
{
  bEaten         = false;
  time_to_return = 0.0f;
  root->setTransform(&coord);
}   // reset
// -----------------------------------------------------------------------------
int Herring::hitKart(Kart* kart) 
{
  return sgDistanceSquaredVec3 ( kart->getCoord()->xyz, coord.xyz ) < 0.8f;
}   // hitKart

// -----------------------------------------------------------------------------
void Herring::update(float delta) 
{
  if(bEaten) {
    float t = time_to_return - world->clock;
    if ( t > 0 ) 
    {
      sgVec3 hell;
      sgCopyVec3(hell, coord.xyz);

      hell[2] = ( t > 1.0f ) ? -1000000.0f : coord.xyz[2] - t / 2.0f;
      root -> setTransform(hell);
    } else {
      bEaten   = FALSE;
      rotation = 0.0f;
      root -> setTransform(&coord);
    }   // t>0
    
  } else 
  {   // not bEaten
    sgCoord c = { { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } } ;
    c.hpr[0] = rotation;
    rotation += 180.0f*delta;
    rotate -> setTransform ( &c ) ;
  }
}   // update

// -----------------------------------------------------------------------------
void Herring::isEaten()
{
    bEaten=TRUE;
    time_to_return=world->clock+2.0f;
}

