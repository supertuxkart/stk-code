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

struct SHCoefficients
{
    float blue_SH_coeff[9];
    float green_SH_coeff[9];
    float red_SH_coeff[9];
};


class SphericalHarmonics
{
private:
    /** The 6 spherical harmonics textures */
    std::vector<irr::video::IImage *> m_spherical_harmonics_textures;
    
    /** Ambient light is used for tracks without spherical harmonics textures */
    irr::video::SColor m_ambient;
    
    /** The spherical harmonics coefficients */
    SHCoefficients *m_SH_coeff;

    void generateSphericalHarmonics(unsigned char *sh_rgba[6], unsigned int edge_size);
    
public:
    SphericalHarmonics(const std::vector<irr::video::IImage *> &spherical_harmonics_textures);
    SphericalHarmonics(const irr::video::SColor &ambient);
    ~SphericalHarmonics();
    
    void setTextures(const std::vector<irr::video::IImage *> &spherical_harmonics_textures);
    void setAmbientLight(const irr::video::SColor &ambient);

    inline const SHCoefficients* getCoefficients() const { return m_SH_coeff;  }
    
    inline bool has6Textures() const {return m_spherical_harmonics_textures.size()==6;}
    
    void printCoeff();
    
    void unprojectSH (unsigned int width, unsigned int height,
                      float *Y00[], float *Y1minus1[], float *Y10[],
                      float *Y11[], float *Y2minus2[], float *Y2minus1[],
                      float * Y20[], float *Y21[], float *Y22[],
                      float *output[]);
};

#endif //HEADER_SPHERICAL_HARMONICS_HPP
