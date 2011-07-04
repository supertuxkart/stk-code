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
#include "items/swatter.hpp"
#include "karts/kart.hpp"
#include "modes/three_strikes_battle.hpp"
#include "network/race_state.hpp"
#include "network/network_manager.hpp"
#include "utils/constants.hpp"

Attachment::Attachment(Kart* kart)
{
    m_type           = ATTACH_NOTHING;
    m_time_left      = 0.0;
    m_plugin         = NULL;
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
    
    // If necessary create the appropriate plugin which encapsulates 
    switch(type)
    {
    case ATTACH_SWATTER : 
        m_plugin = new Swatter(this, m_kart);
    default: break;
    }   // switch(type)

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

    // A parachute can be attached as result of the usage of an item. In this
    // case we have to save the current kart speed so that it can be detached
    // by slowing down.
    if(m_type==ATTACH_PARACHUTE)
    {
        m_initial_speed = m_kart->getSpeed();
        // if going very slowly or backwards, braking won't remove parachute
        if(m_initial_speed <= 1.5) m_initial_speed = 1.5; 
        
        if (UserConfigParams::m_graphical_effects)
        {
            // .blend was created @25 (<10 real, slow computer), make it faster
            m_node->setAnimationSpeed(50);
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
    if(m_plugin)
    {
        delete m_plugin;
        m_plugin = NULL;
    }

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
        projectile_manager->newExplosion(m_kart->getXYZ(), "explosion", 
                                m_kart->getController()->isPlayerController());
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

             // if going very slowly or backwards, braking won't remove parachute
            if(m_initial_speed <= 1.5) m_initial_speed = 1.5;
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

    if(m_plugin)
    {
        bool discard = m_plugin->updateAndTestFinished(dt);
        if(discard)
        {
            clear();  // also removes the plugin
            return;
        }
        m_node->setRotation(m_plugin->getRotation());
    }

    switch (m_type)
    {
    case ATTACH_PARACHUTE:
        // Partly handled in Kart::updatePhysics
        // Otherwise: disable if a certain percantage of
        // initial speed was lost
        if(m_kart->getSpeed() <= 
            m_initial_speed*stk_config->m_parachute_done_fraction)
        {
            m_time_left = -1;
        }
        break;
    case ATTACH_ANVIL:     // handled in Kart::updatePhysics
    case ATTACH_NOTHING:   // Nothing to do, but complete all cases for switch
    case ATTACH_MAX:
        break;
    case ATTACH_SWATTER:
        // Everything is done in the plugin.
        break;
    case ATTACH_BOMB:
        // Mesh animation frames are 1 to 61 frames (60 steps)
        // The idea is change second by second, counterclockwise 60 to 0 secs
        // If longer times needed, it should be a surprise "oh! bomb activated!"
        if(m_time_left <= (m_node->getEndFrame() - m_node->getStartFrame()-1))
        {
            m_node->setCurrentFrame(m_node->getEndFrame() 
                                    - m_node->getStartFrame()-1-m_time_left);
        }
        if(m_time_left<=0.0)
        {
            projectile_manager->newExplosion(m_kart->getXYZ(), "explosion", 
                                m_kart->getController()->isPlayerController());
            m_kart->handleExplosion(m_kart->getXYZ(), 
                                    /*direct_hit*/ true);
        }
        break;
    case ATTACH_TINYTUX:
        // Nothing to do for tinytux, this is all handled in EmergencyAnimation
        break;
    }   // switch

    // Detach attachment if its time is up.
    if ( m_time_left <= 0.0f)
        clear();
}   // update

// ----------------------------------------------------------------------------
/** Uses the swatter to swat at an item. The actual functionality is 
 *  implemented in the swatter plugin.
 *  \pre The item must be an attachment (and m_plugin must be a swatter).
 */
void Attachment::swatItem()
{
    assert(m_type==ATTACH_SWATTER);
    ((Swatter*)m_plugin)->swatItem();
}   // swatItem
// ----------------------------------------------------------------------------
/** Returns if the swatter is currently aiming, i.e. can be used to
 *  swat an incoming projectile. */
bool Attachment::isSwatterReady() const 
{
    assert(m_type==ATTACH_SWATTER);
    return ((Swatter*)m_plugin)->isSwatterReady();
}   // isSwatterReady
