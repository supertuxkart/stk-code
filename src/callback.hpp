//  $Id: callback.hpp 796 2006-09-27 07:06:34Z hiker $
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

#ifndef CALLBACK_H
#define CALLBACK_H

#include "btBulletDynamicsCommon.h"

class Callback
{
public:
    virtual     ~Callback()     {};
    virtual void update         (float dt) = 0;
    virtual void init           ()         = 0;
    virtual void reset          ()           {};
    virtual void handleExplosion(const btVector3& pos, bool directHit) {};
}
;   // Callback

#endif

