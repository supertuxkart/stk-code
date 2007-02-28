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

#include "collectable.hpp"
#include "user_config.hpp"
#include "projectile_manager.hpp"
#include "kart.hpp"
#include "sound_manager.hpp"
#include "world.hpp"
#include "stk_config.hpp"

//-----------------------------------------------------------------------------
Collectable::Collectable(Kart* kart_)
{
    owner  = kart_;
    type   = COLLECT_NOTHING;
    number = 0;
}   // Collectable

//-----------------------------------------------------------------------------
void Collectable::set(collectableType _type, int n)
{
    if (type==_type)
    {
        number+=n;
        return;
    }
    type=_type;
    number=n;
}  // set

//-----------------------------------------------------------------------------
Material *Collectable::getIcon()
{
    // Check if it's one of the types which have a separate
    // data file which includes the icon:
    return collectable_manager->getIcon(type);
}

//-----------------------------------------------------------------------------
void Collectable::use()
{
#ifdef USE_MAGNET
    if(user_config->disableMagnet)
    {
        attachmentType at=owner->getAttachment();
        if(at==ATTACH_MAGNET)
        {
            owner->setAttachmentType(ATTACH_MAGNET_BZZT);
        }
        else if(at==ATTACH_MAGNET_BZZT)
        {
            owner->setAttachmentType(ATTACH_MAGNET     );
        }   // if MAGNET_BZZT
    }  // user_config->disableMagnet
#endif
    number--;
    switch (type)
    {
#ifdef USE_MAGNET
    case COLLECT_MAGNET:   owner->attach(ATTACH_MAGNET_BZZT, stk_config->m_magnet_time);
        break ;
#endif
    case COLLECT_ZIPPER:   owner->handleZipper();
        break ;
    case COLLECT_HOMING_MISSILE:
    case COLLECT_SPARK:
    case COLLECT_MISSILE:
        if(owner->isPlayerKart())
            sound_manager->playSfx(SOUND_SHOT);

        projectile_manager->newProjectile(owner, type);
        break ;

    case COLLECT_ANVIL:
        //Attach an anvil(twice as good as the one given
        //by the bananas) to the kart in the 1st position.
        for(int i = 0 ; i < world->getNumKarts(); ++i)
        {
            if(world->getKart(i) == owner) continue;
            if(world->getKart(i)->getPosition() == 1)
            {
                world->getKart(i)->
                attach(ATTACH_ANVIL, stk_config->m_anvil_time);
                
                world->getKart(i)->adjustSpeedWeight(stk_config->m_anvil_speed_factor*0.5f);

                if(world->getKart(i)->isPlayerKart())
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
            for(int i = 0 ; i < world->getNumKarts(); ++i)
            {
                if(world->getKart(i) == owner) continue;
                if(owner->getPosition() > world->
                   getKart(i)->getPosition())
                {
                    world->getKart(i)->attach(
                        ATTACH_PARACHUTE, stk_config->m_parachute_time_other);

                    if(world->getKart(i)->isPlayerKart())
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

    if ( number <= 0 )
    {
        clear();
    }
}   // use

//-----------------------------------------------------------------------------
void Collectable::hitRedHerring(int n)
{
    //The probabilities of getting the anvil or the parachute increase
    //depending on how bad the owner's position is. For the first
    //driver the posibility is none, for the last player is 15 %.

    if(owner->getPosition() != 1 && type == COLLECT_NOTHING)
    {
        const int SPECIAL_PROB = (int)(15.0 / ((float)world->getNumKarts() /
                                         (float)owner->getPosition()));
        const int RAND_NUM = rand()%100;
        if(RAND_NUM <= SPECIAL_PROB)
        {
            //If the driver in the first position has finished, give the driver
            //the parachute.
            for(int i=0; i < world->getNumKarts(); ++i)
            {
                if(world->getKart(i) == owner) continue;
                if(world->getKart(i)->getPosition() == 1 && world->getKart(i)->
                   raceIsFinished())
                {
                    type = COLLECT_PARACHUTE;
                    number = 1;
                    return;
                }
            }

            type = rand()%(2) == 0 ? COLLECT_ANVIL : COLLECT_PARACHUTE;
            number = 1;
            return;
        }
    }

    //rand() is moduled by COLLECT_MAX - 1 - 2 because because we have to
    //exclude the anvil and the parachute, but later we have to add 1 to prevent
    //having a value of 0 since that isn't a valid collectable.
    collectableType newC;
    if(!user_config->m_profile)
    {
        newC = (collectableType)(rand()%(COLLECT_MAX - 1 - 2) + 1);
    }
    else
    {
        // No random effects when profiling!
        static int simpleCounter=-1;
        simpleCounter++;
        newC = (collectableType)(simpleCounter%(COLLECT_MAX - 1 - 2) + 1);
    }
    if(type==COLLECT_NOTHING)
    {
        type=newC;
        number = n;
    }
    else if(newC==type)
    {
        number+=n;
        if(number > MAX_COLLECTABLES) number = MAX_COLLECTABLES;
    }
    // Ignore new collectable if it is different from the current one
}   // hitRedHerring
