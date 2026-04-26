//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2015 Joerg Henrichs
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

#include "graphics/attachable_library_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/sp/sp_base.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/skin.hpp"
#include "io/file_manager.hpp"

#include <IAnimatedMesh.h>

AttachmentManager *attachment_manager = 0;

struct  initAttachmentType {Attachment::AttachmentType attachment;
                            const char *file;
                            const char *icon_file;
                            bool library_attach;
                            const char *library_id;};

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

static const initAttachmentType iat[]=
{
    // TODO: Move this info to a config file
    {Attachment::ATTACH_PARACHUTE,        "parachute.spm",        "parachute-attach-icon.png",   false, "" },
    {Attachment::ATTACH_BOMB,             "bomb.spm",             "bomb-attach-icon.png",        false, "" },
    {Attachment::ATTACH_ANVIL,            "anchor.spm",           "anchor-attach-icon.png",      false, "" },
    {Attachment::ATTACH_SWATTER,          "swatter.spm",          "swatter-icon.png",            false, "" },
    {Attachment::ATTACH_NOLOKS_SWATTER,   "swatter_nolok.spm",    "swatter-icon.png",            false, "" },
    {Attachment::ATTACH_SWATTER_ANIM,     "swatter_anim.spm",     "swatter-icon.png",            false, "" },
    {Attachment::ATTACH_BUBBLEGUM_SHIELD, "bubblegum_shield.spm", "shield-icon.png",             false, "" },
    {Attachment::ATTACH_NOLOK_BUBBLEGUM_SHIELD, "bubblegum_shield_nolok.spm", "shield-icon.png", false, "" },
    {Attachment::ATTACH_BUBBLEGUM_SHIELD_SMALL, "bubblegum_shield.spm", "shield-icon.png",       false, "" },
    {Attachment::ATTACH_NOLOK_BUBBLEGUM_SHIELD_SMALL, "bubblegum_shield_nolok.spm", "shield-icon.png", false, "" },
    {Attachment::ATTACH_ELECTRO_SHIELD,   "stklib_electro_shield_a",   "electro-shield-icon.png", true, "electro-shield"},
    {Attachment::ATTACH_MAX,              "",                     "",                            false, ""},
};

//-----------------------------------------------------------------------------
AttachmentManager::~AttachmentManager()
{
    for(int i=0; iat[i].attachment!=Attachment::ATTACH_MAX; i++)
    {
        scene::IMesh *mesh = m_attachments[iat[i].attachment];
        if (mesh == nullptr)
            continue;

        mesh->drop();
        // If the count is 1, the only reference is in the
        // irrlicht mesh cache, so the mesh can be removed
        // from the cache.
        // Note that this test is necessary, since some meshes
        // are also used in powerup_manager!!!
        if(mesh->getReferenceCount()==1)
            irr_driver->removeMeshFromCache(mesh);
    }
}   // ~AttachmentManager

//-----------------------------------------------------------------------------
void AttachmentManager::loadModels()
{
    for(int i=0; iat[i].attachment!=Attachment::ATTACH_MAX; i++)
    {
        if (iat[i].library_attach)
        {
            m_attachments[iat[i].attachment] = nullptr;
            std::string folder = iat[i].file;
            assert(iat[i].library_id != "");
            std::string lib_id = iat[i].library_id;
            AttachableLibraryManager::get()->loadLibraryNode(folder, lib_id);
        }
        else
        {
            std::string full_path = file_manager->getAsset(FileManager::MODEL,iat[i].file);
            scene::IAnimatedMesh* mesh = irr_driver->getAnimatedMesh(full_path);
            mesh->grab();
#ifndef SERVER_ONLY
            SP::uploadSPM(mesh);
#endif
            m_attachments[iat[i].attachment] = mesh;
            // TODO: investigate excluding at a higher level?
            if (GUIEngine::isNoGraphics())
                mesh->freeMeshVertexBuffer();
        }

        if(iat[i].icon_file)
        {
            std::string full_icon_path     =
                GUIEngine::getSkin()->getThemedIcon(std::string("gui/icons/")
                                                    + iat[i].icon_file);
            m_all_icons[iat[i].attachment] =
                material_manager->getMaterial(full_icon_path,
                                              /*full_path*/             true,
                                              /*make_permanent*/        true,
                                              /*complain_if_not_found*/ true,
                                              /*strip_path*/            false);
        }
    }   // for
}   // reInit

