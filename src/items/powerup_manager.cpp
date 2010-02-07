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
#include "io/xml_node.hpp"
#include "items/bowling.hpp" 
#include "items/cake.hpp"
#include "items/plunger.hpp"

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
PowerupManager::~PowerupManager()
{
    for(unsigned int i=POWERUP_FIRST; i<=POWERUP_LAST; i++)
    {
        if(m_all_meshes[(PowerupType)i])
            m_all_meshes[(PowerupType)i]->drop();
    }
 
}   // ~PowerupManager

//-----------------------------------------------------------------------------
void PowerupManager::removeTextures()
{
}   // removeTextures

//-----------------------------------------------------------------------------
/** Determines the powerup type for a given name.
 *  \param name Name of the powerup to look up.
 *  \return The type, or POWERUP_NOTHING if the name is not found
 */
PowerupType PowerupManager::getPowerupType(const std::string &name)
{
    // Must match the order of PowerupType in powerup_manager.hpp!!
    static std::string powerup_names[] = {
        "",            /* Nothing */ 
        "bubblegum", "cake", "bowling", "zipper", "plunger", "switch", 
        "parachute", "anchor"
    };

    for(unsigned int i=POWERUP_FIRST; i<=POWERUP_LAST; i++)
    {
        if(powerup_names[i]==name) return(PowerupType)i;
    }
    return POWERUP_NOTHING;
}   // getPowerupType

//-----------------------------------------------------------------------------
/** Loads all projectiles from the powerup.xml file.
 */
void PowerupManager::loadAllPowerups()
{
    XMLNode *root = file_manager->createXMLTree("data/powerup.xml");
    for(unsigned int i=0; i<root->getNumNodes(); i++)
    {
        const XMLNode *node=root->getNode(i);
        std::string name;
        node->get("name", &name);
        PowerupType type = getPowerupType(name);
        LoadPowerup(type, *node);
    }
}  // loadAllPowerups

//-----------------------------------------------------------------------------
void PowerupManager::LoadPowerup(PowerupType type, const XMLNode &node)
{
    std::string icon_file(""); 
    node.get("icon", &icon_file);
    m_all_icons[type] = material_manager->getMaterial(icon_file,
                                  /* full_path */     false,
                                  /*make_permanent */ true); 


    std::string model(""); 
    node.get("model", &model);
    if(model.size()>0)
    {
        // FIXME LEAK: not freed (unimportant, since the models have to exist
        // for the whole game anyway).
        std::string full_path = file_manager->getModelFile(model);
        m_all_meshes[type] = irr_driver->getMesh(full_path);
        if(!m_all_meshes[type])
        {
            std::ostringstream o;
            o<<"Can't load model '"<<model<<"' for powerup type '"<<type<<"', aborting.";
            throw std::runtime_error(o.str());
        }
        m_all_meshes[type]->grab();
    }
    else
    {
        m_all_meshes[type] = 0;
        m_all_extends[type] = btVector3(0.0f,0.0f,0.0f);
    }
    // Load special attributes for certain powerups
    switch (type) {
        case POWERUP_BOWLING:          
             Bowling::init(node, m_all_meshes[type]); break;
        case POWERUP_PLUNGER:          
             Plunger::init(node, m_all_meshes[type]); break;
        case POWERUP_CAKE: 
             Cake::init(node, m_all_meshes[type]);    break;
        default:;
    }   // switch
}   // LoadNode
