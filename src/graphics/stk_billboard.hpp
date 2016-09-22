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

#ifndef STKBILLBOARD_HPP
#define STKBILLBOARD_HPP

#include "stkirrlicht/CBillboardSceneNode.h"
#include <IBillboardSceneNode.h>
#include <irrTypes.h>
#include "utils/cpp2011.hpp"

class STKBillboard : public irr::scene::CBillboardSceneNode
{
public:
    STKBillboard(irr::scene::ISceneNode* parent, irr::scene::ISceneManager* mgr,
                 irr::s32 id, const irr::core::vector3df& position, 
                 const irr::core::dimension2d<irr::f32>& size,
                 irr::video::SColor colorTop = irr::video::SColor(0xFFFFFFFF),
                 irr::video::SColor colorBottom = irr::video::SColor(0xFFFFFFFF));

    virtual void OnRegisterSceneNode() OVERRIDE;

    virtual void render() OVERRIDE;
};   // STKBillboard

#endif
