//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 SuperTuxKart-Team
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

#ifndef SERVER_ONLY

#include "graphics/stk_scene_manager.hpp"
#include <SViewFrustum.h>
#include <vector>

using namespace irr;

// From irrlicht code
static
bool isBoxInFrontOfPlane(const core::plane3df &plane, const core::vector3df edges[8])
{
    for (u32 j = 0; j<8; ++j)
        if (plane.classifyPointRelation(edges[j]) != core::ISREL3D_FRONT)
            return false;
    return true;
}

std::vector<float> BoundingBoxes;

void addEdge(const core::vector3df &P0, const core::vector3df &P1)
{
    BoundingBoxes.push_back(P0.X);
    BoundingBoxes.push_back(P0.Y);
    BoundingBoxes.push_back(P0.Z);
    BoundingBoxes.push_back(P1.X);
    BoundingBoxes.push_back(P1.Y);
    BoundingBoxes.push_back(P1.Z);
}

bool isCulledPrecise(const scene::ICameraSceneNode *cam, const scene::ISceneNode *node)
{
    if (!node->getAutomaticCulling())
        return false;

    const core::matrix4 &trans = node->getAbsoluteTransformation();
    const scene::SViewFrustum &frust = *cam->getViewFrustum();

    core::vector3df edges[8];
    node->getBoundingBox().getEdges(edges);
    for (unsigned i = 0; i < 8; i++)
        trans.transformVect(edges[i]);

    for (s32 i = 0; i < scene::SViewFrustum::VF_PLANE_COUNT; ++i)
        if (isBoxInFrontOfPlane(frust.planes[i], edges))
            return true;
    return false;
}

#endif   // !SERVER_ONLY
