//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 Lauri Kasanen
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

#ifndef HEADER_LIGHT_HPP
#define HEADER_LIGHT_HPP

#include <ISceneNode.h>
#include <utils/cpp2011.h>
#include <vector>

using namespace irr;

namespace irr
{
    namespace scene { class IMesh; }
}

// The actual node
class LightNode: public scene::ISceneNode
{
public:
    LightNode(scene::ISceneManager* mgr, float radius, float energy, float r, float g, float b);
    virtual ~LightNode();

    virtual void render() OVERRIDE;
    static void renderLightSet(const std::vector<float> &positions, const std::vector<float> &colors, const std::vector<float> &energy);

    virtual const core::aabbox3d<f32>& getBoundingBox() const OVERRIDE
    {
        return box;
    }

    virtual void OnRegisterSceneNode() OVERRIDE;

    virtual u32 getMaterialCount() const OVERRIDE { return 1; }
    virtual bool isPointLight() { return true; }

    float getRadius() const { return m_radius; }
    float getEnergy() const { return energy; }
    core::vector3df getColor() const { return core::vector3df(m_color[0], m_color[1], m_color[2]); }

protected:
    static core::aabbox3df box;

    class ScreenQuad *sq;

    float m_radius;
    float m_color[3];
    float energy;
};

#endif
