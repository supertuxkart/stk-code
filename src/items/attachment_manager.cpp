//  $Id: attachment_manager.cpp 808 2006-10-03 20:17:37Z coz $
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

#include "items/attachment_manager.hpp"

#include "graphics/irr_driver.hpp"
#include "graphics/material_manager.hpp"
#include "io/file_manager.hpp"

AttachmentManager *attachment_manager = 0;

struct  initAttachmentType {attachmentType attachment; const char *file; const char *icon_file;};

/* Some explanations to the attachments:
   Parachute: This will increase the air friction, reducing the maximum speed.
              It will not have too much of an effect on slow speeds, since air
              friction only becomes important at higher speeds.
   Anvil:     It increases the weight of the kart.But this will NOT have any
              effect on karts already driving at highest speed: the accelerating
       force is independent of the mass, so it is 0 at highest speed 
       (engine force = air- plus system-force) and only this value gets
       divided by the mass later --> at highest speed there would be no 
       effect when the mass is changed, only at lower speeds the acting 
       acceleration will be lower.Reducing the power slows the kart down,
       but doesn't give the feeling of a sudden weight increase. 
       Therefore the anvil will reduce by a certain factor (see physics
       parameters) once when it is attached. Together with the mass 
       increase (lower acceleration) it's sufficient negative.
*/

initAttachmentType iat[]=
{
    {ATTACH_PARACHUTE,   "parachute.b3d",   "parachute-attach-icon.png"},
    {ATTACH_BOMB,        "bomb.b3d",        "bomb-attach-icon.png"     },
    {ATTACH_ANVIL,       "anchor.b3d",      "anchor-attach-icon.png"   },
    {ATTACH_TINYTUX,     "reset-button.b3d","reset-attach-icon.png"    },
    {ATTACH_MAX,         "",                ""                         },
};

//-----------------------------------------------------------------------------
void AttachmentManager::removeTextures()
{
    for(int i=0; iat[i].attachment!=ATTACH_MAX; i++)
    {
        // FIXME: free attachment textures
    }   // for
}   // removeTextures

//-----------------------------------------------------------------------------
void AttachmentManager::loadModels()
{
    for(int i=0; iat[i].attachment!=ATTACH_MAX; i++)
    {
        // FIXME LEAK: these models are not removed (unimportant, since they
        // have to be in memory till the end of the game.
        std::string full_path = file_manager->getModelFile(iat[i].file);
        m_attachments[iat[i].attachment]=irr_driver->getAnimatedMesh(full_path);
        if(iat[i].icon_file)
        {
            std::string full_icon_path     =
                file_manager->getModelFile(iat[i].icon_file);
            m_all_icons[iat[i].attachment] =
                material_manager->getMaterial(full_icon_path,
                                              /* full_path */     true,
                                              /*make_permanent */ true); 
        }

    }   // for
}   // reInit

