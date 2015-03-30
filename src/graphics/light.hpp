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

#ifndef HEADER_LIGHT_HPP
#define HEADER_LIGHT_HPP

#include <ISceneNode.h>
#include <utils/cpp2011.hpp>
#include <vector>

using namespace irr;

namespace irr
{
    namespace scene { class IMesh; }
}

//#define __LIGHT_NODE_VISUALISATION__

// The actual node
class LightNode: public scene::ISceneNode
{
#ifdef __LIGHT_NODE_VISUALISATION__
    bool m_viz_added;
#endif

public:
    LightNode(scene::ISceneManager* mgr, scene::ISceneNode* parent, float energy, float d, float r, float g, float b);
    virtual ~LightNode();

    virtual void render() OVERRIDE;

    virtual const core::aabbox3d<f32>& getBoundingBox() const OVERRIDE
    {
        return box;
    }

    virtual void OnRegisterSceneNode() OVERRIDE;

    virtual u32 getMaterialCount() const OVERRIDE { return 1; }
    virtual bool isPointLight() { return true; }

    float getRadius() const { return m_radius; }
    float getEnergy() const { return m_energy; }
    float getEffectiveEnergy() const { return m_energy_multiplier * m_energy; }
    core::vector3df getColor() const { return core::vector3df(m_color[0], m_color[1], m_color[2]); }
    void setColor(float r, float g, float b) { m_color[0] = r; m_color[1] = g; m_color[2] = b; }

    float getEnergyMultiplier() const { return m_energy_multiplier; }
    void  setEnergyMultiplier(float newval) { m_energy_multiplier = newval; }

    // For the debug menu
    void setEnergy(float energy) { m_energy = energy; }
    void setRadius(float radius) { m_radius = radius; }

protected:
    static core::aabbox3df box;

    float m_radius;
    float m_color[3];
    float m_energy;

    /// The energy multiplier is in range [0, 1] and is used to fade in lights when they come in range
    float m_energy_multiplier;
};

#endif
