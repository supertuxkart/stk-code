//  $Id: Projectile.h,v 1.4 2004/09/24 15:45:02 matzebraun Exp $
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

#ifndef HEADER_PROJECTILE_H
#define HEADER_PROJECTILE_H

class World;
class KartDriver;

class Projectile
{
private:
  World* world;
  KartDriver *owner ;
  int type;

  sgCoord velocity;

public:
  Projectile(World* world, KartDriver* _owner, int type);
  virtual ~Projectile();

  void fire ( KartDriver *who, int _type );

  virtual void update ( float delta_t ) ;
};

#endif
