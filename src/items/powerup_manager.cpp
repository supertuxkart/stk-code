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

#include "graphics/irr_driver.hpp"
#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "io/file_manager.hpp"
#include "items/bowling.hpp" 
#include "items/cake.hpp"
#include "items/plunger.hpp"


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
    {POWERUP_BUBBLEGUM, "bubblegum.xml"            },
    {POWERUP_CAKE,      "cake.projectile"          },
    {POWERUP_ANVIL,     "anvil.collectable"        },
    {POWERUP_SWITCH,    "switch.collectable"       },
    {POWERUP_PARACHUTE, "parachute.collectable"    },
    {POWERUP_PLUNGER,   "plunger.projectile"       },
    {POWERUP_MAX,       ""                         },
};

PowerupManager* powerup_manager=0;

//-----------------------------------------------------------------------------
PowerupManager::PowerupManager()
{
    for(int i=0; i<POWERUP_MAX; i++)
    {
        m_all_meshes[i] = NULL;
        m_all_icons[i]  = (Material*)NULL;
    }
}   // PowerupManager

//-----------------------------------------------------------------------------
void PowerupManager::removeTextures()
{
}   // removeTextures

//-----------------------------------------------------------------------------
/** Loads all projectiles from the powerup.xml file.
 */
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
    lisp->get("mesh",            sModel                             );
    lisp->get("icon",            sIconFile                          );
    // load material
    m_all_icons[collectType] = material_manager->getMaterial(sIconFile,
                                                     /* full_path */    false,
                                                     /*make_permanent */ true); 
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
    if(!m_all_meshes[collectType])
    {
        std::ostringstream o;
        o<<"Can't load model '"<<sModel<<"' for '"<<sName<<"', aborting.";
        throw std::runtime_error(o.str());
    }
    // Load special attributes for certain powerups
    switch (collectType) {
        case POWERUP_BOWLING:          
             Bowling::init(lisp, m_all_meshes[collectType]); break;
        case POWERUP_PLUNGER:          
             Plunger::init(lisp, m_all_meshes[collectType]); break;
        case POWERUP_CAKE: 
             Cake::init(lisp, m_all_meshes[collectType]); break;
        default:;
    }   // switch
}   // LoadNode

