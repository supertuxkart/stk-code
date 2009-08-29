//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Marianne Gagnon
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



#ifndef HEADER_MODELVIEW_HPP
#define HEADER_MODELVIEW_HPP

#include <irrlicht.h>

#include "graphics/irr_driver.hpp"
#include "guiengine/widget.hpp"
#include "utils/ptr_vector.hpp"

using namespace irr;
using namespace gui;


namespace GUIEngine
{
    /** A model view widget. See guiengine/engine.hpp for a detailed overview */
    class ModelViewWidget : public Widget
    {
        
        ptr_vector<scene::IMesh, REF> m_models;
        std::vector<Vec3> m_model_location;
        
        video::ITexture* m_texture;
        
        IrrDriver::RTTProvider* m_rtt_provider;
        
        float angle;
    public:
        ModelViewWidget();
        ~ModelViewWidget();
        
        void add();
        void clearModels();
        void addModel(irr::scene::IMesh* mesh, const Vec3& location = Vec3(0,0,0));
        void update(float delta);
    };
    
}

#endif
