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


#ifndef HEADER_SPHERICAL_HARMONICS_HPP
#define HEADER_SPHERICAL_HARMONICS_HPP

#include <ITexture.h>
#include <vector>

struct Color
{
    float Red;
    float Green;
    float Blue;
};


class SphericalHarmonics
{
private:
    /** The 6 spherical harmonics textures */
    std::vector<irr::video::ITexture *> m_spherical_harmonics_textures;
    
    /** Ambient light is used for tracks without spherical harmonics textures */
    irr::video::SColor m_ambient;
    
    /** The spherical harmonics coefficients */
    float m_blue_SH_coeff[9];
    float m_green_SH_coeff[9];
    float m_red_SH_coeff[9];
    

    void projectSH(Color *cubemap_face[6], size_t edge_size, float *Y00[],
                      float *Y1minus1[], float *Y10[], float *Y11[],
                      float *Y2minus2[], float *Y2minus1[], float * Y20[],
                      float *Y21[], float *Y22[]);
    
    void generateSphericalHarmonics(Color *cubemap_face[6], size_t edge_size);
    
public:
    SphericalHarmonics(const std::vector<irr::video::ITexture *> &spherical_harmonics_textures);
    SphericalHarmonics(const irr::video::SColor &ambient);
    
    void setTextures(const std::vector<irr::video::ITexture *> &spherical_harmonics_textures);    
    void setAmbientLight(const irr::video::SColor &ambient);
    
    inline const float* getBlueSHCoeff () const     {return m_blue_SH_coeff;  }
    inline const float* getGreenSHCoeff() const     {return m_green_SH_coeff; }
    inline const float* getRedSHCoeff  () const     {return m_red_SH_coeff;   }    
    
    void printCoeff();
    
    void unprojectSH (size_t width, size_t height,
                      float *Y00[], float *Y1minus1[], float *Y10[],
                      float *Y11[], float *Y2minus2[], float *Y2minus1[],
                      float * Y20[], float *Y21[], float *Y22[],
                      float *output[]);
};

#endif //HEADER_SPHERICAL_HARMONICS_HPP
