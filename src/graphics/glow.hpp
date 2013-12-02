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

#ifndef HEADER_GLOW_HPP
#define HEADER_GLOW_HPP

#include <ISceneNode.h>
#include <IMesh.h>

using namespace irr;

// The actual node
class GlowNode: public scene::ISceneNode
{
public:
    GlowNode(scene::ISceneManager* mgr, float radius);
    ~GlowNode();

    virtual void render();

    virtual const core::aabbox3d<f32>& getBoundingBox() const
    {
        return box;
    }

    virtual void OnRegisterSceneNode();

    virtual u32 getMaterialCount() const { return 1; }
    virtual video::SMaterial& getMaterial(u32 i) { return mat; }

private:
    static video::SMaterial mat;
    static core::aabbox3df box;

    static scene::IMesh *sphere;
};

#endif
