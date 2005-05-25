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

#ifndef HEADER_PROJECTILE_H
#define HEADER_PROJECTILE_H

#define PROJECTILE_ISECT_STEP_SIZE 0.4f

#define MISSILE_COLLISION_SPHERE_RADIUS 0.7f
#define HOMING_MISSILE_COLLISION_SPHERE_RADIUS 0.7f
#define SPARK_COLLISION_SPHERE_RADIUS 0.6f

//After a certain amount of seconds, projectiles should explode
#define MISSILE_LIFETIME 3.0f
#define HOMING_MISSILE_LIFETIME 6.5f
#define SPARK_LIFETIME 45.0f

class World;
class KartDriver;

class Projectile
{
private:
  World* world;
  KartDriver *owner ;
  int type;

  sgCoord velocity;
  sgCoord position;
  sgCoord last_pos;
  
  ssgTransform* model;

  float collision_sphere_radius;
  int collided;
  int exploded;
  
  sgVec3 surface_avoidance_vector ;
  
  float lifetime_limit_secs;
  float current_lifetime;//How much time has the projectile lived
  
  
  float collectIsectData ( sgVec3 start, sgVec3 end);
  float getIsectData ( sgVec3 start, sgVec3 end );
  void doCollisionAnalysis  ( float  delta,  float  hot  );
  
  void updateLifetime(float delta);
  
public:
  Projectile(World* world, KartDriver* _owner, int type);
  virtual ~Projectile();

  sgCoord *getCoord ()
  {
     return & position ;
  }

  void setCoord ( sgCoord *pos )
  {
     sgCopyCoord ( & position, pos ) ;
  }
  
  int hasExploded ()
  {
     return exploded;
  }
  
  //void fire ( KartDriver *who, int _type );

  virtual void update ( float delta_t ) ;
};

#endif
