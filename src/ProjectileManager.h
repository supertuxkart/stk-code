//  $Id: ProjectileManager.h,v 1.5 2005/08/17 22:36:30 joh Exp $
//
//  SuperTuxKart - a fun racing game with go-kart
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

#ifndef HEADER_PROJECTILEMANAGER_H
#define HEADER_PROJECTILEMANAGER_H

#include <vector>
#include <plib/ssg.h>

class Kart;
class Projectile;
class Explosion;

class ProjectileManager {
 private:
  typedef std::vector<Projectile*> Projectiles;
  typedef std::vector<Explosion* > Explosions;

  // The list of all active projectiles, i.e. projectiles 
  // which are currently moving on the track
  Projectiles      activeProjectiles;
  
  // The list of all deleted projectiles, i.e. projectils which
  // hit something and have therefore been deleted. The objects
  // in this list can be reused later, this removes the overhead
  // of object creation
  Projectiles      deletedProjectiles;

  // All active explosions, i.e. explosions which are currently
  // being shown
  Explosions       activeExplosions;

  // The list of deleted explosion, which will be reused.
  Explosions       deletedExplosions;

  ssgSelector*     explosionModel;
  bool             somethingWasHit;
  bool             explosionEnded;

public:
                   ProjectileManager() {};
                  ~ProjectileManager(){};
  void             explode          (Projectile *p){somethingWasHit=true;}
  void             FinishedExplosion(Explosion *p) {explosionEnded =true;}
  ssgSelector*     getExplosionModel()             {
                             return (ssgSelector*)explosionModel->clone();}
  int              getProjectileId  (const std::string ident);
  void             loadData         ();
  void             cleanup          ();
  void             update           (float dt);
  Projectile*      newProjectile    (Kart *kart, int type);
  Explosion*       newExplosion     (Projectile *p);
  void             Deactivate       (Projectile *p) {}
};

extern ProjectileManager *projectile_manager;

#endif

/* EOF */
