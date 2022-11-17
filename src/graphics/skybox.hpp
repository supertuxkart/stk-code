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


#ifndef HEADER_SKYBOX_HPP
#define HEADER_SKYBOX_HPP

#include "graphics/gl_headers.hpp"
#include <vector>

namespace irr
{
    namespace scene
    {
        class ISceneNode; class ICameraSceneNode;
    }
    namespace video
    {
        class IImage; class ITexture;
    }
}

class Skybox
{
private:
    /** The 6 skybox images */
    std::vector<irr::video::IImage *> m_skybox_textures;
    
    /** The skybox texture id */
    GLuint m_cube_map;
    
    /** The specular probe texture id */
    GLuint m_specular_probe;   
    

    void generateCubeMapFromTextures ();
    void generateSpecularCubemap ();
    
public:
    Skybox(const std::vector<irr::video::IImage *> &skybox_textures);
    ~Skybox();
    
    void render(const irr::scene::ICameraSceneNode *camera) const;

    inline GLuint getSpecularProbe()      const     {return m_specular_probe; }


};

#endif //HEADER_SKYBOX_HPP
