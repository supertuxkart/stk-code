//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 Lauri Kasanen
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

#ifndef HEADER_WATER_HPP
#define HEADER_WATER_HPP

#include <IMeshSceneNode.h>
#include <utils/cpp2011.hpp>

using namespace irr;

namespace irr
{
    namespace scene { class IMesh; }
}

// The actual node
class WaterNode: public scene::IMeshSceneNode
{
public:
    WaterNode(scene::ISceneManager* mgr, scene::IMesh *mesh, float height, float speed,
              float length);
    virtual ~WaterNode();

    virtual void render() OVERRIDE;

    virtual const core::aabbox3d<f32>& getBoundingBox() const OVERRIDE
    {
        return m_box;
    }

    virtual void OnRegisterSceneNode() OVERRIDE;

    virtual u32 getMaterialCount() const OVERRIDE { return 1; }
    virtual video::SMaterial& getMaterial(u32 i) OVERRIDE { return m_mat; }

    virtual scene::ESCENE_NODE_TYPE getType() const OVERRIDE { return scene::ESNT_MESH; }

    virtual void setMesh(scene::IMesh *) OVERRIDE {}
    virtual scene::IMesh *getMesh() OVERRIDE { return m_mesh; }
    virtual void setReadOnlyMaterials(bool) OVERRIDE {}
    virtual bool isReadOnlyMaterials() const OVERRIDE { return false; }

protected:
    video::SMaterial m_mat;
    core::aabbox3df m_box;

    scene::IMesh *m_mesh;

    float m_height;
    float m_speed;
    float m_length;
};

#endif
