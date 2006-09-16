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
#include "config.hpp"
#include "projectile_manager.hpp"
#include "kart.hpp"
#include "sound.hpp"
#include "world.cpp"

// -----------------------------------------------------------------------------
Collectable::Collectable(Kart* kart_) {
  owner  = kart_;
  type   = COLLECT_NOTHING;
  number = 0;
}   // Collectable

// -----------------------------------------------------------------------------
void Collectable::set(collectableType _type, int n) {
  if (type==_type) {
    number+=n;
    return;
  }
  type=_type;
  number=n;
}  // set

// -----------------------------------------------------------------------------
Material *Collectable::getIcon() {
  // Check if it's one of the types which have a separate
  // data file which includes the icon:
  return collectable_manager->getIcon(type);
}

// -----------------------------------------------------------------------------
void Collectable::use() {
#ifdef USE_MAGNET
  if(config->disableMagnet) {
    attachmentType at=owner->getAttachment();
    if(at==ATTACH_MAGNET) {
      owner->setAttachmentType(ATTACH_MAGNET_BZZT);
    } else if(at==ATTACH_MAGNET_BZZT) {
      owner->setAttachmentType(ATTACH_MAGNET     );
    }   // if MAGNET_BZZT
  }  // config->disableMagnet
#endif
  number--;
  switch (type) {
#ifdef USE_MAGNET
    case COLLECT_MAGNET:   owner->attach(ATTACH_MAGNET_BZZT, 10.0f);
                           break ;
#endif
    case COLLECT_ZIPPER:   owner->handleZipper();
			               break ;
    case COLLECT_HOMING_MISSILE:
    case COLLECT_SPARK:
    case COLLECT_MISSILE:
			               if(owner->isPlayerKart())
				               sound->playSfx(SOUND_SHOT);
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
                                       attach(ATTACH_ANVIL, 4.0f);
                                   world->getKart(i)->getVelocity()->xyz[1] *=
                                       physicsParameters->anvilSpeedFactor *
                                       0.5f;
                                   break;
                               }

                           }

                           break;

    case COLLECT_PARACHUTE:
                           //Attach a parachute to all the karts that are
                           //in front of this one.
                           for(int i = 0 ; i < world->getNumKarts(); ++i)
                           {
                               if(world->getKart(i) == owner) continue;
                               if(owner->getPosition() > world->getKart(i)->
                                  getPosition())
                               {
                                   world->getKart(i)->attach(ATTACH_PARACHUTE,
                                       8.0f);
                               }

                           }
                           break;

    case COLLECT_NOTHING:
    default :              break ;
  }

  if ( number <= 0 ) {
    clear();
  }
}   // use

// -----------------------------------------------------------------------------
void Collectable::hitRedHerring(int n) {
  //rand() is moduled by COLLECT_MAX - 1 because it's the number of
  //collectables, but since it might give a result of 0(and that position isn't
  //a valid collectable) we have to add 1.
  collectableType newC=(collectableType)(rand()%(COLLECT_MAX - 1) + 1);
  if(type==COLLECT_NOTHING) {
    type=newC;
    number=n;
  } else if(newC==type) {
    number+=n;
    if(number > MAX_COLLECTABLES) number = MAX_COLLECTABLES;
  }
  // Ignore new collectable if it is different from the current one
}   // hitRedHerring
