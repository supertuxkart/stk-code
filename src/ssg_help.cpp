//  $Id: ssg_help.cpp 837 2006-10-23 07:43:05Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006 SuperTuxKart-Team, Joerg Henrichs, Steve Baker
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


#include <plib/ssg.h>

#include "herring_manager.hpp"
#include "sound_manager.hpp"
#include "loader.hpp"
#include "skid_mark.hpp"
#include "config.hpp"
#include "constants.hpp"
#include "shadow.hpp"
#include "track.hpp"
#include "world.hpp"
#include "kart.hpp"

// -----------------------------------------------------------------------------
/// Make VtTables use display lists.
///
/// Calls recursively 'makeDList' in all ssgVtxTable of the entity.
/// \param entity Tree in which to create display lists.
void createDisplayLists(ssgEntity* entity) 
{
    if (!entity) return;
  
    ssgVtxTable* table = dynamic_cast<ssgVtxTable*>(entity);
    if(table)
    {
        if(table->getNumTriangles()>1) table->makeDList();
    }
    ssgBranch* branch = dynamic_cast<ssgBranch*>(entity);
      
    if (branch) 
    {
        for(ssgEntity* i = branch->getKid(0); i != NULL; 
            i = branch->getNextKid()) 
        {
            createDisplayLists(i);
        }   // for
    }   // if branch

}  // createDisplayLists

// -----------------------------------------------------------------------------
/// Adds a transform node to the branch. 
///
/// Creates a new ssgTransform node to which all children of the branch are
/// added. The new ssgTransform is then set as the only child of the
/// branch.
/// \param branch The branch to which a transform node is added.
ssgTransform* add_transform(ssgBranch* branch) 
{
    if (!branch) return 0;
   
    ssgTransform* transform = new ssgTransform;
    transform->ref();
    for(ssgEntity* i = branch->getKid(0); i != NULL; i = branch->getNextKid()) 
    {
        transform->addKid(i);
    }
   
    branch->removeAllKids();
    branch->addKid(transform);
   
    // Set some user data, so that the wheel isn't ssgFlatten()'ed
    branch->setUserData(new ssgBase());
    transform->setUserData(new ssgBase());
   
    return transform;
}   // add_transform

// -----------------------------------------------------------------------------
/// Recursively prints a model.
///
/// Recursively prints a model. That function can most likely be removed, the
/// print method of the ssg objects do the same.
/// \param entity The entity ro print
/// \param indent Indentation to use
/// \param maxLevel maximum number of levels to print
void print_model(ssgEntity* entity, int indent, int maxLevel) 
{
    if(maxLevel <0) return;
    if (entity) 
    {
        for(int i = 0; i < indent; ++i)
            std::cout << "  ";
      
        std::cout << entity->getTypeName() << " " << entity->getType() << " '" 
                  << entity->getPrintableName() 
                  << "' '" 
                  << (entity->getName() ? entity->getName() : "null")
                  << "' " << entity << std::endl;

        ssgBranch* branch = dynamic_cast<ssgBranch*>(entity);
      
        if (branch) 
        {
            for(ssgEntity* i = branch->getKid(0); i != NULL; 
                i = branch->getNextKid()) 
            {
                print_model(i, indent + 1, maxLevel-1);
            }
        }   // if branch
    }   // if entity
}   // print_model

/* EOF */
