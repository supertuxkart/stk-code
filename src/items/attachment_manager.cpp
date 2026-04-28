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
   Anchor:    It increases the weight of the kart. But this will NOT have any
              effect on karts already driving at highest speed: the accelerating
       force is independent of the mass, so it is 0 at highest speed
       (engine force = air- plus system-force) and only this value gets
       divided by the mass later --> at highest speed there would be no
       effect when the mass is changed, only at lower speeds the acting
       acceleration will be lower.Reducing the power slows the kart down,
       but doesn't give the feeling of a sudden weight increase.
       Therefore the anchor will reduce by a certain factor (see physics
       parameters) once when it is attached. Together with the mass
       increase (lower acceleration) it's sufficient negative.
       // TODO : rewrite the comment on anchors which is rather poorly written...
*/

//-----------------------------------------------------------------------------
AttachmentManager::~AttachmentManager()
{
    for(int i=0; i < Attachment::ATTACH_COUNT; i++)
    {
        scene::IMesh *mesh = m_attachments[i];
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
/** Loads attachment models and icons from the attachment.xml file.
 */
void AttachmentManager::loadModels()
{
    const std::string file_name = file_manager->getAsset("attachment.xml");
    XMLNode *root               = file_manager->createXMLTree(file_name);
    for(unsigned int i=0; i<root->getNumNodes(); i++)
    {
        const XMLNode *node=root->getNode(i);
        if(node->getName()!="attach-type") continue;
        std::string name;
        node->get("name", &name);
        Attachment::AttachmentType type = getAttachmentType(name);
        if(type == Attachment::ATTACH_NOTHING)
        {
            Log::fatal("AttachmentManager",
                       "Can't find attachment '%s' from attachment.xml, entry %d.",
                       name.c_str(), i+1);
            exit(-1);
        }
        else
        {
            loadAttachment(type, *node);
        }
    }

    delete root;
}   // loadModels

//-----------------------------------------------------------------------------
/** Loads models and icons for one attachment type
 */
void AttachmentManager::loadAttachment(Attachment::AttachmentType type, const XMLNode &node)
{
    std::string icon_file("");
    node.get("icon", &icon_file);

    icon_file = GUIEngine::getSkin()->getThemedIcon(std::string("gui/icons/") + icon_file);
    m_all_icons[type] = material_manager->getMaterial(icon_file,
                                              /*full_path*/             true,
                                              /*make_permanent*/        true,
                                              /*complain_if_not_found*/ true,
                                              /*strip_path*/            false);

    assert(m_all_icons[type] != nullptr);
    assert(m_all_icons[type]->getTexture() != nullptr);

    std::string model("");
    node.get("model-or-lib", &model);

    std::string library_id("");
    node.get("lib-id", &library_id);

    // If there is a library id, 
    if (library_id != "")
    {
        m_attachments[type] = nullptr;
        std::string folder = model;
        AttachableLibraryManager::get()->loadLibraryNode(folder, library_id);
    }
    else // we only have a mesh model
    {
        std::string full_path = file_manager->getAsset(FileManager::MODEL,model);
        scene::IAnimatedMesh* mesh = irr_driver->getAnimatedMesh(full_path);
        mesh->grab();
#ifndef SERVER_ONLY
        SP::uploadSPM(mesh);
#endif
        m_attachments[type] = mesh;
        // TODO: investigate excluding at a higher level?
        if (GUIEngine::isNoGraphics())
            mesh->freeMeshVertexBuffer();
    }
    // TODO HANDLE VAR-x, var-model-or-lib-X,, var-lib-id-X

}   // loadAttachment

//-----------------------------------------------------------------------------
/** Determines the attachment type for a given name.
 *  \param name Name of the attachment to look up.
 *  \return The type, or ATTACHMENT_NOTHING if the name is not found
 */
Attachment::AttachmentType AttachmentManager::getAttachmentType(const std::string &name) const
{
    // Must match the order of AttachmentType in attachment.hpp!!
    static const std::string attachment_names[] = {
        "parachute", "anchor", "bomb", "swatter", "swatter-anim",
        "bubblegum", "bubblegum-small", "electro-shield"
    };

    for(unsigned int i=Attachment::ATTACH_FIRST; i < Attachment::ATTACH_COUNT; i++)
    {
        if(attachment_names[i]==name) return(Attachment::AttachmentType)i;
    }
    return Attachment::ATTACH_NOTHING;
}   // getAttachmentType