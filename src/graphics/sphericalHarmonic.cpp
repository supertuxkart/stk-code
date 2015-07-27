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


#include "graphics/IBL.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/sphericalHarmonic.hpp"

#include <algorithm> 
#include <cassert>

using namespace irr;

namespace 
{ 
    void convertToFloatTexture(unsigned char *sh_rgba[6], unsigned sh_w, unsigned sh_h, Color *float_tex_cube[6]) {
        for (unsigned i = 0; i < 6; i++)
        {
            float_tex_cube[i] = new Color[sh_w * sh_h];
            for (unsigned j = 0; j < sh_w * sh_h; j++)
            {
                float_tex_cube[i][j].Blue = powf(float(0xFF & sh_rgba[i][4 * j]) / 255.f, 2.2f);
                float_tex_cube[i][j].Green = powf(float(0xFF & sh_rgba[i][4 * j + 1]) / 255.f, 2.2f);
                float_tex_cube[i][j].Red = powf(float(0xFF & sh_rgba[i][4 * j + 2]) / 255.f, 2.2f);
            }
        }    
    } //convertToFloatTexture

}

void SphericalHarmonic::printCoeff() {
    Log::debug("Skybox", "Blue_SH: %f, %f, %f, %f, %f, %f, %f, %f, %f",
                m_blue_SH_coeff[0], m_blue_SH_coeff[1], m_blue_SH_coeff[2],
                m_blue_SH_coeff[3], m_blue_SH_coeff[4], m_blue_SH_coeff[5],
                m_blue_SH_coeff[6], m_blue_SH_coeff[7], m_blue_SH_coeff[8]);
    Log::debug("Skybox", "Green_SH: %f, %f, %f, %f, %f, %f, %f, %f, %f",
                m_green_SH_coeff[0], m_green_SH_coeff[1], m_green_SH_coeff[2],
                m_green_SH_coeff[3], m_green_SH_coeff[4], m_green_SH_coeff[5],
                m_green_SH_coeff[6], m_green_SH_coeff[7], m_green_SH_coeff[8]);
    Log::debug("Skybox", "Red_SH: %f, %f, %f, %f, %f, %f, %f, %f, %f",
                m_red_SH_coeff[0], m_red_SH_coeff[1], m_red_SH_coeff[2],
                m_red_SH_coeff[3], m_red_SH_coeff[4], m_red_SH_coeff[5],
                m_red_SH_coeff[6], m_red_SH_coeff[7], m_red_SH_coeff[8]);    
}

SphericalHarmonic::SphericalHarmonic(const std::vector<video::ITexture *> &spherical_harmonics_textures)
{
    assert(spherical_harmonics_textures.size() == 6);
    
    m_spherical_harmonics_textures = spherical_harmonics_textures;

    const unsigned texture_permutation[] = { 2, 3, 0, 1, 5, 4 };
    unsigned char *sh_rgba[6];
    unsigned sh_w = 0, sh_h = 0;

    for (unsigned i = 0; i < 6; i++)
    {
        sh_w = std::max(sh_w, m_spherical_harmonics_textures[i]->getSize().Width);
        sh_h = std::max(sh_h, m_spherical_harmonics_textures[i]->getSize().Height);
    }

    for (unsigned i = 0; i < 6; i++)
        sh_rgba[i] = new unsigned char[sh_w * sh_h * 4];
    
    for (unsigned i = 0; i < 6; i++)
    {
        unsigned idx = texture_permutation[i];

        video::IImage* image = irr_driver->getVideoDriver()->createImageFromData(
            m_spherical_harmonics_textures[idx]->getColorFormat(),
            m_spherical_harmonics_textures[idx]->getSize(),
            m_spherical_harmonics_textures[idx]->lock(),
            false
            );
        m_spherical_harmonics_textures[idx]->unlock();

        image->copyToScaling(sh_rgba[i], sh_w, sh_h);
        delete image;
    } //for (unsigned i = 0; i < 6; i++)

    Color *float_tex_cube[6];
    convertToFloatTexture(sh_rgba, sh_w, sh_h, float_tex_cube);
    generateSphericalHarmonics(float_tex_cube, sh_w, m_blue_SH_coeff, m_green_SH_coeff, m_red_SH_coeff);

    for (unsigned i = 0; i < 6; i++)
    {
        delete[] sh_rgba[i];
        delete[] float_tex_cube[i];
    }

    // Diffuse env map is x 0.25, compensate
    for (unsigned i = 0; i < 9; i++)
    {
        m_blue_SH_coeff[i] *= 4;
        m_green_SH_coeff[i] *= 4;
        m_red_SH_coeff[i] *= 4;
    }
    
}// SphericalHarmonic(const std::vector<video::ITexture *> &spherical_harmonics_textures)

SphericalHarmonic::SphericalHarmonic(const video::SColor &ambient)
{
    unsigned char *sh_rgba[6];
    unsigned sh_w = 16;
    unsigned sh_h = 16;

    for (unsigned i = 0; i < 6; i++)
    {
        sh_rgba[i] = new unsigned char[sh_w * sh_h * 4];

        for (unsigned j = 0; j < sh_w * sh_h * 4; j += 4)
        {
            sh_rgba[i][j] = ambient.getBlue();
            sh_rgba[i][j + 1] = ambient.getGreen();
            sh_rgba[i][j + 2] = ambient.getRed();
            sh_rgba[i][j + 3] = 255;
        }
    }    

    Color *float_tex_cube[6];
    convertToFloatTexture(sh_rgba, sh_w, sh_h, float_tex_cube);
    generateSphericalHarmonics(float_tex_cube, sh_w, m_blue_SH_coeff, m_green_SH_coeff, m_red_SH_coeff);

    for (unsigned i = 0; i < 6; i++)
    {
        delete[] sh_rgba[i];
        delete[] float_tex_cube[i];
    }
    
}// SphericalHarmonic(const video::SColor &ambient)



