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


#ifndef HEADER_SPHERICAL_HARMONIC_HPP
#define HEADER_SPHERICAL_HARMONIC_HPP

#include <ITexture.h>
#include <vector>

struct Color
{
    float Red;
    float Green;
    float Blue;
};


class SphericalHarmonic
{
private:
    /** The 6 spherical harmonics textures */
    std::vector<irr::video::ITexture *> m_spherical_harmonics_textures;
        
    /** The spherical harmonics coefficients */
    float m_blue_SH_coeff[9];
    float m_green_SH_coeff[9];
    float m_red_SH_coeff[9];    


    void projectSH(Color *cubemap_face[6], size_t edge_size, float *Y00[],
                      float *Y1minus1[], float *Y10[], float *Y11[],
                      float *Y2minus2[], float *Y2minus1[], float * Y20[],
                      float *Y21[], float *Y22[], float *blue_sh_coeff,
                      float *green_sh_coeff, float *red_sh_coeff);
    
    void generateSphericalHarmonics(Color *cubemap_face[6], size_t edge_size,
                                    float *blue_sh_coeff, float *green_sh_coeff,
                                    float *red_sh_coeff);
    
public:
    SphericalHarmonic(const std::vector<irr::video::ITexture *> &spherical_harmonics_textures);
    SphericalHarmonic(const irr::video::SColor &ambient);
    
    inline const float* getBlueSHCoeff () const     {return m_blue_SH_coeff;  }
    inline const float* getGreenSHCoeff() const     {return m_green_SH_coeff; }
    inline const float* getRedSHCoeff  () const     {return m_red_SH_coeff;   }    
    
    /** Print spherical harmonics coefficients (debug) */
    void printCoeff();
    
    void unprojectSH (float *output[], size_t width, size_t height,
                      float *Y00[], float *Y1minus1[], float *Y10[],
                      float *Y11[], float *Y2minus2[], float *Y2minus1[],
                      float * Y20[], float *Y21[], float *Y22[]);
};

#endif //HEADER_SPHERICAL_HARMONIC_HPP
