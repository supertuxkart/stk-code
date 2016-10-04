//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2015 Joerg Henrichs
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
#include "achievements/achievement_info.hpp"
#include "audio/sfx_base.hpp"
#include "config/player_manager.hpp"
#include "config/stk_config.hpp"
#include "config/user_config.hpp"
#include "graphics/explosion.hpp"
#include "graphics/irr_driver.hpp"
#include "items/attachment_manager.hpp"
#include "items/item_manager.hpp"
#include "items/projectile_manager.hpp"
#include "items/swatter.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/controller/controller.hpp"
#include "karts/explosion_animation.hpp"
#include "karts/kart_properties.hpp"
#include "modes/three_strikes_battle.hpp"
#include "modes/world.hpp"
#include "network/rewind_manager.hpp"
#include "physics/triangle_mesh.hpp"
#include "tracks/track.hpp"
#include "physics/triangle_mesh.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"

/** Initialises the attachment each kart has.
 */
Attachment::Attachment(AbstractKart* kart)
          : EventRewinder()
{
    m_type                 = ATTACH_NOTHING;
    m_time_left            = 0.0;
    m_plugin               = NULL;
    m_kart                 = kart;
    m_previous_owner       = NULL;
    m_bomb_sound           = NULL;
    m_bubble_explode_sound = NULL;
    m_node_scale           = 1.0f;
    m_initial_speed        = 0.0f;

    // If we attach a NULL mesh, we get a NULL scene node back. So we
    // have to attach some kind of mesh, but make it invisible.
    m_node = irr_driver->addAnimatedMesh(
                         attachment_manager->getMesh(Attachment::ATTACH_BOMB), "bomb");
#ifdef DEBUG
    std::string debug_name = kart->getIdent()+" (attachment)";
    m_node->setName(debug_name.c_str());
#endif
    m_node->setAnimationEndCallback(this);
    m_node->setParent(m_kart->getNode());
    m_node->setVisible(false);
}   // Attachment

//-----------------------------------------------------------------------------
/** Removes the attachment object. It removes the scene node used to display
 *  the attachment, and stops any sfx from being played.
 */
Attachment::~Attachment()
{
    if(m_node)
        irr_driver->removeNode(m_node);

    if (m_bomb_sound)
    {
        m_bomb_sound->deleteSFX();
        m_bomb_sound = NULL;
    }

    if (m_bubble_explode_sound)
    {
        m_bubble_explode_sound->deleteSFX();
        m_bubble_explode_sound = NULL;
    }
}   // ~Attachment

//-----------------------------------------------------------------------------
/** Sets the attachment a kart has. This will also handle animation to be
 *  played, e.g. when a swatter replaces a bomb.
 *  \param type The type of the new attachment.
 *  \param time How long this attachment should stay with the kart.
 *  \param current_kart The kart from which an attachment is transferred.
 *         This is currently used for the bomb (to avoid that a bomb
 *         can be passed back to the previous owner). NULL if a no
 *         previous owner exists.
 */
void Attachment::set(AttachmentType type, float time,
                     AbstractKart *current_kart)
{
    bool was_bomb = (m_type == ATTACH_BOMB);
    scene::ISceneNode* bomb_scene_node = NULL;
    if (was_bomb && type == ATTACH_SWATTER)
    {
        // let's keep the bomb node, and create a new one for
        // the new attachment
        bomb_scene_node = m_node;

        m_node = irr_driver->addAnimatedMesh(
                     attachment_manager->getMesh(Attachment::ATTACH_BOMB), "bomb");
#ifdef DEBUG
        std::string debug_name = m_kart->getIdent() + " (attachment)";
        m_node->setName(debug_name.c_str());
#endif
        m_node->setAnimationEndCallback(this);
        m_node->setParent(m_kart->getNode());
        m_node->setVisible(false);
    }

    clear();
    m_node_scale = 0.3f;

    // If necessary create the appropriate plugin which encapsulates
    // the associated behavior
    switch(type)
    {
    case ATTACH_SWATTER :
        if (m_kart->getIdent() == "nolok")
            m_node->setMesh(attachment_manager->getMesh(ATTACH_NOLOKS_SWATTER));
        else
            m_node->setMesh(attachment_manager->getMesh(type));
        m_plugin = new Swatter(m_kart, was_bomb, bomb_scene_node);
        break;
    case ATTACH_BOMB:
        m_node->setMesh(attachment_manager->getMesh(type));
        m_node->setAnimationSpeed(0);
        if (m_bomb_sound) m_bomb_sound->deleteSFX();
        m_bomb_sound = SFXManager::get()->createSoundSource("clock");
        m_bomb_sound->setLoop(true);
        m_bomb_sound->setPosition(m_kart->getXYZ());
        m_bomb_sound->play();
        break;
    default:
        m_node->setMesh(attachment_manager->getMesh(type));
        break;
    }   // switch(type)

    if (!UserConfigParams::m_graphical_effects)
    {
        m_node->setAnimationSpeed(0);
        m_node->setCurrentFrame(0);
    }

    m_node->setScale(core::vector3df(m_node_scale,m_node_scale,m_node_scale));

    m_type             = type;
    m_time_left        = time;
    m_previous_owner   = current_kart;
    m_node->setRotation(core::vector3df(0, 0, 0));

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

    irr_driver->applyObjectPassShader(m_node);

    // Save event about the new attachment
    RewindManager *rwm = RewindManager::get();
    if(rwm->isEnabled() && !rwm->isRewinding())
    {
        BareNetworkString *buffer = new BareNetworkString(2);
        saveState(buffer);
        rwm->addEvent(this, buffer);
    }
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

    if (m_bomb_sound)
    {
        m_bomb_sound->deleteSFX();
        m_bomb_sound = NULL;
    }

    m_type=ATTACH_NOTHING;

    m_time_left=0.0;
    m_node->setVisible(false);
    m_node->setPosition(core::vector3df());
    m_node->setRotation(core::vector3df());

    // Resets the weight of the kart if the previous attachment affected it
    // (e.g. anvil). This must be done *after* setting m_type to
    // ATTACH_NOTHING in order to reset the physics parameters.
    m_kart->updateWeight();
}   // clear

// -----------------------------------------------------------------------------
/** Saves the attachment state. Called as part of the kart saving its state.
 *  \param buffer The kart rewinder's state buffer.
 */
void Attachment::saveState(BareNetworkString *buffer) const
{
    // We use bit 7 to indicate if a previous owner is defined for a bomb
    assert(ATTACH_MAX<=127);
    uint8_t type = m_type | (( (m_type==ATTACH_BOMB) && (m_previous_owner!=NULL) )
                             ? 0x80 : 0 );
    buffer->addUInt8(type);
    if(m_type!=ATTACH_NOTHING)
    {
        buffer->addFloat(m_time_left);
        if(m_type==ATTACH_BOMB && m_previous_owner)
            buffer->addUInt8(m_previous_owner->getWorldKartId());
        // m_initial_speed is not saved, on restore state it will
        // be set to the kart speed, which has already been restored
    }
}   // saveState

// -----------------------------------------------------------------------------
/** Called from the kart rewinder when resetting to a certain state.
 *  \param buffer The kart rewinder's buffer with the attachment state next.
 */
void Attachment::rewindTo(BareNetworkString *buffer)
{
    uint8_t type = buffer->getUInt8();
    AttachmentType new_type = AttachmentType(type & 0x7f);   // mask out bit 7

    // If there is no attachment, clear the attachment if necessary and exit
    if(new_type==ATTACH_NOTHING)
    {
        if(m_type!=new_type) clear();
        return;
    }

    float time_left = buffer->getFloat();

    // Attaching an object can be expensive (loading new models, ...)
    // so avoid doing this if there is no change in attachment type
    if(new_type == m_type)
    {
        setTimeLeft(time_left);
        return;
    }

    // Now it is a new attachment:

    if (type == (ATTACH_BOMB | 0x80))   // we have previous owner information
    {
        uint8_t kart_id = buffer->getUInt8();
        m_previous_owner = World::getWorld()->getKart(kart_id);
    }
    else
    {
        m_previous_owner = NULL;
    }
    set(new_type, time_left, m_previous_owner);
}   // rewindTo
// -----------------------------------------------------------------------------
/** Called when going forwards in time during a rewind. 
 *  \param buffer Buffer with the rewind information.
 */
void Attachment::rewind(BareNetworkString *buffer)
{
    // Event has same info as a state, so re-use the restore function
    rewindTo(buffer);
}   // rewind

// -----------------------------------------------------------------------------
/** Randomly selects the new attachment. For a server process, the
*   attachment can be passed into this function.
*  \param item The item that was collected.
*  \param new_attachment Optional: only used on the clients, it
*                        specifies the new attachment to use
*/
void Attachment::hitBanana(Item *item, int new_attachment)
{
    if(m_kart->getController()->canGetAchievements())
        PlayerManager::increaseAchievement(AchievementInfo::ACHIEVE_BANANA,
                                           "banana",1                      );
    //Bubble gum shield effect:
    if(m_type == ATTACH_BUBBLEGUM_SHIELD ||
       m_type == ATTACH_NOLOK_BUBBLEGUM_SHIELD)
    {
        m_time_left = 0.0f;
        return;
    }

    float leftover_time   = 0.0f;

    bool add_a_new_item = true;

    if (dynamic_cast<ThreeStrikesBattle*>(World::getWorld()) != NULL)
    {
        World::getWorld()->kartHit(m_kart->getWorldKartId());
        ExplosionAnimation::create(m_kart);
        return;
    }

    const KartProperties *kp = m_kart->getKartProperties();
    switch(getType())   // If there already is an attachment, make it worse :)
    {
    case ATTACH_BOMB:
        {
        add_a_new_item = false;
        HitEffect *he = new Explosion(m_kart->getXYZ(), "explosion", "explosion_bomb.xml");
        if(m_kart->getController()->isLocalPlayerController())
            he->setLocalPlayerKartHit();
        projectile_manager->addHitEffect(he);
        ExplosionAnimation::create(m_kart);
        clear();
        if(new_attachment==-1)
            new_attachment = m_random.get(3);
        // Disable the banana on which the kart just is for more than the
        // default time. This is necessary to avoid that a kart lands on the
        // same banana again once the explosion animation is finished, giving
        // the kart the same penalty twice.
        float f = std::max(item->getDisableTime(), kp->getExplosionDuration() + 2.0f);
        item->setDisableTime(f);
        break;
        }
    case ATTACH_ANVIL:
        // if the kart already has an anvil, attach a new anvil,
        // and increase the overall time
        new_attachment = 1;
        leftover_time  = m_time_left;
        break;
    case ATTACH_PARACHUTE:
        new_attachment = 0;
        leftover_time  = m_time_left;
        break;
    default:
        // There is no attachment currently, but there will be one
        // so play the character sound ("Uh-Oh")
        m_kart->playCustomSFX(SFXManager::CUSTOM_ATTACH);

        if(new_attachment==-1)
        {
            if(race_manager->getMinorMode() == RaceManager::MINOR_MODE_TIME_TRIAL)
                new_attachment = m_random.get(2);
            else
                new_attachment = m_random.get(3);
        }
    }   // switch

    if (add_a_new_item)
    {
        switch (new_attachment)
        {
        case 0:
            set(ATTACH_PARACHUTE, kp->getParachuteDuration() + leftover_time);
            m_initial_speed = m_kart->getSpeed();

            // if going very slowly or backwards,
            // braking won't remove parachute
            if(m_initial_speed <= 1.5) m_initial_speed = 1.5;
            break ;
        case 1:
            set(ATTACH_ANVIL, kp->getAnvilDuration() + leftover_time);
            // if ( m_kart == m_kart[0] )
            //   sound -> playSfx ( SOUND_SHOOMF ) ;
            // Reduce speed once (see description above), all other changes are
            // handled in Kart::updatePhysics
            m_kart->adjustSpeed(kp->getAnvilSpeedFactor());
            m_kart->updateWeight();
            break ;
        case 2:
            set( ATTACH_BOMB, stk_config->m_bomb_time+leftover_time);

            // if ( m_kart == m_kart[0] )
            //   sound -> playSfx ( SOUND_SHOOMF ) ;
            break ;
        }   // switch
    }
}   // hitBanana

//-----------------------------------------------------------------------------
/** Updates the attachments in case of a kart-kart collision. This must only
 *  be called for one of the karts in the collision, since it will update
 *  the attachment for both karts.
 *  \param other Pointer to the other kart hit.
 */
void Attachment::handleCollisionWithKart(AbstractKart *other)
{
    Attachment *attachment_other=other->getAttachment();

    if(getType()==Attachment::ATTACH_BOMB)
    {
        // Don't attach a bomb when the kart is shielded
        if(other->isShielded())
        {
            other->decreaseShieldTime();
            return;
        }
        // If both karts have a bomb, explode them immediately:
        if(attachment_other->getType()==Attachment::ATTACH_BOMB)
        {
            setTimeLeft(0.0f);
            attachment_other->setTimeLeft(0.0f);
        }
        else  // only this kart has a bomb, move it to the other
        {
            // if there are only two karts, let them switch bomb from one to other
            if (getPreviousOwner() != other || World::getWorld()->getNumKarts() <= 2)
            {
                // Don't move if this bomb was from other kart originally
                other->getAttachment()->set(ATTACH_BOMB,
                                            getTimeLeft()+
                                            stk_config->m_bomb_time_increase,
                                            m_kart);
                other->playCustomSFX(SFXManager::CUSTOM_ATTACH);
                clear();
            }
        }
    }   // type==BOMB
    else if(attachment_other->getType()==Attachment::ATTACH_BOMB &&
            (attachment_other->getPreviousOwner()!=m_kart || World::getWorld()->getNumKarts() <= 2))
    {
        // Don't attach a bomb when the kart is shielded
        if(m_kart->isShielded())
        {
            m_kart->decreaseShieldTime();
            return;
        }
        set(ATTACH_BOMB, other->getAttachment()->getTimeLeft()+
                         stk_config->m_bomb_time_increase, other);
        other->getAttachment()->clear();
        m_kart->playCustomSFX(SFXManager::CUSTOM_ATTACH);
    }
    else
    {
        m_kart->playCustomSFX(SFXManager::CUSTOM_CRASH);
        other->playCustomSFX(SFXManager::CUSTOM_CRASH);
    }

}   // handleCollisionWithKart

//-----------------------------------------------------------------------------
void Attachment::update(float dt)
{
    if(m_type==ATTACH_NOTHING) return;
    m_time_left -=dt;


    bool is_shield = (m_type == ATTACH_BUBBLEGUM_SHIELD|| m_type == ATTACH_NOLOK_BUBBLEGUM_SHIELD);
    float m_wanted_node_scale = is_shield ? std::max(1.0f, m_kart->getHighestPoint()*1.1f) : 1.0f;

    if (m_node_scale < m_wanted_node_scale)
    {
        m_node_scale += dt*1.5f;
        if (m_node_scale > m_wanted_node_scale) m_node_scale = m_wanted_node_scale;
        m_node->setScale(core::vector3df(m_node_scale,m_node_scale,m_node_scale));
    }

    if(m_plugin)
    {
        bool discard = m_plugin->updateAndTestFinished(dt);
        if(discard)
        {
            clear();  // also removes the plugin
            return;
        }
    }

    switch (m_type)
    {
    case ATTACH_PARACHUTE:
        {
        // Partly handled in Kart::updatePhysics
        // Otherwise: disable if a certain percantage of
        // initial speed was lost
        // This percentage is based on the ratio of
        // initial_speed / initial_max_speed

        const KartProperties *kp = m_kart->getKartProperties();

        float f = m_initial_speed / kp->getParachuteMaxSpeed();
        if (f > 1.0f) f = 1.0f;   // cap fraction
        if (m_kart->getSpeed() <= m_initial_speed *
                                 (kp->getParachuteLboundFraction() +
                                  f * (kp->getParachuteUboundFraction()
                                     - kp->getParachuteLboundFraction())))
        {
            m_time_left = -1;
        }
        }
        break;
    case ATTACH_ANVIL:     // handled in Kart::updatePhysics
    case ATTACH_NOTHING:   // Nothing to do, but complete all cases for switch
    case ATTACH_MAX:
        break;
    case ATTACH_SWATTER:
        // Everything is done in the plugin.
        break;
    case ATTACH_NOLOKS_SWATTER:
        // Should never be called, this symbols is only used as an index for
        // the model, Nolok's attachment type is ATTACH_SWATTER
        assert(false);
        break;
    case ATTACH_BOMB:

        if (m_bomb_sound) m_bomb_sound->setPosition(m_kart->getXYZ());

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
            HitEffect *he = new Explosion(m_kart->getXYZ(), "explosion", "explosion_bomb.xml");
            if(m_kart->getController()->isLocalPlayerController())
                he->setLocalPlayerKartHit();
            projectile_manager->addHitEffect(he);
            ExplosionAnimation::create(m_kart);

            if (m_bomb_sound)
            {
                m_bomb_sound->deleteSFX();
                m_bomb_sound = NULL;
            }
        }
        break;
    case ATTACH_TINYTUX:
        // Nothing to do for tinytux, this is all handled in EmergencyAnimation
        break;
    case ATTACH_BUBBLEGUM_SHIELD:
    case ATTACH_NOLOK_BUBBLEGUM_SHIELD:
        if (m_time_left < 0)
        {
            m_time_left = 0.0f;
            if (m_bubble_explode_sound) m_bubble_explode_sound->deleteSFX();
            m_bubble_explode_sound = SFXManager::get()->createSoundSource("bubblegum_explode");
            m_bubble_explode_sound->setPosition(m_kart->getXYZ());
            m_bubble_explode_sound->play();

            // drop a small bubble gum
            Vec3 hit_point;
            Vec3 normal;
            const Material* material_hit;
            World* world = World::getWorld();
            world->getTrack()->getTriangleMesh().castRay(m_kart->getXYZ(),
                m_kart->getTrans().getBasis() * Vec3(0, -10000, 0), &hit_point,
                &material_hit, &normal);
            // This can happen if the kart is 'over nothing' when dropping
            // the bubble gum
            if(material_hit)
            {
                normal.normalize();

                Vec3 pos = hit_point + m_kart->getTrans().getBasis() * Vec3(0, -0.05f, 0);
                ItemManager::get()->newItem(Item::ITEM_BUBBLEGUM, pos, normal, m_kart);
            }
        }
        break;
    }   // switch

    // Detach attachment if its time is up.
    if ( m_time_left <= 0.0f)
        clear();
}   // update

// ----------------------------------------------------------------------------
float Attachment::weightAdjust() const
{
    return m_type == ATTACH_ANVIL ? m_kart->getKartProperties()->getAnvilWeight() : 0.0f;
}

// ----------------------------------------------------------------------------
/** Inform any eventual plugin when an animation is done. */
void Attachment::OnAnimationEnd(scene::IAnimatedMeshSceneNode* node)
{
    if(m_plugin)
        m_plugin->onAnimationEnd();
}
