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

#include "network/network_manager.hpp"
#include "network/race_state.hpp"
#include "collectable.hpp"
#include "user_config.hpp"
#include "race_manager.hpp"
#include "projectile_manager.hpp"
#include "kart.hpp"
#include "sound_manager.hpp"
#include "world.hpp"
#include "stk_config.hpp"

//-----------------------------------------------------------------------------
Collectable::Collectable(Kart* kart_)
{
    m_owner  = kart_;
    reset();
}   // Collectable

//-----------------------------------------------------------------------------
void Collectable::reset()
{
    if(race_manager->getMinorMode()==RaceManager::RM_TIME_TRIAL)
    {
        m_type   = COLLECT_ZIPPER;
        m_number = race_manager->getNumLaps();
    }
    else
    {
        m_type   = COLLECT_NOTHING;
        m_number = 0;
    }
}   // reset

//-----------------------------------------------------------------------------
void Collectable::set(CollectableType type, int n)
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
Material *Collectable::getIcon()
{
    // Check if it's one of the types which have a separate
    // data file which includes the icon:
    return collectable_manager->getIcon(m_type);
}

//-----------------------------------------------------------------------------
void Collectable::use()
{
    m_number--;
    switch (m_type)
    {
    case COLLECT_ZIPPER:   m_owner->handleZipper();
        break ;
    case COLLECT_HOMING:
    case COLLECT_BOWLING:
    case COLLECT_MISSILE:
        if(m_owner->isPlayerKart())
            sound_manager->playSfx(SOUND_SHOT);
        projectile_manager->newProjectile(m_owner, m_type);
        break ;

    case COLLECT_ANVIL:
        //Attach an anvil(twice as good as the one given
        //by the bananas) to the kart in the 1st position.
        for(unsigned int i = 0 ; i < race_manager->getNumKarts(); ++i)
        {
            Kart *kart=world->getKart(i);
            if(kart->isEliminated()) continue;
            if(kart == m_owner) continue;
            if(kart->getPosition() == 1)
            {
                kart->attach(ATTACH_ANVIL, stk_config->m_anvil_time);
                kart->adjustSpeedWeight(stk_config->m_anvil_speed_factor*0.5f);

                if(kart->isPlayerKart())
                    sound_manager->playSfx(SOUND_USE_ANVIL);
                break;
            }
        }

        break;

    case COLLECT_PARACHUTE:
        {
            bool player_affected = false;
            //Attach a parachutte(that last as twice as the
            //one from the bananas) to all the karts that
            //are in front of this one.
            for(unsigned int i = 0 ; i < race_manager->getNumKarts(); ++i)
            {
                Kart *kart=world->getKart(i);
                if(kart->isEliminated() || kart== m_owner) continue;
                if(m_owner->getPosition() > kart->getPosition())
                {
                    kart->attach(ATTACH_PARACHUTE, stk_config->m_parachute_time_other);

                    if(kart->isPlayerKart())
                        player_affected = true;
                }

            }

            if(player_affected)
                sound_manager->playSfx(SOUND_USE_PARACHUTE);
        }
        break;

    case COLLECT_NOTHING:
    default :              break ;
    }

    if ( m_number <= 0 )
    {
        m_number = 0;
        m_type   = COLLECT_NOTHING;
    }
}   // use

//-----------------------------------------------------------------------------
void Collectable::hitRedHerring(int n, const Herring &herring, int add_info)
{
    //The probabilities of getting the anvil or the parachute increase
    //depending on how bad the owner's position is. For the first
    //driver the posibility is none, for the last player is 15 %.
    if(m_owner->getPosition() != 1 && m_type == COLLECT_NOTHING)
    {
        // On client: just set the value
        if(network_manager->getMode()==NetworkManager::NW_CLIENT)
        {
            int dummy = m_random.get(100);    // keep random numbers in sync
            m_type    = (CollectableType)add_info;
            m_number  = 1;
            return;
        }
        const int SPECIAL_PROB = (int)(15.0 / ((float)world->getCurrentNumKarts() /
                                         (float)m_owner->getPosition()));
        const int RAND_NUM = m_random.get(100);
        if(RAND_NUM <= SPECIAL_PROB)
        {
            //If the driver in the first position has finished, give the driver
            //the parachute.
            for(unsigned int i=0; i < race_manager->getNumKarts(); ++i)
            {
                Kart *kart = world->getKart(i);
                if(kart->isEliminated() || kart == m_owner) continue;
                if(kart->getPosition() == 1 && kart->hasFinishedRace())
                {
                    m_type = COLLECT_PARACHUTE;
                    m_number = 1;
                    if(network_manager->getMode()==NetworkManager::NW_SERVER)
                    {
                        race_state->herringCollected(m_owner->getWorldKartId(), 
                                                     herring.getHerringId(), 
                                                     m_type);
                    }
                    return;
                }
            }

            m_type   = m_random.get(2) == 0 ? COLLECT_ANVIL : COLLECT_PARACHUTE;
            m_number = 1;
            if(network_manager->getMode()==NetworkManager::NW_SERVER)
            {
                race_state->herringCollected(m_owner->getWorldKartId(), 
                                             herring.getHerringId(), 
                                             (char)m_type);
            }
            return;
        }
    }


    // If no special case is done: on the client just adjust the number
    // dependent on the server informaion:
    if(network_manager->getMode()==NetworkManager::NW_CLIENT)
    {
        if(m_type==COLLECT_NOTHING)
        {
            m_type   = (CollectableType)add_info;
            m_number = n;
        }
        else if((CollectableType)add_info==m_type)
        {
            m_number+=n;
            if(m_number > MAX_COLLECTABLES) m_number = MAX_COLLECTABLES;
        }
        // Ignore new collectable if it is different from the current one
        int dummy = m_random.get(100);    // keep random numbers in synch

        return;
    }   // if network client

    // Otherwise (server or no network): determine collectable randomly

    //rand() is moduled by COLLECT_MAX - 1 - 2 because because we have to
    //exclude the anvil and the parachute, but later we have to add 1 to prevent
    //having a value of 0 since that isn't a valid collectable.
    CollectableType newC;
    newC = (CollectableType)(m_random.get(COLLECT_MAX - 1 - 2) + 1);
    // Save the information about the collectable in the race state
    // so that the clients can be updated.
    if(network_manager->getMode()==NetworkManager::NW_SERVER)
    {
        race_state->herringCollected(m_owner->getWorldKartId(), 
                                     herring.getHerringId(), 
                                     newC);
    }

    if(m_type==COLLECT_NOTHING)
    {
        m_type=newC;
        m_number = n;
    }
    else if(newC==m_type)
    {
        m_number+=n;
        if(m_number > MAX_COLLECTABLES) m_number = MAX_COLLECTABLES;
    }
    // Ignore new collectable if it is different from the current one
}   // hitRedHerring
