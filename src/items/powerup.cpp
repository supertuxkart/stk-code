//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2013 Joerg Henrichs
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

#include "items/powerup.hpp"

#include "achievements/achievement_info.hpp"
#include "config/player_manager.hpp"

#include "audio/sfx_base.hpp"
#include "audio/sfx_manager.hpp"
#include "config/stk_config.hpp"
#include "items/attachment.hpp"
#include "items/item_manager.hpp"
#include "items/projectile_manager.hpp"
#include "items/rubber_ball.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/controller/controller.hpp"
#include "karts/kart_properties.hpp"
#include "modes/world.hpp"
#include "physics/triangle_mesh.hpp"
#include "tracks/track.hpp"
#include "utils/string_utils.hpp"
#include "utils/log.hpp" //TODO: remove after debugging is done

//-----------------------------------------------------------------------------
/** Constructor, stores the kart to which this powerup belongs.
 *  \param kart The kart to which this powerup belongs.
 */
Powerup::Powerup(AbstractKart* kart)
{
    m_owner               = kart;
    m_sound_use           = NULL;
    reset();
}   // Powerup

//-----------------------------------------------------------------------------
/** Frees the memory for the sound effects.
 */
Powerup::~Powerup()
{
    if(m_sound_use) m_sound_use->deleteSFX();
}   // ~Powerup

//-----------------------------------------------------------------------------
/** Resets the powerup, called at begin of a race.
 */
void Powerup::reset()
{
    m_type = PowerupManager::POWERUP_NOTHING;
    m_number = 0;

    int type, number;
    World::getWorld()->getDefaultCollectibles( &type, &number );
    set( (PowerupManager::PowerupType)type, number );
}   // reset

//-----------------------------------------------------------------------------
/** Sets the collected items. The number of items is increased if the same
 *  item is currently collected, otherwise replaces the existing item. It also
 *  sets item specific sounds.
 *  \param type Thew new type.
 *  \param n Number of items of the given type.
 */
void Powerup::set(PowerupManager::PowerupType type, int n)
{
    if (m_type==type)
    {
        m_number+=n;
        return;
    }
    m_type=type;
    m_number=n;

    if(m_sound_use != NULL)
    {
        m_sound_use->deleteSFX();
        m_sound_use = NULL;
    }

    switch (m_type)
    {
        // No sound effect when arming the glove
        case PowerupManager::POWERUP_SWATTER:
            break;

        case PowerupManager::POWERUP_ZIPPER:
            break ;

        case PowerupManager::POWERUP_BOWLING:
            m_sound_use = SFXManager::get()->createSoundSource("bowling_shoot");
            break ;

        case PowerupManager::POWERUP_ANVIL:
            m_sound_use = SFXManager::get()->createSoundSource("anvil");
            break;

        case PowerupManager::POWERUP_PARACHUTE:
            m_sound_use = SFXManager::get()->createSoundSource("parachute");
            break;

        case PowerupManager::POWERUP_BUBBLEGUM:
                m_sound_use = SFXManager::get()->createSoundSource("goo");
            break ;

        case PowerupManager::POWERUP_SWITCH:
            m_sound_use = SFXManager::get()->createSoundSource("swap");
            break;

        case PowerupManager::POWERUP_NOTHING:
        case PowerupManager::POWERUP_CAKE:
        case PowerupManager::POWERUP_PLUNGER:
        default :
            m_sound_use = SFXManager::get()->createSoundSource("shoot");
            break ;
    }

}  // set

//-----------------------------------------------------------------------------
/** Returns the icon for the currently collected powerup. Used in the
 *  race_gui to display the collected item.
 */
Material *Powerup::getIcon() const
{
    // Check if it's one of the types which have a separate
    // data file which includes the icon:
    return powerup_manager->getIcon(m_type);
}   // getIcon

//-----------------------------------------------------------------------------
/** Does the sound configuration.
 */
void  Powerup::adjustSound()
{
    m_sound_use->setPosition(m_owner->getXYZ());
    // in multiplayer mode, sounds are NOT positional (because we have multiple listeners)
    // so the sounds of all AIs are constantly heard. So reduce volume of sounds.
    if (race_manager->getNumLocalPlayers() > 1)
    {
        // player karts played at full volume; AI karts much dimmer

        if (m_owner->getController()->isPlayerController())
        {
            m_sound_use->setVolume( 1.0f );
        }
        else
        {
            m_sound_use->setVolume( 
                     std::min(0.5f, 1.0f / race_manager->getNumberOfKarts()) );
        }
    }
}   // adjustSound

//-----------------------------------------------------------------------------
/** Use (fire) this powerup.
 */
void Powerup::use()
{
    // The player gets an achievement point for using a powerup
    StateManager::ActivePlayer * player = m_owner->getController()->getPlayer();
    if (m_type != PowerupManager::POWERUP_NOTHING &&
        player != NULL && player->getConstProfile() == PlayerManager::getCurrentPlayer())
    {
        PlayerManager::increaseAchievement(AchievementInfo::ACHIEVE_POWERUP_LOVER, "poweruplover");
    }

    // Play custom kart sound when collectible is used //TODO: what about the bubble gum?
    if (m_type != PowerupManager::POWERUP_NOTHING &&
        m_type != PowerupManager::POWERUP_SWATTER &&
        m_type != PowerupManager::POWERUP_ZIPPER)
        m_owner->playCustomSFX(SFXManager::CUSTOM_SHOOT);

    // FIXME - for some collectibles, set() is never called
    if(m_sound_use == NULL)
    {
        //if (m_type == POWERUP_SWITCH) m_sound_use = SFXManager::get()->newSFX(SFXManager::SOUND_SWAP);
        //else
        m_sound_use = SFXManager::get()->createSoundSource("shoot");
    }

    m_number--;
    World *world = World::getWorld();
    switch (m_type)
    {
    case PowerupManager::POWERUP_ZIPPER:
        m_owner->handleZipper(NULL, true);
        break ;
    case PowerupManager::POWERUP_SWITCH:
        {
            ItemManager::get()->switchItems();
            m_sound_use->setPosition(m_owner->getXYZ());
            m_sound_use->play();
            break;
        }
    case PowerupManager::POWERUP_CAKE:
    case PowerupManager::POWERUP_RUBBERBALL:
    case PowerupManager::POWERUP_BOWLING:
    case PowerupManager::POWERUP_PLUNGER:
        if(stk_config->m_shield_restrict_weapos)
            m_owner->setShieldTime(0.0f); // make weapon usage destroy the shield
        Powerup::adjustSound();
        m_sound_use->play();

        projectile_manager->newProjectile(m_owner, m_type);
        break ;

    case PowerupManager::POWERUP_SWATTER:
        m_owner->getAttachment()
                ->set(Attachment::ATTACH_SWATTER,
                      m_owner->getKartProperties()->getSwatterDuration());
        break;

    case PowerupManager::POWERUP_BUBBLEGUM:
        // use the bubble gum the traditional way, if the kart is looking back
        if (m_owner->getControls().m_look_back)
        {
            Vec3 hit_point;
            Vec3 normal;
            const Material* material_hit;
            Vec3 pos = m_owner->getXYZ();
            Vec3 to=pos+Vec3(0, -10000, 0);
            world->getTrack()->getTriangleMesh().castRay(pos, to, &hit_point,
                                                     &material_hit, &normal);
            // This can happen if the kart is 'over nothing' when dropping
            // the bubble gum
            if(!material_hit)
                return;
            normal.normalize();

            Powerup::adjustSound();
            m_sound_use->play();

            pos.setY(hit_point.getY()-0.05f);

            ItemManager::get()->newItem(Item::ITEM_BUBBLEGUM, pos, normal, m_owner);
        }
        else // if the kart is looking forward, use the bubblegum as a shield
        {

            if(!m_owner->isShielded()) //if the previous shield had been used up.
            {
                if (m_owner->getIdent() == "nolok")
                {
                    m_owner->getAttachment()->set(Attachment::ATTACH_NOLOK_BUBBLEGUM_SHIELD,
                                                   stk_config->m_bubblegum_shield_time);
                }
                else
                {
                    m_owner->getAttachment()->set(Attachment::ATTACH_BUBBLEGUM_SHIELD,
                                                   stk_config->m_bubblegum_shield_time);
                }
            }
            else // using a bubble gum while still having a shield
            {
                if (m_owner->getIdent() == "nolok")
                {
                    m_owner->getAttachment()->set(Attachment::ATTACH_NOLOK_BUBBLEGUM_SHIELD,
                                                  stk_config->m_bubblegum_shield_time + m_owner->getShieldTime());
                }
                else
                {
                    m_owner->getAttachment()->set(Attachment::ATTACH_BUBBLEGUM_SHIELD,
                                                  stk_config->m_bubblegum_shield_time + m_owner->getShieldTime());
                }
            }

            m_sound_use = SFXManager::get()->createSoundSource("inflate");//Extraordinary. Usually sounds are set in Powerup::set()
            //In this case this is a workaround, since the bubblegum item has two different sounds.

            Powerup::adjustSound();
            m_sound_use->play();

        }   // end of PowerupManager::POWERUP_BUBBLEGUM
        break;

    case PowerupManager::POWERUP_ANVIL:
        //Attach an anvil(twice as good as the one given
        //by the bananas) to the kart in the 1st position.
        for(unsigned int i = 0 ; i < world->getNumKarts(); ++i)
        {
            AbstractKart *kart=world->getKart(i);
            if(kart->isEliminated()) continue;
            if(kart == m_owner) continue;
            if(kart->getPosition() == 1)
            {
                kart->getAttachment()->set(Attachment::ATTACH_ANVIL,
                                           stk_config->m_anvil_time);
                kart->updateWeight();
                kart->adjustSpeed(stk_config->m_anvil_speed_factor*0.5f);

                // should we position the sound at the kart that is hit,
                // or the kart "throwing" the anvil? Ideally it should be both.
                // Meanwhile, don't play it near AI karts since they obviously
                // don't hear anything
                if(kart->getController()->isPlayerController())
                    m_sound_use->setPosition(kart->getXYZ());
                else
                    m_sound_use->setPosition(m_owner->getXYZ());

                m_sound_use->play();
                break;
            }
        }

        break;

    case PowerupManager::POWERUP_PARACHUTE:
        {
            AbstractKart* player_kart = NULL;
            //Attach a parachutte(that last twice as long as the
            //one from the bananas) to all the karts that
            //are in front of this one.
            for(unsigned int i = 0 ; i < world->getNumKarts(); ++i)
            {
                AbstractKart *kart=world->getKart(i);
                if(kart->isEliminated() || kart== m_owner) continue;
                if(kart->isShielded())
                {
                    kart->decreaseShieldTime();
                    continue;
                }
                if(m_owner->getPosition() > kart->getPosition())
                {
                    kart->getAttachment()
                        ->set(Attachment::ATTACH_PARACHUTE,
                              stk_config->m_parachute_time_other);

                    if(kart->getController()->isPlayerController())
                        player_kart = kart;
                }
            }

            // should we position the sound at the kart that is hit,
            // or the kart "throwing" the anvil? Ideally it should be both.
            // Meanwhile, don't play it near AI karts since they obviously
            // don't hear anything
            if(m_owner->getController()->isPlayerController())
                m_sound_use->setPosition(m_owner->getXYZ());
            else if(player_kart)
                m_sound_use->setPosition(player_kart->getXYZ());
            m_sound_use->play();
        }
        break;

    case PowerupManager::POWERUP_NOTHING:
        {
            if(!m_owner->getKartAnimation())
                m_owner->beep();
        }
        break;
    default : break;
    }

    if ( m_number <= 0 )
    {
        m_number = 0;
        m_type   = PowerupManager::POWERUP_NOTHING;
    }
}   // use

//-----------------------------------------------------------------------------
/** This function is called when a bnous box is it. This function can be
 *  called on a server (in which case item and add_info are not used),
 *  or on a client, in which case the item and additional info is used
 *  to make sure server and clients are synched correctly.
 *  \param n
 *  \param item The item (bonux box) that was hit. This is necessary
 *         for servers so that the clients can be informed which item
 *         was collected.
 *  \param add_info Additional information. This is used for network games
 *         so that the server can overwrite which item is collectted
 *         (otherwise a random choice is done).
 */
void Powerup::hitBonusBox(const Item &item, int add_info)
{
    // Position can be -1 in case of a battle mode (which doesn't have
    // positions), but this case is properly handled in getRandomPowerup.
    int position = m_owner->getPosition();

    unsigned int n=1;
    PowerupManager::PowerupType new_powerup;

    // Check if rubber ball is the current power up held by the kart. If so,
    // reset the bBallCollectTime to 0 before giving new powerup.
    if(m_type == PowerupManager::POWERUP_RUBBERBALL)
        powerup_manager->setBallCollectTime(0);

    // Check if two bouncing balls are collected less than getRubberBallTimer()
    //seconds apart. If yes, then call getRandomPowerup again. If no, then break.
    if (add_info<0)
    {
        for(int i=0; i<20; i++)
        {
            new_powerup = powerup_manager->getRandomPowerup(position, &n);
            if(new_powerup != PowerupManager::POWERUP_RUBBERBALL ||
                ( World::getWorld()->getTime() - powerup_manager->getBallCollectTime()) >
                  RubberBall::getTimeBetweenRubberBalls() )
                break;
        }
    }
    else // set powerup manually
    {
        new_powerup = (PowerupManager::PowerupType)((add_info>>4)&0x0f); // highest 4 bits for the type
        n = (add_info&0x0f); // last 4 bits for the amount
    }

    if(new_powerup == PowerupManager::POWERUP_RUBBERBALL)
        powerup_manager->setBallCollectTime(World::getWorld()->getTime());

    // Always add a new powerup in ITEM_MODE_NEW (or if the kart
    // doesn't have a powerup atm).
    if(m_type == PowerupManager::POWERUP_NOTHING ||
       stk_config->m_same_powerup_mode == STKConfig::POWERUP_MODE_NEW )
    {
        set( new_powerup, n );
    }
    else
    {
        // If powerup mode is 'SAME', or it's ONLY_IF_SAME and it is the
        // same powerup, increase the number of items.
        if(stk_config->m_same_powerup_mode == STKConfig::POWERUP_MODE_SAME ||
            new_powerup==m_type)
        {
            m_number+=n;
            if(m_number > MAX_POWERUPS)
                m_number = MAX_POWERUPS;
        }
    }
    // Ignore new powerup if it is different from the current one and not
    // POWERUP_MODE_SAME

}   // hitBonusBox
