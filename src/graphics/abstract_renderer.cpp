//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2015 SuperTuxKart-Team
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

#include "graphics/abstract_renderer.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/scalable_font.hpp"

using namespace irr;

#ifdef DEBUG
#include <IAnimatedMeshSceneNode.h>
#include <ISceneCollisionManager.h>
#include <ISceneManager.h>
#include <ISkinnedMesh.h>
#include <IVideoDriver.h>

void AbstractRenderer::drawDebugMeshes() const
{
    std::vector<irr::scene::IAnimatedMeshSceneNode*> debug_meshes = irr_driver->getDebugMeshes();
    
    for (unsigned int n=0; n<debug_meshes.size(); n++)
    {
        scene::IMesh* mesh = debug_meshes[n]->getMesh();
        scene::ISkinnedMesh* smesh = static_cast<scene::ISkinnedMesh*>(mesh);
        const core::array< scene::ISkinnedMesh::SJoint * >& joints =
            smesh->getAllJoints();

        for (unsigned int j=0; j<joints.size(); j++)
        {
            drawJoint( false, true, joints[j], smesh, j);
        }
    }

    video::SColor color(255,255,255,255);
    video::SMaterial material;
    material.Thickness = 2;
    material.AmbientColor = color;
    material.DiffuseColor = color;
    material.EmissiveColor= color;
    material.BackfaceCulling = false;
    material.setFlag(video::EMF_LIGHTING, false);
    irr_driver->getVideoDriver()->setMaterial(material);
    irr_driver->getVideoDriver()->setTransform(video::ETS_WORLD, core::IdentityMatrix);

    for (unsigned int n=0; n<debug_meshes.size(); n++)
    {
        scene::IMesh* mesh = debug_meshes[n]->getMesh();


        scene::ISkinnedMesh* smesh = static_cast<scene::ISkinnedMesh*>(mesh);
        const core::array< scene::ISkinnedMesh::SJoint * >& joints =
            smesh->getAllJoints();

        for (unsigned int j=0; j<joints.size(); j++)
        {
            scene::IMesh* mesh = debug_meshes[n]->getMesh();
            scene::ISkinnedMesh* smesh = static_cast<scene::ISkinnedMesh*>(mesh);

            drawJoint(true, false, joints[j], smesh, j);
        }
    }     
}  // drawDebugMeshes

// ----------------------------------------------------------------------------
/** Draws a joint for debugging skeletons.
 *  \param drawline If true draw a line to the parent.
 *  \param drawname If true draw the name of the joint.
 *  \param sjoint The joint to draw.
 *  \param mesh The mesh whose skeleton is drawn (only used to get
 *         all joints to find the parent).
 *  \param id Index, which (%4) determines the color to use.
 */
void AbstractRenderer::drawJoint(bool drawline, bool drawname,
                                 void* sjoint,
                                 scene::ISkinnedMesh* mesh, int id) const
{
    scene::ISkinnedMesh::SJoint* joint = (scene::ISkinnedMesh::SJoint*)sjoint;
    scene::ISkinnedMesh::SJoint* parent = NULL;
    const core::array< scene::ISkinnedMesh::SJoint * >& joints
        = mesh->getAllJoints();
    for (unsigned int j=0; j<joints.size(); j++)
    {
        if (joints[j]->Children.linear_search(joint) != -1)
        {
            parent = joints[j];
            break;
        }
    }

    core::vector3df jointpos = joint->GlobalMatrix.getTranslation();

    video::SColor color(255, 255,255,255);
    if (parent == NULL) color = video::SColor(255,0,255,0);

    switch (id % 4)
    {
        case 0:
            color = video::SColor(255,255,0,255);
            break;
        case 1:
            color = video::SColor(255,255,0,0);
            break;
        case 2:
            color = video::SColor(255,0,0,255);
            break;
        case 3:
            color = video::SColor(255,0,255,255);
            break;
    }


    if (parent)
    {
        core::vector3df parentpos = parent->GlobalMatrix.getTranslation();

        jointpos = joint->GlobalMatrix.getTranslation();

        if (drawline)
        {
            irr_driver->getVideoDriver()->draw3DLine(jointpos,
                                                     parentpos,
                                                     color);
        }
    }

    if (joint->Children.size() == 0)
    {
        switch ((id + 1) % 4)
        {
            case 0:
                color = video::SColor(255,255,0,255);
                break;
            case 1:
                color = video::SColor(255,255,0,0);
                break;
            case 2:
                color = video::SColor(255,0,0,255);
                break;
            case 3:
                color = video::SColor(255,0,255,255);
                break;
        }

        // This code doesn't quite work. 0.25 is used so that the bone is not
        // way too long (not sure why I need to manually size it down)
        // and the rotation of the bone is often rather off
        core::vector3df v(0.0f, 0.25f, 0.0f);
        //joint->GlobalMatrix.rotateVect(v);
        joint->LocalMatrix.rotateVect(v);
        v *= joint->LocalMatrix.getScale();
        irr_driver->getVideoDriver()->draw3DLine(jointpos,
                                                 jointpos + v,
                                                 color);
    }

    switch ((id + 1) % 4)
    {
        case 0:
            color = video::SColor(255,255,0,255);
            break;
        case 1:
            color = video::SColor(255,255,0,0);
            break;
        case 2:
            color = video::SColor(255,0,0,255);
            break;
        case 3:
            color = video::SColor(255,0,255,255);
            break;
    }

    if (drawname)
    {
        irr_driver->getVideoDriver()->setTransform(video::ETS_WORLD,
                                                   core::IdentityMatrix);

        core::vector2di textpos =
            irr_driver->getSceneManager()->getSceneCollisionManager()
            ->getScreenCoordinatesFrom3DPosition(jointpos);

        GUIEngine::getSmallFont()->draw( core::stringw(joint->Name.c_str()),
                                         core::rect<s32>(textpos,
                                               core::dimension2d<s32>(500,50)),
                                         color, false, false );
    }
} //drawJoint

#endif //DEBUG
