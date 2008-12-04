//  $Id: missile.cpp 1284 2007-11-08 12:31:54Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2007 Joerg Henrichs
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
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

#include "items/plunger.hpp"

#include "constants.hpp"
#include "camera.hpp"
#include "race_manager.hpp"
#include "scene.hpp"
#include "items/rubber_band.hpp"
#include "items/projectile_manager.hpp"
#include "karts/player_kart.hpp"
#include "modes/world.hpp"

// -----------------------------------------------------------------------------
Plunger::Plunger(Kart *kart) : Flyable(kart, POWERUP_PLUNGER)
{
    float y_offset = 0.5f*kart->getKartLength()+0.5f*m_extend.getY();
    
    // if the kart is looking backwards, release from the back
    PlayerKart* pk = dynamic_cast<PlayerKart*>(kart);
    const bool reverse_mode = (pk != NULL && pk->getCamera()->getMode() == Camera::CM_REVERSE);
    
    createPhysics(y_offset, btVector3(0.0f, m_speed*2, 0.0f),
                  new btCylinderShape(0.5f*m_extend), 0.0f /* gravity */, false /* rotates */, reverse_mode );
    m_rubber_band = new RubberBand(this, *kart);
    m_rubber_band->ref();
    m_keep_alive = -1;
}   // Plunger

// -----------------------------------------------------------------------------
Plunger::~Plunger()
{
    m_rubber_band->removeFromScene();
    ssgDeRefDelete(m_rubber_band);
}   // ~Plunger

// -----------------------------------------------------------------------------
void Plunger::init(const lisp::Lisp* lisp, ssgEntity *plunger_model)
{
    Flyable::init(lisp, plunger_model, POWERUP_PLUNGER);
}   // init

// -----------------------------------------------------------------------------
void Plunger::update(float dt)
{
    // In keep-alive mode, just update the rubber band
    if(m_keep_alive >= 0)
    {
        m_keep_alive -= dt;
        if(m_keep_alive<=0)
        {
            setHasHit();
            projectile_manager->notifyRemove();
            ssgTransform *m = getModelTransform();
            m->removeAllKids();
            scene->remove(m);
        }
        m_rubber_band->update(dt);
        return;
    }

    // Else: update the flyable and rubber band
    Flyable::update(dt);
    m_rubber_band->update(dt);
}   // update

// -----------------------------------------------------------------------------
/** Virtual function called when the plunger hits something.
 *  The plunger is special in that it is not deleted when hitting an object.
 *  Instead it stays around (though not as a graphical or physical object)
 *  till the rubber band expires.
 *  \param kart Pointer to the kart hit (NULL if not a kart).
 *  \param mp  Pointer to MovingPhysics object if hit (NULL otherwise).
 */
void Plunger::hit(Kart *kart, MovingPhysics *mp)
{
    if(isOwnerImmunity(kart)) return;

    m_keep_alive = m_owner->getKartProperties()->getRubberBandDuration();

    // Make this object invisible by placing it faaar down. Not that if this
    // objects is simply removed from the scene graph, it might be auto-deleted
    // because the ref count reaches zero.
    Vec3 hell(0, 0, -10000);
    getModelTransform()->setTransform(hell.toFloat());
    RaceManager::getWorld()->getPhysics()->removeBody(getBody());
    
    if(kart)
    {
        m_rubber_band->hit(kart);
        return;
    }
    else if(mp)
    {
        Vec3 pos(mp->getBody()->getWorldTransform().getOrigin());
        m_rubber_band->hit(NULL, &pos);
    }
    else
    {
        m_rubber_band->hit(NULL, &(getXYZ()));
    }

}   // hit

// -----------------------------------------------------------------------------
/** Called when the plunger hits the track. In this case, notify the rubber
 *  band, and remove the plunger (but keep it alive). 
 */
void Plunger::hitTrack()
{
    hit(NULL, NULL);
}   // hitTrack

// -----------------------------------------------------------------------------
