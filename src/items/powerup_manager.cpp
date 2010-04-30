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
#include "modes/world.hpp"
#include "utils/constants.hpp"
#include "utils/string_utils.hpp"

PowerupManager* powerup_manager=0;

//-----------------------------------------------------------------------------
/** The constructor initialises everything to zero. */
PowerupManager::PowerupManager()
{
    for(int i=0; i<POWERUP_MAX; i++)
    {
        m_all_meshes[i] = NULL;
        m_all_icons[i]  = (Material*)NULL;
    }
}   // PowerupManager

//-----------------------------------------------------------------------------
/** Destructor, frees all meshes. */
PowerupManager::~PowerupManager()
{
    for(unsigned int i=POWERUP_FIRST; i<=POWERUP_LAST; i++)
    {
        if(m_all_meshes[(PowerupType)i])
            m_all_meshes[(PowerupType)i]->drop();
    }
}   // ~PowerupManager

//-----------------------------------------------------------------------------
/** Removes any textures so that they can be reloaded. FIXME: missing atm.
 */
void PowerupManager::removeTextures()
{
}   // removeTextures

//-----------------------------------------------------------------------------
/** Determines the powerup type for a given name.
 *  \param name Name of the powerup to look up.
 *  \return The type, or POWERUP_NOTHING if the name is not found
 */
PowerupManager::PowerupType PowerupManager::getPowerupType(const std::string &name) const
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
    const std::string file_name = file_manager->getConfigFile("powerup.xml");
    XMLNode *root               = file_manager->createXMLTree(file_name);
    for(unsigned int i=0; i<root->getNumNodes(); i++)
    {
        const XMLNode *node=root->getNode(i);
        std::string name;
        node->get("name", &name);
        PowerupType type = getPowerupType(name);
        // The weight nodes will be also included in this list, so ignore those
        if(type!=POWERUP_NOTHING)
            LoadPowerup(type, *node);
    }
    loadWeights(*root, "first", POSITION_FIRST);
    loadWeights(*root, "top33", POSITION_TOP33);
    loadWeights(*root, "mid33", POSITION_MID33);
    loadWeights(*root, "end33", POSITION_END33);
    loadWeights(*root, "last" , POSITION_LAST );
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

// ----------------------------------------------------------------------------
/** Loads a weight list specified in powerup.xml. The different position
 *  classes must be loaded in the right order
 *  \param root The root node of powerup.xml 
 *  \param class_name The name of the position class to load.
 *  \param position_class The class for which the weights are read.
 */
void PowerupManager::loadWeights(const XMLNode &root,
                                 const std::string &class_name,
                                 PositionClass position_class)
{
    const XMLNode *node = root.getNode(class_name);
    std::string s("");
    if(node) node->get("w", &s);

    if(!node || s=="")
    {
        printf("No weights found for class '%s' - probabilities will be incorrect.\n",
               class_name.c_str());
        return;
    }

    std::vector<std::string> weight_list = StringUtils::split(s, ' ');

    std::vector<std::string>::iterator i=weight_list.begin();
    while(i!=weight_list.end())
    {
        if(*i=="")
        {
            std::vector<std::string>::iterator next=weight_list.erase(i);
            i=next;
        }
        else
            i++;
    }
    // Fill missing entries with 0s
    while(weight_list.size()<(int)POWERUP_LAST)
        weight_list.push_back(0);

    if(weight_list.size()!=(int)POWERUP_LAST)
    {
        printf("Incorrect number of weights found in class '%s':\n", 
               class_name.c_str());
        printf("%d instead of %d - probabilities will be incorrect.\n",
               weight_list.size(), (int)POWERUP_LAST);
        return;
    }

    for(unsigned int i=0; i<weight_list.size(); i++)
    {
        int w = atoi(weight_list[i].c_str());
        m_weights[position_class].push_back(w);
    }

}   // loadWeights

// ----------------------------------------------------------------------------
/** This function set up various arrays for faster lookup later. It first
 *  determines which race position correspond to which position class, and
 *  then filters which powerups are available (not all powerups might be
 *  available in all races). It sets up two arrays: m_position_to_class
 *  which maps which race position corresponds to which position class
 *  \param num_kart Number of karts.
 */
void PowerupManager::updateWeightsForRace(unsigned int num_karts)
{
    m_position_to_class.clear();
    const World *world = World::getWorld();
    for(unsigned int position =1; position <= num_karts; position++)
    {
        // Set up the mapping of position to position class:
        // -------------------------------------------------
        PositionClass pos_class = convertPositionToClass(num_karts, position);
        m_position_to_class.push_back(pos_class);

        // Then determine which items are available:
        m_powerups_for_position[pos_class].clear();
        unsigned int sum = 0;
        for(unsigned int i= POWERUP_FIRST; i<=POWERUP_LAST; i++)
        {
            PowerupType type=(PowerupType)i;
            if(!world->acceptPowerup(type)) continue;
            unsigned int w =m_weights[pos_class][i-POWERUP_FIRST]; 
            sum += w;
            for(unsigned int i=0; i<w; i++)
                m_powerups_for_position[pos_class].push_back(type);
        }   // for type in [POWERUP_FIRST, POWERUP_LAST]
    }

}   // updateWeightsForRace

// ----------------------------------------------------------------------------
/** Determines the position class for a 
 */
PowerupManager::PositionClass 
               PowerupManager::convertPositionToClass(unsigned int num_karts, 
                                                     unsigned int position)
{
    if(position==1)         return POSITION_FIRST;
    if(position==num_karts) return POSITION_LAST;

    // Now num_karts must be >2, since position <=num_players
    
    unsigned int third = (unsigned int)floor((float)(num_karts-1)/3.0f);
    // 1 < Position <= 1+third is top33
    if(position <= 1 + third) return POSITION_TOP33;

    // num_players-third < position is end33
    if(num_karts - third <= position) return POSITION_END33;

    return POSITION_MID33;
}   // convertPositionToClass

// ----------------------------------------------------------------------------
PowerupManager::PowerupType PowerupManager::getRandomPowerup(unsigned int pos)
{
    // Positions start with 1, while the index starts with 0 - so subtract 1
    PositionClass pos_class = m_position_to_class[pos-1];

    int random = rand()%m_powerups_for_position[pos_class].size();
    return m_powerups_for_position[pos_class][random];
}   // getRandomPowerup