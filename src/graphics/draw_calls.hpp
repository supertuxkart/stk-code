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

#ifndef HEADER_DRAW_CALLS_HPP
#define HEADER_DRAW_CALLS_HPP

#ifndef SERVER_ONLY
#include "graphics/gl_headers.hpp"
#include <vector>

#include <irrArray.h>
#include <plane3d.h>
#include <vector3d.h>

using namespace irr;

namespace irr
{
    namespace scene
    {
        class ISceneNode; class ICameraSceneNode;
    }
}

class ShadowMatrices;

class DrawCalls
{
private:
    GLsync                                m_sync;
    std::vector<float>                    m_bounding_boxes;

    void parseSceneManager(core::array<scene::ISceneNode*> &List,
                           const scene::ICameraSceneNode *cam);

    bool isCulledPrecise(const scene::ICameraSceneNode *cam,
                         const scene::ISceneNode* node,
                         bool visualization = false);

    bool isBoxInFrontOfPlane(const core::plane3df &plane,
                             const core::vector3df* edges);

    void addEdgeForViz(const core::vector3df &p0, const core::vector3df &p1);

public:
    DrawCalls();
    ~DrawCalls();

    void prepareDrawCalls(irr::scene::ICameraSceneNode *camnode);

    void renderBoundingBoxes();

    void setFenceSync() { m_sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0); }
};

#endif   // !SERVER_ONLY
#endif //HEADER_DRAW_CALLS_HPP
