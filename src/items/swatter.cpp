//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011 Joerg Henrichs
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

#include "items/swatter.hpp"

#include "items/attachment.hpp"
#include "modes/world.hpp"
#include "karts/kart.hpp"

#define SWAT_ANGLE 22.0f

Swatter::Swatter(Attachment *attachment, Kart *kart) 
       : AttachmentPlugin(attachment, kart)
{
    m_kart             = kart;
    m_count            = kart->getKartProperties()->getSwatterCount();
    m_animation_timer  = 0.0f;
    m_animation_phase  = SWATTER_AIMING;
    m_rot_per_sec      = core::vector3df(0,0,0);
    m_rotation         = core::vector3df(0,0,0);
    m_animation_target = NULL;
}   // Swatter

//-----------------------------------------------------------------------------
Swatter::~Swatter()
{
}   // ~Swatter

//-----------------------------------------------------------------------------
/** Updates an armed swatter: it checks for any karts that are close enough
 *  and not invulnerable, it swats the kart. 
 *  \param dt Time step size.
 *  \return True if the attachment should be discarded.
 */
bool Swatter::updateAndTestFinished(float dt)
{
    m_rotation += m_rot_per_sec * dt;
    switch(m_animation_phase)
    {
    case SWATTER_AIMING:    
        aimSwatter(); 
        break;
    case SWATTER_TO_KART:
        if(fabsf(m_rotation.Z)>=90)
        {
            checkForHitKart(m_rotation.Z>0);
            m_rot_per_sec *= -1.0f;
            m_animation_phase = SWATTER_BACK_FROM_KART;
        }
        break;
    case SWATTER_BACK_FROM_KART:
        if (m_rotation.Z>0)
        {
            m_rotation         = core::vector3df(0,0,0);
            m_rot_per_sec      = core::vector3df(0,0,0);
            m_animation_phase  = SWATTER_AIMING;
            m_animation_target = NULL;
        }
        break;
    case SWATTER_ITEM_1:  // swatter going to the left
        if(m_rotation.Z>SWAT_ANGLE)
        {
            m_animation_phase = SWATTER_ITEM_2;
            m_rot_per_sec    *= -1.0f;
        }
        break;
    case SWATTER_ITEM_2:  // swatter going all the way to the right
        if(m_rotation.Z<-SWAT_ANGLE)
        {
            m_animation_phase = SWATTER_ITEM_3;
            m_rot_per_sec *= -1.0f;
        }
        break;
    case SWATTER_ITEM_3:  // swatter going back to rest position.
        if(m_rotation.Z>0)
        {
            m_rotation         = core::vector3df(0,0,0);
            m_rot_per_sec      = core::vector3df(0,0,0);
            m_animation_phase  = SWATTER_AIMING;
        }
        break;
    }   // switch m_animation_phase


    // If the swatter is used up, trigger cleaning up
    return (m_count==0);
}   // updateAndTestFinished

//-----------------------------------------------------------------------------
/** Returns true if the point xyz is to the left of the kart. 
 *  \param xyz Point to determine the direction 
 */
bool Swatter::isLeftSideOfKart(const Vec3 &xyz)
{
    Vec3 forw_vec = m_kart->getTrans().getBasis().getColumn(2);
    const Vec3& k1 = m_kart->getXYZ();
    const Vec3  k2 = k1+forw_vec;
    return xyz.sideOfLine2D(k1, k2)>0;
}   // isLeftSideOfKart

//-----------------------------------------------------------------------------
/** This function is called when the swatter reaches the hit angle (i.e. it
 *  is furthest down). Check all karts if any one is hit, i.e. is at the right
 *  side and at the right angle and distance.
 *  \param isWattingLeft True if the swatter is aiming to the left side
 *         of the kart.
 */
void Swatter::checkForHitKart(bool isSwattingLeft)
{
    // Square of the minimum distance
    const KartProperties *kp = m_kart->getKartProperties();
    float min_dist2          = kp->getSwatterDistance2();
    Kart *hit_kart           = NULL;
    Vec3 forw_vec            = m_kart->getTrans().getBasis().getColumn(2);
    const World *world       = World::getWorld();

    for(unsigned int i=0; i<world->getNumKarts(); i++)
    {
        Kart *kart = world->getKart(i);
        if(kart->isEliminated() || kart==m_kart || kart->isSquashed())
            continue;
        float f = (kart->getXYZ()-m_kart->getXYZ()).length2();

        // Distance is too great, ignore this kart.
        if(f>min_dist2) continue;

        // Check if the kart is at the right side.
        const bool left = isLeftSideOfKart(kart->getXYZ());
        if(left!=isSwattingLeft)
        {
            //printf("%s wrong side: %d %d\n", 
            //    kart->getIdent().c_str(), left, isSwattingLeft);
            continue;
        }

        Vec3 kart_vec    = m_kart->getXYZ()-kart->getXYZ();
        // cos alpha = a*b/||a||/||b||
        // Since forw_vec is a unit vector, we only have to divide by
        // the length of the vector to the kart.
        float cos_angle = kart_vec.dot(forw_vec)/kart_vec.length();
        float angle     = acosf(cos_angle)*180/M_PI;
        if(angle<45 || angle>135)
        {
            //printf("%s angle %f\n", kart->getIdent().c_str(), angle);
            continue;
        }
        
        kart->setSquash(kp->getSquashDuration(), 
                        kp->getSquashSlowdown());
        // It is assumed that only one kart is within reach of the swatter,
        // so we can stop testing karts here.
        return;
    }   // for i < num_karts

}   // angleToKart

//-----------------------------------------------------------------------------
/** Checks for any kart that is not already squashed that is close enough.
 *  If a kart is found, it changes the state of the swatter to be 
 *  SWATTER_TARGET and starts the animation.
 */
void Swatter::aimSwatter()
{
    const World *world = World::getWorld();
    Kart *min_kart     = NULL;
    // Square of the minimum distance
    float min_dist2    = m_kart->getKartProperties()->getSwatterDistance2();

    for(unsigned int i=0; i<world->getNumKarts(); i++)
    {
        Kart *kart = world->getKart(i);
        if(kart->isEliminated() || kart==m_kart || kart->isSquashed())
            continue;
        float f = (kart->getXYZ()-m_kart->getXYZ()).length2();
        if(f<min_dist2)
        {
            min_dist2 = f;
            min_kart = kart;
        }
    }
    // No kart close enough, nothing to do.
    if(!min_kart) return;

    m_count --;
    m_animation_phase        = SWATTER_TO_KART;
    m_animation_target       = min_kart;
    if(UserConfigParams::logMisc())
        printf("[swatter] %s aiming at %s.\n", 
                m_kart->getIdent().c_str(), min_kart->getIdent().c_str());
    const KartProperties *kp = m_kart->getKartProperties();
    m_animation_timer        = kp->getSwatterAnimationTime();
    const bool left          = isLeftSideOfKart(min_kart->getXYZ());
    m_rot_per_sec = core::vector3df(0, 0, 
                                    left ?90.0f:-90.0f) / m_animation_timer;

}   // aimSwatter

//-----------------------------------------------------------------------------
/** Starts a (smaller) and faster swatting movement to be played
 *  when the kart is hit by an item.
 */
void Swatter::swatItem()
{
    if(UserConfigParams::logMisc())
        printf("[swatter] %s swatting item.\n",
               m_kart->getIdent().c_str());
    assert(m_animation_target==NULL);

    m_animation_phase  = SWATTER_ITEM_1;
    m_animation_timer  = 
        m_kart->getKartProperties()->getSwatterItemAnimationTime();
    m_count--;
    m_rot_per_sec = core::vector3df(0, 0, SWAT_ANGLE) / m_animation_timer;
}   // swatItem
