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

#include <iostream>
#include <stdexcept>
#include "items/powerup_manager.hpp"
#include "file_manager.hpp"
#include "material_manager.hpp"
#include "material.hpp"
#include "translation.hpp"
#include "items/bowling.hpp" 
#include "items/cake.hpp"
#include "loader.hpp"

#if defined(WIN32) && !defined(__CYGWIN__)
#  define snprintf _snprintf
#endif

typedef struct
{
    PowerupType powerup;
    const char*const dataFile;
}
initPowerupType;

initPowerupType ict[]=
{
    {POWERUP_ZIPPER,    "zipper.collectable"       },
    {POWERUP_BOWLING,   "bowling.projectile"       },
    {POWERUP_BUBBLEGUM, "bubblegum.projectile"     },
    {POWERUP_CAKE,      "cake.projectile"          },
    {POWERUP_ANVIL,     "anvil.collectable"        },
    {POWERUP_PARACHUTE, "parachute.collectable"    },
    {POWERUP_MAX,       ""                         },
};

PowerupManager* powerup_manager=0;

//-----------------------------------------------------------------------------
PowerupManager::PowerupManager()
{
    for(int i=0; i<POWERUP_MAX; i++)
    {
        m_all_models[i] = (ssgEntity*)NULL;
        m_all_icons[i]  = (Material*)NULL;
    }
}   // PowerupManager

//-----------------------------------------------------------------------------
void PowerupManager::removeTextures()
{
    for(int i=0; i<POWERUP_MAX; i++)
    {
        if(m_all_icons [i]) ssgDeRefDelete(m_all_icons [i]->getState());
        if(m_all_models[i]) ssgDeRefDelete(m_all_models[i]            );
    }   // for
    callback_manager->clear(CB_COLLECTABLE);

}   // removeTextures

//-----------------------------------------------------------------------------
void PowerupManager::loadPowerups()
{
    for(int i=0; ict[i].powerup != POWERUP_MAX; i++)
    {
        Load(ict[i].powerup, ict[i].dataFile);
    }
}  // loadPowerups

//-----------------------------------------------------------------------------
void PowerupManager::Load(int collectType, const char* filename)
{
    const lisp::Lisp* ROOT = 0;

    lisp::Parser parser;
    std::string tmp= "data/" + (std::string)filename;
    ROOT = parser.parse(file_manager->getConfigFile(filename));
        
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
void PowerupManager::LoadNode(const lisp::Lisp* lisp, int collectType )
{
    std::string sName, sModel, sIconFile; 
    lisp->get("name",            sName                              );
    lisp->get("model",           sModel                             );
    lisp->get("icon",            sIconFile                          );
 
    // load material
    m_all_icons[collectType] = material_manager->getMaterial(sIconFile);
    m_all_icons[collectType]->getState()->ref();

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

    // Load special attributes for certain powerups
    switch (collectType) {
        case POWERUP_BOWLING:          
             Bowling::init  (lisp, m_all_models[collectType]); break;
        //case POWERUP_BUBBLEGUM:        
        //     BubbleGum::init(lisp, m_all_models[collectType]); break;
        case POWERUP_CAKE: 
             Cake::init (lisp, m_all_models[collectType]); break;
        default:;
    }   // switch

}   // LoadNode

