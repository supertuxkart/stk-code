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

#include "items/powerup.hpp"

#include "audio/sfx_base.hpp"
#include "audio/sfx_manager.hpp"
#include "config/player_manager.hpp"
#include "config/stk_config.hpp"
#include "items/attachment.hpp"
#include "items/item_manager.hpp"
#include "items/powerup_audio.hpp"
#include "items/projectile_manager.hpp"
#include "items/rubber_ball.hpp"
#include "karts/kart.hpp"
#include "karts/controller/controller.hpp"
#include "karts/kart_properties.hpp"
#include "modes/world.hpp"
#include "network/network_config.hpp"
#include "network/network_string.hpp"
#include "network/rewind_manager.hpp"
#include "physics/triangle_mesh.hpp"
#include "tracks/track.hpp"
#include "utils/string_utils.hpp"
#include "utils/log.hpp" //TODO: remove after debugging is done

//-----------------------------------------------------------------------------
/** Constructor, stores the kart to which this powerup belongs.
 *  \param kart The kart to which this powerup belongs.
 */
Powerup::Powerup(Kart* kart)
{
    m_kart      = kart;
    reset();
}   // Powerup

//-----------------------------------------------------------------------------
/** Resets the powerup, called at begin of a race.
 */
void Powerup::reset()
{
    m_type = PowerupManager::POWERUP_NOTHING;
    m_mini_state = PowerupManager::NOT_MINI;
    m_number = 0;

    // Ghost kart will update powerup every frame
    if (m_kart->isGhostKart())
        return;
    int type, number;
    World::getWorld()->getDefaultCollectibles( &type, &number );
    set( (PowerupManager::PowerupType)type, number );
}   // reset

//-----------------------------------------------------------------------------
/** Save the powerup state. Called from the kart rewinder when saving the kart
 *  state or when a new powerup even is saved.
 *  \param buffer The buffer into which to save the state.
 */
void Powerup::saveState(BareNetworkString *buffer) const
{
    buffer->addUInt8(uint8_t(m_type));
    buffer->addUInt8(m_number);   // number is <=255
}   // saveState

//-----------------------------------------------------------------------------
/** Restore a powerup state. Called from the kart rewinder when restoring a
 *  state.
 *  \param buffer Buffer with the state of this powerup object.
 */
void Powerup::rewindTo(BareNetworkString *buffer)
{
    PowerupManager::PowerupType new_type = 
        PowerupManager::PowerupType(buffer->getUInt8());
    int n=0;
    if(new_type==PowerupManager::POWERUP_NOTHING)
    {
        set(new_type, 0);
        return;
    }
    n = buffer->getUInt8();
    if(m_type == new_type)
        m_number = n;
    else
    {
        m_number = 0;
        set(new_type, n);
    }
}   // rewindTo

//-----------------------------------------------------------------------------
void Powerup::update(int ticks)
{
    PowerupAudio::getInstance()->update(m_kart, ticks);
}   // update

//-----------------------------------------------------------------------------
/** Sets the collected items. The number of items is increased if the same
 *  item is currently collected, otherwise replaces the existing item.
 *  \param type Thew new type.
 *  \param n Number of items of the given type.
 */
void Powerup::set(PowerupManager::PowerupType type, int n)
{
    if (m_type==type)
    {
        m_number+=n;
    }
    else
    {
        m_type=type;
        m_number=n;

        if (m_type == PowerupManager::POWERUP_MINI)
            m_mini_state = PowerupManager::MINI_SELECT;
    }

    // Limit to 255 (save space in network state saving)
    if(n>255) n = 255;
}  // set

//-----------------------------------------------------------------------------
/** Sets the amount of the current collected item.
 *  \param n Number of items.
 */
void Powerup::setNum(int n)
{
    // Limit to 255 (save space in network state saving)
    if(n>255) n = 255;

    m_number=n;
}

//-----------------------------------------------------------------------------
/** Returns the icon for the currently collected powerup. Used in the
 *  race_gui to display the collected item.
 */
Material *Powerup::getIcon(bool wide) const
{
    // The mini powerup has multiple states,
    // and each state has its own icon.
    if (m_type == PowerupManager::POWERUP_MINI)
    {
        int wide_offset = wide ? 6 : 0;

        if (m_mini_state == PowerupManager::MINI_ZIPPER)
            return powerup_manager->getMiniIcon(3+wide_offset);
        else if (m_mini_state == PowerupManager::MINI_CAKE)
            return powerup_manager->getMiniIcon(4+wide_offset);
        else if (m_mini_state == PowerupManager::MINI_GUM)
            return powerup_manager->getMiniIcon(5+wide_offset);

        // FIXME
        // This duplicates the logic to determine which powerup would be
        // selected by the current cycle

        // If not in one of the mini-powerup-specific states,
        // We assume that the mini_state is MINI_SELECT for correct display
        // This may not always be true in networking mode right after item collection.
        int cycle_ticks = stk_config->time2Ticks(0.65f);
        int cycle_value = World::getWorld()->getTicksSinceStart() % (3 * cycle_ticks);
        if (cycle_value < cycle_ticks)
            return powerup_manager->getMiniIcon(0+wide_offset);
        else if (cycle_value < 2*cycle_ticks)
            return powerup_manager->getMiniIcon(1+wide_offset);
        else
            return powerup_manager->getMiniIcon(2+wide_offset);
    }

    // Check if it's one of the types which have a separate
    // data file which includes the icon:
    return powerup_manager->getIcon(m_type);
}   // getIcon

//-----------------------------------------------------------------------------
/** Use (fire) this powerup.
 */
void Powerup::use()
{
    const KartProperties *kp = m_kart->getKartProperties();

    // The player gets an achievement point for using a powerup
    if (m_type != PowerupManager::POWERUP_NOTHING      &&
        m_kart->getController()->canGetAchievements()    )
    {
        PlayerManager::increaseAchievement(AchievementsStatus::POWERUP_USED, 1);
        if (RaceManager::get()->isLinearRaceMode())
            PlayerManager::increaseAchievement(AchievementsStatus::POWERUP_USED_1RACE, 1);
    }

    m_number--;
    World *world = World::getWorld();
    ItemManager* im = Track::getCurrentTrack()->getItemManager();

    // Some powerups can play different sounds depending on the situation
    int sound_type = 1;
    PowerupManager::MiniState audio_mini_state = m_mini_state;
    // Some powerups can play a sound located at the target kart instead
    // of the kart that used the powerup (e. g. parachute)
    // To avoid using many sound sources (one per affected kart),
    // we play the sound only if a player kart is affected.
    Kart* player_kart = NULL;

    switch (m_type)
    {
    case PowerupManager::POWERUP_ZIPPER:
        {
            m_kart->handleZipper(NULL, true);
            break ;
        }
    case PowerupManager::POWERUP_SWITCH:
        {
            im->switchItems();
            break;
        }
    case PowerupManager::POWERUP_CAKE:
    case PowerupManager::POWERUP_RUBBERBALL:
    case PowerupManager::POWERUP_BOWLING:
    case PowerupManager::POWERUP_PLUNGER:
        {
            // make weapon usage destroy gum shields
            if(stk_config->m_shield_restrict_weapons &&
                m_kart->isGumShielded())
                m_kart->decreaseShieldTime();
            ProjectileManager::get()->newProjectile(m_kart, m_type);
            break;
        }
    case PowerupManager::POWERUP_SWATTER:
        {
            // Instead of having the swatter attachment just replace the gum shield,
            // ensure the gum is popped on the ground.
            // DecreaseShieldTime would not work here because the new attachment
            // would be set before the next attachment update.
            if(m_kart->isGumShielded())
                m_kart->getAttachment()->popGumShield();
            m_kart->getAttachment()->set(Attachment::ATTACH_SWATTER,
                stk_config->time2Ticks(kp->getSwatterDuration()));
            break;
        }
    case PowerupManager::POWERUP_SUDO:
        {
            sound_type = useNitroHack();
            break;
        } // end of PowerupManager::POWERUP_SUDO
    case PowerupManager::POWERUP_ELECTRO:
        {
            // This takes care of the speed boost
            m_kart->setElectroShield();

            // We set the attachment
            m_kart->getAttachment()->set(Attachment::ATTACH_ELECTRO_SHIELD,
                stk_config->time2Ticks(kp->getElectroDuration()));

            break;
        } // end of PowerupManager::POWERUP_ELECTRO
    case PowerupManager::POWERUP_MINI:
        {
            // Lock the selected mini-powerup
            if (m_mini_state == PowerupManager::MINI_SELECT)
            {
                m_number++; // Avoid the powerup being removed when validating the mini-choice
                    
                int cycle_ticks = stk_config->time2Ticks(0.65f);
                int cycle_value = World::getWorld()->getTicksSinceStart() % (3 * cycle_ticks);
                if (cycle_value < cycle_ticks)
                    m_mini_state = PowerupManager::MINI_ZIPPER;
                else if (cycle_value < 2*cycle_ticks)
                    m_mini_state = PowerupManager::MINI_CAKE;
                else
                    m_mini_state = PowerupManager::MINI_GUM;
            }
            // Use the selected mini-powerup
            else
            {
                if (m_mini_state == PowerupManager::MINI_CAKE)
                {
                    // make weapon usage destroy gum shields
                    if(stk_config->m_shield_restrict_weapons && m_kart->isGumShielded())
                        m_kart->decreaseShieldTime();
                    ProjectileManager::get()->newProjectile(m_kart, PowerupManager::POWERUP_MINI);
                } // mini-cake case
                else if (m_mini_state == PowerupManager::MINI_ZIPPER)
                {
                    m_kart->handleZipper(NULL, /* play sound*/ true, /* mini zipper */ true);
                } // mini-zipper case
                else if (m_mini_state == PowerupManager::MINI_GUM)
                {
                    sound_type = useBubblegum(/* mini */ true);
                } // mini-gum case

                // This allows to use multiple mini-wishes in different ways
                m_mini_state = PowerupManager::MINI_SELECT;
            }
            break;
        } // end of PowerupManager::POWERUP_MINI
    case PowerupManager::POWERUP_BUBBLEGUM:
        {
            sound_type = useBubblegum();
            break; 
        }
    case PowerupManager::POWERUP_ANVIL:
        {
            //Attach an anvil(twice as good as the one given
            //by the bananas) to the kart in the 1st position.
            for(unsigned int i = 0 ; i < world->getNumKarts(); ++i)
            {
                Kart *kart=world->getKart(i);
                if(kart->isEliminated() || kart->isInvulnerable()) continue;
                if(kart == m_kart) continue;
                if(kart->getPosition() == 1)
                {
                    kart->getAttachment()->set(Attachment::ATTACH_ANVIL,
                                               stk_config->
                                               time2Ticks(kp->getAnvilDuration()) );
                    kart->adjustSpeed(kp->getAnvilSpeedFactor() * 0.5f);
                    break;
                }
            }
            break;
        }
    case PowerupManager::POWERUP_PARACHUTE:
        {

            //Attach a parachute(that last 1,3 time as long as the
            //one from the bananas and is affected by the rank multiplier)
            //to all the karts that are in front of this one.
            for(unsigned int i = 0 ; i < world->getNumKarts(); ++i)
            {
                Kart *kart=world->getKart(i);
                if(kart->isEliminated() || kart== m_kart || kart->isInvulnerable()) continue;
                if(m_kart->getPosition() > kart->getPosition())
                {
                    if(kart->isShielded())
                    {
                        kart->decreaseShieldTime();
                        continue;
                    }
                    float rank_mult, position_factor;
                    //0 if the one before the item user ; 1 if first ; scaled inbetween
                    if (kart->getPosition() == 1)
                    {
                        position_factor = 1.0f;
                    }
                    else //m_kart position is always >= 3
                    {
                        float rank_factor;

                       rank_factor = (float)(kart->getPosition()   - 1) 
                                    / (float)(m_kart->getPosition() - 2);
                        position_factor = 1.0f - rank_factor;
                    }

                    rank_mult = 1 + (position_factor * 
                                     (kp->getParachuteDurationRankMult() - 1));

                    kart->getAttachment()
                        ->set(Attachment::ATTACH_PARACHUTE,
                              stk_config->time2Ticks(kp->getParachuteDurationOther()*rank_mult) );

                    if(kart->getController()->isLocalPlayerController())
                        player_kart = kart; // This is used to play the parachute sound
                }
            }
            break;
        }
    case PowerupManager::POWERUP_NOTHING:
    default : break;
    }

    PowerupAudio::getInstance()->onUseAudio(m_kart, m_type, sound_type, player_kart, audio_mini_state);

    if ( m_number <= 0 )
    {
        m_number = 0;
        m_type   = PowerupManager::POWERUP_NOTHING;
    }
}   // use

//-----------------------------------------------------------------------------
/** This function handles the bubblegum logic, in backward use (dropping a ground
 * gum) and forward use (creating a gum shield).
 * Its return value indicates the sound to play: none (0), ground gum (1), shield (2) */
int Powerup::useBubblegum(bool mini)
{
    int sound_type = 0;
    ItemManager* im = Track::getCurrentTrack()->getItemManager();
    const KartProperties *kp = m_kart->getKartProperties();

    // use the bubble gum the traditional way, if the kart is looking back
    if (m_kart->getControls().getLookBack())
    {
        Item *new_item = im->dropNewItem(mini ? Item::ITEM_BUBBLEGUM_SMALL :
                                                Item::ITEM_BUBBLEGUM, m_kart);
        // There may not be a new item if e.g. ground is not found in raycast.
        if(new_item)
            sound_type = 1;
    }
    else // if the kart is looking forward, use the bubblegum as a shield
    {
        Attachment::AttachmentType type;

        float mini_factor = (mini) ? 0.5f : 1.0f;

        // If the kart has a normal gum shield, don't change the shield type 
        if (mini && (!m_kart->isGumShielded() || m_kart->isWeakShielded() ))
        {
            if (m_kart->getIdent() == "nolok")
                type = Attachment::ATTACH_NOLOK_BUBBLEGUM_SHIELD_SMALL;
            else
                type = Attachment::ATTACH_BUBBLEGUM_SHIELD_SMALL; 
        }
        else
        {
            if (m_kart->getIdent() == "nolok")
                type = Attachment::ATTACH_NOLOK_BUBBLEGUM_SHIELD;
            else
                type = Attachment::ATTACH_BUBBLEGUM_SHIELD;
        }

        if(!m_kart->isGumShielded()) //if the previous shield had been used up.
        {
                m_kart->getAttachment()->set(type,
                    stk_config->time2Ticks(kp->getBubblegumShieldDuration() * mini_factor));
        }
        // Using a bubble gum while still having a gum shield. In this case,
        // half of the remaining time of the active shield is added. The maximum
        // duration of a shield is never above twice the standard duration.
        // The code above guarantees that, if there is a mix and match
        // of small and normal gum shield between the active shield and
        // the used shield, the new shield will be normal.
        // As long as the mini_factor is >= 0.5, using a mini-gum shield
        // can never reduce the duration of an active gum shield.
        else
        {
            m_kart->getAttachment()->set(type,
                stk_config-> time2Ticks(kp->getBubblegumShieldDuration() * mini_factor
                                       + (m_kart->getShieldTime() / 2.0f)) );
        }
        sound_type = 2;
    }
    return sound_type;
}   // useBubblegum

//-----------------------------------------------------------------------------
/** This function handles the nitro-hack logic.
 * Its return value indicates the sound to play: good sound (1), bad sound (2)
 * or both (3) */
int Powerup::useNitroHack()
{
    unsigned int steal_targets = powerup_manager->getNitroHackMaxTargets();
    float base_bonus = powerup_manager->getNitroHackBaseBonus();
    float negative_multiply = powerup_manager->getNitroHackNegativeMultiply();

    float stolen_energy = 0.0f;
    unsigned int steal_counter = 0;
    int sound_type = (m_kart->getController()->isLocalPlayerController()) ? 1 : 0;
    World *world = World::getWorld();

    // Steal some nitro from up to steal_targets karts ahead.
    // This can set their nitro count to a negative number
    for(unsigned int i = 0 ; i < world->getNumKarts(); ++i)
    {
        Kart *kart = world->getKart(i);
        // Standard invulnerability (the "stars") is not useful here
        if( kart->isEliminated() || kart == m_kart || kart->hasFinishedRace())
            continue;

        int position_diff = m_kart->getPosition() - kart->getPosition();
        if(position_diff > 0)
        {
            float amount_to_steal = powerup_manager->getNitroHackStolenDiff(position_diff);
            if (amount_to_steal > 0.0f)
            {
                // Remove nitro from a target kart and add to the recipient
                kart->addEnergy(-amount_to_steal, /* allow negatives */ true);
                stolen_energy += amount_to_steal;
                steal_counter++;

                // This is used for display in the race GUI
                kart->setStolenNitro(amount_to_steal, /* duration */ 1.75f);

                // If a local player is affected, we will play a negative sound effect
                if(kart->getController()->isLocalPlayerController() && sound_type <= 1)
                    sound_type += 2;
            }
        }
    }

    // Multiply current nitro by a given factor, if it is currently negative
    // With default values, this does nothing.
    if(m_kart->getEnergy() < 0)
        m_kart->setEnergy(m_kart->getEnergy()*negative_multiply);

    // Gift some free nitro if there is not enough targets in front
    if (steal_counter < steal_targets)
        stolen_energy += (steal_targets - steal_counter) * base_bonus;

    // Give the stolen nitro and activate the "nitro boost" mode
    // TODO make the gift of nitro depend on kart class/ nitro efficiency ??
    m_kart->addEnergy(stolen_energy, /* allow negatives */ false);
    m_kart->activateNitroHack();

    return sound_type;
} // useNitroHack

//-----------------------------------------------------------------------------
/** This function is called when a bonus box is it. This function can be
 *  called on a server (in which case item and add_info are not used),
 *  or on a client, in which case the item and additional info is used
 *  to make sure server and clients are synched correctly.
 *  \param n
 *  \param item_state The item_state (bonux box) that was hit. This is
 *         necessary for servers so that the clients can be informed which
 *         item was collected.
 */
void Powerup::hitBonusBox(const ItemState &item_state)
{
    // Position can be -1 in case of a battle mode (which doesn't have
    // positions), but this case is properly handled in getRandomPowerup.
    int position = m_kart->getPosition();

    unsigned int n=1;
    PowerupManager::PowerupType new_powerup;
    World *world = World::getWorld();

    // Determine a 'random' number based on time, index of the item,
    // and position of the kart. The idea is that this process is
    // randomly enough to get the right distribution of the powerups,
    // does not involve additional network communication to keep 
    // client and server in sync, and is not exploitable:
    // While it is not possible for a client to determine the item
    // (the server will always finally determine which item a player
    // receives), we need to make sure that people cannot modify the
    // sources and display the item that will be collected next
    // at a box - otherwise the player could chose the 'best' box.
    // Using synchronised pseudo-random-generators would not prevent
    // cheating, since the a cheater could determine the next random
    // number that will be used. If we use the server to always
    // send the information to the clients, we need to add a delay
    // before items can be used.
    // So instead we determine a random number that is based on:
    // (1) The item id
    // (2) The time
    // (3) The position of the kart
    // (4) An extra random 64bit integer
    // Using (1) means that not all boxes at a certain time for a kart
    // will give the same box. Using (2) means that the item will
    // change over time - even if the next item is displayed, it 
    // will mean a cheater has to wait, and because of the frequency
    // of the time component it will also be difficult to get the
    // item at the right time. Using (3) adds another cheat-prevention
    // layer: even if a cheater is waiting for the right sequence
    // of items, if he is overtaken the sequence will change, using (4)
    // to avoid same item sequence when starting
    //

    // In order to increase the probability of correct client prediction
    // in networking (where there might be 1 or 2 frames difference
    // between client and server when collecting an item), the time
    // is divided by 10, meaning even if there is one frame difference,
    // the client will still have a 90% chance to correctly predict the
    // item.
    const int time = world->getTicksSinceStart() / stk_config->time2Ticks(0.083334f);

    // A bitmask storing which random buckets have been used
    uint32_t buckets_mask = m_kart->getPowerupMask();
    int random_number = 0;

    // Pick a random number
    // If not in full random mode, check that's not in one of the already used buckets
    for (unsigned int i=0; i<30;i++)
    {
        // Random_number is in the range 0-32767 
        random_number = simplePRNG((int)(powerup_manager->getRandomSeed()+i),
                                    time+1000*i, item_state.getItemId(), position);

        // Make sure the random number is equally likely to be in any
        // of the buckets
        if (random_number > (32767 - (32768 % BUCKET_COUNT)))
            continue;

        // We don't check for buckets if full random mode is enabled in config
        // This steps occurs after the previous one to ensure that
        // the random number is equally likely to be in any part of
        // the weights list
        if (stk_config->m_full_random)
            break; 

        // Determine the random number's bucket
        int bucket = random_number / (32768 / BUCKET_COUNT);

        uint32_t temp_mask = 1 << bucket;
        // Check that the new random number is not in an already used bucket
        if ((buckets_mask & temp_mask) == 0)
        {
            // We update the buckets_mask
            // This function also handles removing previously used buckets
            // before we have cycled over all possible buckets
            m_kart->updatePowerupMask(bucket);
            break;
        }
    }

    new_powerup = powerup_manager->getRandomPowerup(position, &n, 
                                                    random_number);

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

//-----------------------------------------------------------------------------
/** This function is called when a pseudo-random number for a new item
 *  is needed.
 *  Usual PRNG update their state when generating a number. For synchronization
 *  purposes, this would create a difficulty. If the collection of an item
 *  box is mispredicted, the PRNG could go out of synch. And if messages had
 *  to be sent to resynch the PRNG, then it would be more efficient to directly
 *  send appropriate final pseudo-random number for the items.
 *  The state of the simplePRNG is defined by the seed it is called with
 *  (determined at race start and constant),
 *  and some race parameters :
 *  - the current time of the race
 *  - the collected item ID
 *  - the kart's position
 *  The simplePRNG takes advantage of the fact that this state will have more
 *  significant bits that the needed output to extract higher-order bits and
 *  get enough impredictability - and regularity - for our purpose.
 *  The simplePRNG is modulo 2^31 : it doesn't need to do modulo calculations,
 *  the arithmetic of 32-bit ints take care of this automatically.
 *  \param seed The base number.
 *  \param item_id The item id
 *  \param position The kart position
 */
int Powerup::simplePRNG(const int seed, const int time, const int item_id, const int position)
{
    const int c = 12345*(1+2*time); // This is always an odd number

    const int a = 1103515245;
    int rand = a*(seed + c);
    if (rand < 0)
    {
        // We substract 2^31, which sends back the number to the positives
        // while keeping the same value modulo 2^31
        rand += -2147483648;
    }
    if (position > 0 && position > item_id)
        return simplePRNG(rand, time, item_id, position-1);
    else if (item_id > 0)
        return simplePRNG(-2147483648 - rand, time, item_id-1, 0);

#ifdef ITEM_DISTRIBUTION_DEBUG
    Log::verbose("Powerup", "Raw PRN is %d", rand);
#endif

    // Return the final value and drop the lower order bits
    return (rand/65536);
} // simplePRNG
