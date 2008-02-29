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

#include <iostream>
#include <stdexcept>
#include "collectable_manager.hpp"
#include "loader.hpp"
#include "material_manager.hpp"
#include "material.hpp"
#include "translation.hpp"
#include "spark.hpp"
#include "missile.hpp"
#include "homing.hpp"
#if defined(WIN32) && !defined(__CYGWIN__)
#  define snprintf _snprintf
#endif

typedef struct
{
    CollectableType collectable;
    const char*const dataFile;
}
initCollectableType;

initCollectableType ict[]=
{
    {COLLECT_ZIPPER,    "zipper.collectable"       },
    {COLLECT_SPARK,     "spark.projectile"         },
    {COLLECT_MISSILE,   "missile.projectile"       },
    {COLLECT_HOMING,    "homingmissile.projectile" },
    {COLLECT_ANVIL,     "anvil.collectable"        },
    {COLLECT_PARACHUTE, "parachute.collectable"    },
    {COLLECT_MAX,       ""                         },
};

CollectableManager* collectable_manager=0;

//-----------------------------------------------------------------------------
CollectableManager::CollectableManager()
{
    for(int i=0; i<COLLECT_MAX; i++)
    {
        m_all_models[i] = (ssgEntity*)NULL;
        m_all_icons[i]  = (Material*)NULL;
    }
}   // CollectableManager

//-----------------------------------------------------------------------------
void CollectableManager::removeTextures()
{
    for(int i=0; i<COLLECT_MAX; i++)
    {
        if(m_all_icons [i]) ssgDeRefDelete(m_all_icons [i]->getState());
        if(m_all_models[i]) ssgDeRefDelete(m_all_models[i]            );
    }   // for
    callback_manager->clear(CB_COLLECTABLE);

}   // removeTextures

//-----------------------------------------------------------------------------
void CollectableManager::loadCollectables()
{
    for(int i=0; ict[i].collectable != COLLECT_MAX; i++)
    {
        Load(ict[i].collectable, ict[i].dataFile);
    }
}  // loadCollectables

//-----------------------------------------------------------------------------
void CollectableManager::Load(int collectType, const char* filename)
{
    const lisp::Lisp* ROOT = 0;

    lisp::Parser parser;
    std::string tmp= "data/" + (std::string)filename;
    ROOT = parser.parse(loader->getConfigFile(filename));
        
    const lisp::Lisp* lisp = ROOT->getLisp("tuxkart-collectable");
    if(!lisp)
    {
        char msg[MAX_ERROR_MESSAGE_LENGTH];
        snprintf(msg, sizeof(msg), 
                 "No 'tuxkart-collectable' node found while parsing '%s'.",
                 filename);
        throw std::runtime_error(msg);
    }
    LoadNode(lisp, collectType);

    delete ROOT;

}   // Load

//-----------------------------------------------------------------------------
void CollectableManager::LoadNode(const lisp::Lisp* lisp, int collectType )
{
    std::string sName, sModel, sIconFile; 
    lisp->get("name",            sName                              );
    lisp->get("model",           sModel                             );
    lisp->get("icon",            sIconFile                          );
 
    // load material
    m_all_icons[collectType] = material_manager->getMaterial(sIconFile);
    m_all_icons[collectType]->getState()->ref();

    //FIXME: something probably forgets to disable GL_CULL_FACE after enabling it,
    //this is just a quick fix.
    if(collectType == COLLECT_SPARK) m_all_icons[COLLECT_SPARK]->getState()->disable ( GL_CULL_FACE ) ;

    if(sModel!="")
    {
        ssgEntity* e = loader->load(sModel, CB_COLLECTABLE);
        m_all_models[collectType] = e;
        e->ref();
        e->clrTraversalMaskBits(SSGTRAV_ISECT|SSGTRAV_HOT);
    }
    else
    {
        m_all_models[collectType] = 0;
        m_all_extends[collectType] = btVector3(0.0f,0.0f,0.0f);
    }

    // Load special attributes for certain collectables
    switch (collectType) {
        case COLLECT_SPARK:          
             Spark::init  (lisp, m_all_models[collectType]); break;
        case COLLECT_MISSILE:        
             Missile::init(lisp, m_all_models[collectType]); break;
        case COLLECT_HOMING: 
             Homing::init (lisp, m_all_models[collectType]); break;
        default:;
    }   // switch

}   // LoadNode

