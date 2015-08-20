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

#ifndef HEADER_LIGHT_PREPASS_RENDERER_HPP
#define HEADER_LIGHT_PREPASS_RENDERER_HPP

#include "graphics/abstract_renderer.hpp"
#include "graphics/irr_driver.hpp"


class ShaderBasedRenderer: public AbstractRenderer
{
private:
    bool                 m_wireframe;
    bool                 m_mipviz;
    
    
    void compressPowerUpTextures();
    void setOverrideMaterial();
    std::vector<IrrDriver::GlowData> updateGlowingList();
    void prepareForwardRenderer();

    void renderBoundingBoxes();
    void debugPhysics();
    void renderPostProcessing(Camera * const camera);

public:
    
    void render(float dt);
};

#endif //HEADER_LIGHT_PREPASS_RENDERER_HPP
