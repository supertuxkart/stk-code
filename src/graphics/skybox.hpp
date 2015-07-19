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
#include <ITexture.h>
#include <IVideoDriver.h>
#include <vector>

class Skybox
{
private:
    /** The cube map texture id */
    GLuint m_cube_map;
    /** The specular probe texture id */
    GLuint m_specular_probe;   

    /** The spherical harmonic coefficients */
    float m_blue_SH_coeff[9];
    float m_green_SH_coeff[9];
    float m_red_SH_coeff[9];
    


    void generateDiffuseCoefficients(irr::video::IVideoDriver *video_driver,
                                     const std::vector<irr::video::ITexture *> &spherical_harmonics_textures,
                                     const irr::video::SColor &ambient);
                                     
    GLuint generateCubeMapFromTextures(const std::vector<irr::video::ITexture *> &skybox_textures);

    
    
    
public:
    Skybox(irr::video::IVideoDriver *video_driver,
           const std::vector<irr::video::ITexture *> &skybox_textures,
           const std::vector<irr::video::ITexture *> &spherical_harmonics_textures,
           const irr::video::SColor &ambient);
    ~Skybox();
    
    inline const float* getBlueSHCoeff()  const     {return m_blue_SH_coeff;  }
    inline const float* getGreenSHCoeff() const     {return m_green_SH_coeff; }
    inline const float* getRedSHCoeff()   const     {return m_red_SH_coeff;   }



};

#endif //HEADER_SKYBOX_HPP
