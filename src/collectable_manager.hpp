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

#ifndef HEADER_COLLECTABLEMANAGER_H
#define HEADER_COLLECTABLEMANAGER_H

#include "lisp/parser.hpp"
#include "lisp/lisp.hpp"

class Material;
class ssgEntity;

//The anvil and parachute must be at the end of the enum.
enum collectableType {COLLECT_NOTHING, COLLECT_MISSILE,
                      COLLECT_SPARK, COLLECT_HOMING_MISSILE,
                      COLLECT_ZIPPER,
                      COLLECT_ANVIL, COLLECT_PARACHUTE,
#ifdef USE_MAGNETS
                      COLLECT_MAGNET,
#endif
                      COLLECT_MAX};

class CollectableManager
{
protected:
    Material*    m_all_icons [COLLECT_MAX];
    float        m_all_speeds[COLLECT_MAX];
    ssgEntity*   m_all_models[COLLECT_MAX];
    void         LoadNode       (const lisp::Lisp* lisp, int collectType);
public:
    CollectableManager           ();
    void         loadCollectables();
    void         removeTextures  ();
    void         Load            (int collectType, const char* filename);
    Material*    getIcon         (int type) {return m_all_icons [type];}
    float        getSpeed        (int type) {return m_all_speeds[type];}
    ssgEntity*   getModel        (int type) {return m_all_models[type];}
};

extern CollectableManager* collectable_manager;

#endif
