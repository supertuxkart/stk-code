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

#include "items/powerup.hpp"

#include "network/network_manager.hpp"
#include "network/race_state.hpp"
#include "user_config.hpp"
#include "race_manager.hpp"
#include "items/projectile_manager.hpp"
#include "kart.hpp"
#include "audio/sfx_manager.hpp"
#include "audio/sfx_base.hpp"
#include "modes/world.hpp"
#include "stk_config.hpp"

//-----------------------------------------------------------------------------
Powerup::Powerup(Kart* kart_)
{
    m_owner               = kart_;
    m_sound_shot          = sfx_manager->newSFX(SFXManager::SOUND_SHOT);
    m_sound_use_anvil     = sfx_manager->newSFX(SFXManager::SOUND_USE_ANVIL);
    m_sound_use_parachute = sfx_manager->newSFX(SFXManager::SOUND_USE_PARACHUTE);
    reset();
}   // Powerup

//-----------------------------------------------------------------------------
/** Frees the memory for the sound effects.
 */
Powerup::~Powerup()
{
    sfx_manager->deleteSFX(m_sound_shot);
    sfx_manager->deleteSFX(m_sound_use_anvil);
    sfx_manager->deleteSFX(m_sound_use_parachute);

}   // ~Powerup

//-----------------------------------------------------------------------------
void Powerup::reset()
{
    int type;
    RaceManager::getWorld()->getDefaultCollectibles( type, m_number );
    m_type = (PowerupType)type;
}   // reset

//-----------------------------------------------------------------------------
void Powerup::set(PowerupType type, int n)
{
    if (m_type==type)
    {
        m_number+=n;
        return;
    }
    m_type=type;
    m_number=n;
}  // set

//-----------------------------------------------------------------------------
Material *Powerup::getIcon()
{
    // Check if it's one of the types which have a separate
    // data file which includes the icon:
    return powerup_manager->getIcon(m_type);
}

//-----------------------------------------------------------------------------
void Powerup::use()
{
    m_number--;
    switch (m_type)
    {
    case POWERUP_ZIPPER:   m_owner->handleZipper();
        break ;
    case POWERUP_CAKE:
    case POWERUP_BOWLING:
    case POWERUP_MISSILE:
        m_sound_shot->position(m_owner->getXYZ());
        m_sound_shot->play();
        projectile_manager->newProjectile(m_owner, m_type);
        break ;

    case POWERUP_ANVIL:
        //Attach an anvil(twice as good as the one given
        //by the bananas) to the kart in the 1st position.
        for(unsigned int i = 0 ; i < race_manager->getNumKarts(); ++i)
        {
            Kart *kart=RaceManager::getKart(i);
            if(kart->isEliminated()) continue;
            if(kart == m_owner) continue;
            if(kart->getPosition() == 1)
            {
                kart->attach(ATTACH_ANVIL, stk_config->m_anvil_time);
                kart->adjustSpeedWeight(stk_config->m_anvil_speed_factor*0.5f);
                m_sound_use_anvil->position(m_owner->getXYZ());
                m_sound_use_anvil->play();
                break;
            }
        }

        break;

    case POWERUP_PARACHUTE:
        {
            Kart* player_kart = NULL;
            //Attach a parachutte(that last as twice as the
            //one from the bananas) to all the karts that
            //are in front of this one.
            for(unsigned int i = 0 ; i < race_manager->getNumKarts(); ++i)
            {
                Kart *kart=RaceManager::getKart(i);
                if(kart->isEliminated() || kart== m_owner) continue;
                if(m_owner->getPosition() > kart->getPosition())
                {
                    kart->attach(ATTACH_PARACHUTE, stk_config->m_parachute_time_other);

                    if(kart->isPlayerKart())
                        player_kart = kart;
                }
            }

            if(player_kart)
            {
                m_sound_use_parachute->position(player_kart->getXYZ());
                m_sound_use_parachute->play();
            }
        }
        break;

    case POWERUP_NOTHING:
    default :              break ;
    }

    if ( m_number <= 0 )
    {
        m_number = 0;
        m_type   = POWERUP_NOTHING;
    }
}   // use

//-----------------------------------------------------------------------------
void Powerup::hitBonusBox(int n, const Item &item, int add_info)
{
    //The probabilities of getting the anvil or the parachute increase
    //depending on how bad the owner's position is. For the first
    //driver the posibility is none, for the last player is 15 %.
    if(m_owner->getPosition() != 1 && m_type == POWERUP_NOTHING)
    {
        // On client: just set the value
        if(network_manager->getMode()==NetworkManager::NW_CLIENT)
        {
            m_random.get(100);    // keep random numbers in sync
            m_type    = (PowerupType)add_info;
            m_number  = 1;
            return;
        }
        const int SPECIAL_PROB = (int)(15.0 / ((float)RaceManager::getWorld()->getCurrentNumKarts() /
                                         (float)m_owner->getPosition()));
        const int RAND_NUM = m_random.get(100);
        if(RAND_NUM <= SPECIAL_PROB)
        {
            //If the driver in the first position has finished, give the driver
            //the parachute.
            for(unsigned int i=0; i < race_manager->getNumKarts(); ++i)
            {
                Kart *kart = RaceManager::getKart(i);
                if(kart->isEliminated() || kart == m_owner) continue;
                if(kart->getPosition() == 1 && kart->hasFinishedRace())
                {
                    m_type = POWERUP_PARACHUTE;
                    m_number = 1;
                    if(network_manager->getMode()==NetworkManager::NW_SERVER)
                    {
                        race_state->itemCollected(m_owner->getWorldKartId(), 
                                                  item.getItemId(), 
                                                  m_type);
                    }
                    return;
                }
            }

            m_type   = m_random.get(2) == 0 ? POWERUP_ANVIL : POWERUP_PARACHUTE;
            m_number = 1;
            if(network_manager->getMode()==NetworkManager::NW_SERVER)
            {
                race_state->itemCollected(m_owner->getWorldKartId(), 
                                          item.getItemId(), 
                                          (char)m_type);
            }
            return;
        }
    }


    // If no special case is done: on the client just adjust the number
    // dependent on the server informaion:
    if(network_manager->getMode()==NetworkManager::NW_CLIENT)
    {
        if(m_type==POWERUP_NOTHING)
        {
            m_type   = (PowerupType)add_info;
            m_number = n;
        }
        else if((PowerupType)add_info==m_type)
        {
            m_number+=n;
            if(m_number > MAX_POWERUPS) m_number = MAX_POWERUPS;
        }
        // Ignore new powerup if it is different from the current one
        m_random.get(100);    // keep random numbers in synch

        return;
    }   // if network client

    // Otherwise (server or no network): determine powerup randomly

    //rand() is moduled by POWERUP_MAX - 1 - 2 because because we have to
    //exclude the anvil and the parachute, but later we have to add 1 to prevent
    //having a value of 0 since that isn't a valid powerup.
    PowerupType newC;
    newC = (PowerupType)(m_random.get(POWERUP_MAX - 1 - 2) + 1);
    // Save the information about the powerup in the race state
    // so that the clients can be updated.
    if(network_manager->getMode()==NetworkManager::NW_SERVER)
    {
        race_state->itemCollected(m_owner->getWorldKartId(), 
                                  item.getItemId(), 
                                  newC);
    }

    if(m_type==POWERUP_NOTHING)
    {
        m_type=newC;
        m_number = n;
    }
    else if(newC==m_type)
    {
        m_number+=n;
        if(m_number > MAX_POWERUPS) m_number = MAX_POWERUPS;
    }
    // Ignore new powerup if it is different from the current one
}   // hitBonusBox
