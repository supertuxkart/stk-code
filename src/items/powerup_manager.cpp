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

#include "items/powerup_manager.hpp"

#include <iostream>
#include <stdexcept>
#include <sstream>

#include "material_manager.hpp"
#include "material.hpp"
#include "graphics/irr_driver.hpp"
#include "io/file_manager.hpp"
#include "items/bowling.hpp" 
#include "items/cake.hpp"
#include "items/plunger.hpp"
#include "loader.hpp"


typedef struct
{
    PowerupType powerup;
    const char*const dataFile;
}
initPowerupType;

initPowerupType ict[]=
{
#ifdef HAVE_IRRLICHT
    {POWERUP_ZIPPER,    "zipper.collectable"       },
    {POWERUP_BOWLING,   "bowling.projectile"       },
    {POWERUP_BUBBLEGUM, "bubblegum.xml"            },
    {POWERUP_CAKE,      "cake.projectile"          },
    {POWERUP_ANVIL,     "anvil.collectable"        },
    {POWERUP_PARACHUTE, "parachute.collectable"    },
    {POWERUP_PLUNGER,   "plunger.projectile"       },
    {POWERUP_MAX,       ""                         },
#else
    {POWERUP_ZIPPER,    "zipper.collectable"       },
    {POWERUP_BOWLING,   "bowling.projectile"       },
    {POWERUP_BUBBLEGUM, "bubblegum.projectile"     },
    {POWERUP_CAKE,      "cake.projectile"          },
    {POWERUP_ANVIL,     "anvil.collectable"        },
    {POWERUP_PARACHUTE, "parachute.collectable"    },
    {POWERUP_PLUNGER,   "plunger.projectile"       },
    {POWERUP_MAX,       ""                         },
#endif
};

PowerupManager* powerup_manager=0;

//-----------------------------------------------------------------------------
PowerupManager::PowerupManager()
{
    for(int i=0; i<POWERUP_MAX; i++)
    {
#ifdef HAVE_IRRLICHT
        m_all_meshes[i] = NULL;
#else
        m_all_models[i] = (ssgEntity*)NULL;
#endif
        m_all_icons[i]  = (Material*)NULL;
    }
}   // PowerupManager

//-----------------------------------------------------------------------------
void PowerupManager::removeTextures()
{
#ifndef HAVE_IRRLICHT
    for(int i=0; i<POWERUP_MAX; i++)
    {
        if(m_all_icons [i]) ssgDeRefDelete(m_all_icons [i]->getState());
        if(m_all_models[i]) ssgDeRefDelete(m_all_models[i]            );
    }   // for
#endif
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
        std::ostringstream msg;
        msg << "No 'tuxkart-collectable' node found while parsing '" 
            << filename << "'.";
        throw std::runtime_error(msg.str());
    }
    LoadNode(lisp, collectType);

    delete ROOT;

}   // Load

//-----------------------------------------------------------------------------
void PowerupManager::LoadNode(const lisp::Lisp* lisp, int collectType )
{
    std::string sName, sModel, sIconFile; 
    lisp->get("name",            sName                              );
#ifdef HAVE_IRRLICHT
    lisp->get("mesh",            sModel                             );
#else
    lisp->get("model",           sModel                             );
#endif
    lisp->get("icon",            sIconFile                          );
    // load material
    m_all_icons[collectType] = material_manager->getMaterial(sIconFile,
                                                     /* full_path */    false,
                                                     /*make_permanent */ true); 
#ifdef HAVE_IRRLICHT
    if(sModel!="")
    {
        // FIXME LEAK: not freed (unimportant, since the models have to exist
        // for the whole game anyway).
        std::string full_path = file_manager->getModelFile(sModel);
        m_all_meshes[collectType] = irr_driver->getMesh(full_path);
    }
    else
    {
        m_all_meshes[collectType] = 0;
        m_all_extends[collectType] = btVector3(0.0f,0.0f,0.0f);
    }
#else
    m_all_icons[collectType]->getState()->ref();

    if(sModel!="")
    {
        // FIXME LEAK: not freed (unimportant, since the models have to exist
        // for the whole game anyway).
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
#endif

    // Load special attributes for certain powerups
#ifdef HAVE_IRRLICHT
    switch (collectType) {
        case POWERUP_BOWLING:          
             Bowling::init(lisp, m_all_meshes[collectType]); break;
        case POWERUP_PLUNGER:          
             Plunger::init(lisp, m_all_meshes[collectType]); break;
        case POWERUP_CAKE: 
             Cake::init(lisp, m_all_meshes[collectType]); break;
        default:;
    }   // switch
#else
    switch (collectType) {
        case POWERUP_BOWLING:          
             Bowling::init  (lisp, m_all_models[collectType]); break;
        case POWERUP_PLUNGER:          
             Plunger::init  (lisp, m_all_models[collectType]); break;
        case POWERUP_CAKE: 
             Cake::init (lisp, m_all_models[collectType]); break;
        default:;
    }   // switch
#endif
}   // LoadNode

