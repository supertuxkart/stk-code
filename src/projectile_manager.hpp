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

#ifndef HEADER_PROJECTILEMANAGER_H
#define HEADER_PROJECTILEMANAGER_H

#include <vector>
#include <plib/ssg.h>

class Kart;
class Projectile;
class Explosion;

class ProjectileManager
{
private:
    typedef std::vector<Projectile*> Projectiles;
    typedef std::vector<Explosion* > Explosions;

    // The list of all active projectiles, i.e. projectiles
    // which are currently moving on the track
    Projectiles      m_active_projectiles;

    // The list of all deleted projectiles, i.e. projectils which
    // hit something and have therefore been deleted. The objects
    // in this list can be reused later, this removes the overhead
    // of object creation
    Projectiles      m_deleted_projectiles;

    // All active explosions, i.e. explosions which are currently
    // being shown
    Explosions       m_active_explosions;

    // The list of deleted explosion, which will be reused.
    Explosions       m_deleted_explosions;

    ssgSelector*     m_explosion_model;
    bool             m_something_was_hit;
    bool             m_explosion_ended;

public:
    ProjectileManager() {m_something_was_hit=false;};
    ~ProjectileManager() {};
    void             explode          () {m_something_was_hit=true;}
    void             FinishedExplosion() {m_explosion_ended =true;}
    ssgSelector*     getExplosionModel()
    {
        return (ssgSelector*)m_explosion_model->clone();
    }
    int              getProjectileId  (const std::string ident);
    void             loadData         ();
    void             cleanup          ();
    void             update           (float dt);
    Projectile*      newProjectile    (Kart *kart, int type);
    Explosion*       newExplosion     (sgCoord *coord);
    void             Deactivate       (Projectile *p) {}
    void             removeTextures   ();
};

extern ProjectileManager *projectile_manager;

#endif

/* EOF */
