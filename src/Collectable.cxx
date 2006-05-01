//  $Id: Collectable.cxx,v 1.6 2005/08/23 20:00:57 joh Exp $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
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

#include "Collectable.h"
#include "Config.h"
#include "ProjectileManager.h"
#include "Kart.h"

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
  if(config->disableMagnet) {
    attachmentType at=owner->getAttachment();
    if(at==ATTACH_MAGNET) {
      owner->setAttachmentType(ATTACH_MAGNET_BZZT);
    } else if(at==ATTACH_MAGNET_BZZT) {
      owner->setAttachmentType(ATTACH_MAGNET     );
    }   // if MAGNET_BZZT
  }  // config->disableMagnet
  number--;
  switch (type) {
    case COLLECT_MAGNET:   owner->attach(ATTACH_MAGNET_BZZT, 10.0f);
                           break ;
    case COLLECT_ZIPPER:   owner->handleZipper();
			   break ;
    case COLLECT_HOMING_MISSILE:
    case COLLECT_SPARK:
    case COLLECT_MISSILE:  projectile_manager->newProjectile(owner, type);
                           break ;

    case COLLECT_NOTHING:
    default :              break ;
  }

  if ( number <= 0 ) {
    clear();
  }
}   // use

// -----------------------------------------------------------------------------
void Collectable::hitRedHerring(int n) {
  collectableType newC=(collectableType)(rand()%5+1);
  if(type==COLLECT_NOTHING) {
    type=newC;
    number=n;
  } else if(newC==type) {
    number+=n;
    if(number > MAX_COLLECTABLES) number = MAX_COLLECTABLES;
  }
  // Ignore new collectable if it is different from the current one
}   // hitRedHerring
