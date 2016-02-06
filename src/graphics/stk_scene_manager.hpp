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


// Not really a scene manager yet but hold algorithm that
// rework scene manager output

#ifndef HEADER_STKSCENEMANAGER_HPP
#define HEADER_STKSCENEMANAGER_HPP

#include "graphics/central_settings.hpp"
#include "graphics/gl_headers.hpp"
#include "graphics/gpu_particles.hpp"
#include "graphics/stk_billboard.hpp"
#include "graphics/stk_mesh.hpp"

void addEdge(const core::vector3df &P0, const core::vector3df &P1);

bool isCulledPrecise(const scene::ICameraSceneNode *cam, const scene::ISceneNode *node);

#endif
