//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
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

#include "items/attachment.hpp"

#include <algorithm>
#include "config/stk_config.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "items/attachment_manager.hpp"
#include "items/projectile_manager.hpp"
#include "karts/kart.hpp"
#include "modes/three_strikes_battle.hpp"
#include "network/race_state.hpp"
#include "network/network_manager.hpp"
#include "utils/constants.hpp"

#define SWAT_ANGLE 22.0f

Attachment::Attachment(Kart* kart)
{
    m_type           = ATTACH_NOTHING;
    m_time_left      = 0.0;
    m_kart           = kart;
    m_previous_owner = NULL;

    // If we attach a NULL mesh, we get a NULL scene node back. So we
    // have to attach some kind of mesh, but make it invisible.
    m_node = irr_driver->addAnimatedMesh(
                         attachment_manager->getMesh(Attachment::ATTACH_BOMB));
#ifdef DEBUG
    std::string debug_name = kart->getIdent()+" (attachment)";
    m_node->setName(debug_name.c_str());
#endif
    m_node->setParent(m_kart->getNode());
    m_node->setVisible(false);
}   // Attachment

//-----------------------------------------------------------------------------
Attachment::~Attachment()
{
    if(m_node)
        irr_driver->removeNode(m_node);
}   // ~Attachment

//-----------------------------------------------------------------------------
void Attachment::set(AttachmentType type, float time, Kart *current_kart)
{
    clear();
    
    m_node->setMesh(attachment_manager->getMesh(type));

    if (!UserConfigParams::m_graphical_effects)
    {
        m_node->setAnimationSpeed(0);
        m_node->setCurrentFrame(0);
    }
    
    m_type             = type;
    m_time_left        = time;
    m_previous_owner   = current_kart;
    m_node->setRotation(core::vector3df(0, 0, 0));
    m_count            = m_type==ATTACH_SWATTER
                         ? m_kart->getKartProperties()->getSwatterCount()
                         : 1;
    m_animation_timer  = 0.0f;
    m_animation_phase  = SWATTER_AIMING;
    m_rot_per_sec      = core::vector3df(0,0,0);
    m_animation_target = NULL;

    // A parachute can be attached as result of the usage of an item. In this
    // case we have to save the current kart speed so that it can be detached
    // by slowing down.
    if(m_type==ATTACH_PARACHUTE)
    {
        m_initial_speed = m_kart->getSpeed();
        if(m_initial_speed <= 1.5) m_initial_speed = 1.5; // if going very slowly or backwards, braking won't remove parachute
        
        if (UserConfigParams::m_graphical_effects)
        {
            m_node->setAnimationSpeed(50); // .blend was created @25 (<10 real, slow computer), make it faster
        }
    }
    m_node->setVisible(true);
}   // set

// -----------------------------------------------------------------------------
/** Removes any attachement currently on the kart. As for the anvil attachment,
 *  takes care of resetting the owner kart's physics structures to account for
 *  the updated mass.
 */
void Attachment::clear()
{
    m_type=ATTACH_NOTHING; 
    
    m_time_left=0.0;
    m_node->setVisible(false);

    // Resets the weight of the kart if the previous attachment affected it 
    // (e.g. anvil). This must be done *after* setting m_type to
    // ATTACH_NOTHING in order to reset the physics parameters.
    m_kart->updatedWeight();
}   // clear

// -----------------------------------------------------------------------------
/** Randomly selects the new attachment. For a server process, the
*   attachment can be passed into this function.
*  \param item The item that was collected.
*  \param new_attachment Optional: only used on the clients, it
*                        specifies the new attachment to use
*/
void Attachment::hitBanana(Item *item, int new_attachment)
{
    float leftover_time   = 0.0f;
    
    bool add_a_new_item = true;
    
    if (dynamic_cast<ThreeStrikesBattle*>(World::getWorld()) != NULL)
    {
        World::getWorld()->kartHit(m_kart->getWorldKartId());
        m_kart->handleExplosion(Vec3(0.0f, 1.0f, 0.0f), true);
        return;
    }
    
    switch(getType())   // If there already is an attachment, make it worse :)
    {
    case ATTACH_BOMB:
        {
        add_a_new_item = false;
        projectile_manager->newExplosion(m_kart->getXYZ(), "explosion", m_kart->getController()->isPlayerController());
        m_kart->handleExplosion(m_kart->getXYZ(), /*direct_hit*/ true);
        clear();
        if(new_attachment==-1) 
            new_attachment = m_random.get(3);
        // Disable the banana on which the kart just is for more than the 
        // default time. This is necessary to avoid that a kart lands on the 
        // same banana again once the explosion animation is finished, giving 
        // the kart the same penalty twice.
        float f = std::max(item->getDisableTime(), 
                           m_kart->getKartProperties()->getExplosionTime()+2.0f);
        item->setDisableTime(f);
        break;
        }
    case ATTACH_ANVIL:
        // if the kart already has an anvil, attach a new anvil, 
        // and increase the overall time 
        new_attachment = 2;
        leftover_time  = m_time_left;
        break;
    case ATTACH_PARACHUTE:
        new_attachment = 2;
        leftover_time  = m_time_left;
        break;
    default:
        // There is no attachment currently, but there will be one
        // so play the character sound ("Uh-Oh")
        m_kart->playCustomSFX(SFXManager::CUSTOM_ATTACH);

        if(new_attachment==-1)
            new_attachment = m_random.get(3);
    }   // switch
    
    // Save the information about the attachment in the race state
    // so that the clients can be updated.
    if(network_manager->getMode()==NetworkManager::NW_SERVER)
    {
        race_state->itemCollected(m_kart->getWorldKartId(),
                                  item->getItemId(),
                                  new_attachment);
    }

    if (add_a_new_item)
    {
        switch (new_attachment)
        {
        case 0: 
            set( ATTACH_PARACHUTE,stk_config->m_parachute_time+leftover_time);
            m_initial_speed = m_kart->getSpeed();
            if(m_initial_speed <= 1.5) m_initial_speed = 1.5; // if going very slowly or backwards, braking won't remove parachute
            // if ( m_kart == m_kart[0] )
            //   sound -> playSfx ( SOUND_SHOOMF ) ;
            break ;
        case 1:
            set( ATTACH_BOMB, stk_config->m_bomb_time+leftover_time);
            // if ( m_kart == m_kart[0] )
            //   sound -> playSfx ( SOUND_SHOOMF ) ;
            break ;
        case 2:
            set( ATTACH_ANVIL, stk_config->m_anvil_time+leftover_time);
            // if ( m_kart == m_kart[0] )
            //   sound -> playSfx ( SOUND_SHOOMF ) ;
            // Reduce speed once (see description above), all other changes are
            // handled in Kart::updatePhysics
            m_kart->adjustSpeed(stk_config->m_anvil_speed_factor);
            m_kart->updatedWeight();
            break ;
        }   // switch
    }
}   // hitBanana

//-----------------------------------------------------------------------------
//** Moves a bomb from kart FROM to kart TO.
void Attachment::moveBombFromTo(Kart *from, Kart *to)
{
    to->getAttachment()->set(ATTACH_BOMB,
                             from->getAttachment()->getTimeLeft()+
                             stk_config->m_bomb_time_increase, from);
    from->getAttachment()->clear();
}   // moveBombFromTo

//-----------------------------------------------------------------------------
void Attachment::update(float dt)
{
    if(m_type==ATTACH_NOTHING) return;
    m_time_left -=dt;

    switch (m_type)
    {
    case ATTACH_PARACHUTE:
        // Partly handled in Kart::updatePhysics
        // Otherwise: disable if a certain percantage of
        // initial speed was lost
        if(m_kart->getSpeed() <= m_initial_speed*stk_config->m_parachute_done_fraction)
        {
            m_time_left = -1;
        }
        break;
    case ATTACH_ANVIL:     // handled in Kart::updatePhysics
    case ATTACH_NOTHING:   // Nothing to do, but complete all cases for switch
    case ATTACH_MAX:
        break;
    case ATTACH_SWATTER:
        updateSwatter(dt);
        // If the swatter is used up, trigger cleaning up
        if(m_count==0) m_time_left = -1.0f;
        break;
    case ATTACH_BOMB:
        if(m_time_left <= (m_node->getEndFrame() - m_node->getStartFrame() - 1))
        {
            m_node->setCurrentFrame(m_node->getEndFrame() - m_node->getStartFrame() - 1 -m_time_left);
        }
        if(m_time_left<=0.0)
        {
            projectile_manager->newExplosion(m_kart->getXYZ(), "explosion", m_kart->getController()->isPlayerController());
            m_kart->handleExplosion(m_kart->getXYZ(), 
                                    /*direct_hit*/ true);
        }
        break;
    case ATTACH_TINYTUX:
        // Nothing to do for tiny tux, this is all handled in EmergencyAnimation
        break;
    }   // switch

    // Detach attachment if its time is up.
    if ( m_time_left <= 0.0f)
        clear();
}   // update

//-----------------------------------------------------------------------------
/** Updates an armed swatter: it checks for any karts that are close enough
 *  and not invulnerable, it swats the kart. 
 *  \param dt Time step size.
 */
void Attachment::updateSwatter(float dt)
{
    core::vector3df r = m_node->getRotation();
    r += m_rot_per_sec * dt;
    switch(m_animation_phase)
    {
    case SWATTER_AIMING:    
        aimSwatter(); 
        break;
    case SWATTER_TO_KART:
        if(fabsf(r.Z)>=90)
        {
            checkForHitKart(r.Z>0);
            m_rot_per_sec *= -1.0f;
            m_animation_phase = SWATTER_BACK_FROM_KART;
        }
        break;
    case SWATTER_BACK_FROM_KART:
        if (r.Z>0)
        {
            r                  = core::vector3df(0,0,0);
            m_rot_per_sec      = r;
            m_animation_phase  = SWATTER_AIMING;
            m_animation_target = NULL;
        }
        break;
    case SWATTER_ITEM_1:  // swatter going to the left
        if(r.Z>SWAT_ANGLE)
        {
            m_animation_phase = SWATTER_ITEM_2;
            m_rot_per_sec    *= -1.0f;
        }
        break;
    case SWATTER_ITEM_2:  // swatter going all the way to the right
        if(r.Z<-SWAT_ANGLE)
        {
            m_animation_phase = SWATTER_ITEM_3;
            m_rot_per_sec *= -1.0f;
        }
        break;
    case SWATTER_ITEM_3:  // swatter going back to rest position.
        if(r.Z>0)
        {
            r                 = core::vector3df(0,0,0);
            m_rot_per_sec     = r;
            m_animation_phase = SWATTER_AIMING;
        }
        break;
    }   // switch m_animation_phase

    m_node->setRotation(r);
}   // updateSwatter

//-----------------------------------------------------------------------------
/** Returns true if the point xyz is to the left of the kart. 
 *  \param xyz Point to determine the direction 
 */
bool Attachment::isLeftSideOfKart(const Vec3 &xyz)
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
void Attachment::checkForHitKart(bool isSwattingLeft)
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
void Attachment::aimSwatter()
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
void Attachment::swatItem()
{
    if(UserConfigParams::logMisc())
        printf("[swatter] %s swatting item.\n",
               m_kart->getIdent().c_str());
    assert(m_type==ATTACH_SWATTER);
    assert(m_animation_target==NULL);

    m_animation_phase  = SWATTER_ITEM_1;
    m_animation_timer  = 
        m_kart->getKartProperties()->getSwatterItemAnimationTime();
    m_count--;
    m_rot_per_sec = core::vector3df(0, 0, SWAT_ANGLE) / m_animation_timer;
}   // swatItem
