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


#include "graphics/skybox.hpp"
#include "graphics/IBL.hpp"

#include <algorithm> 

using namespace irr;


void Skybox::generateDiffuseCoefficients(video::IVideoDriver *video_driver,
                                         const std::vector<video::ITexture *> &spherical_harmonics_textures,
                                         const video::SColor &ambient)
{
    const unsigned texture_permutation[] = { 2, 3, 0, 1, 5, 4 };

    unsigned sh_w = 0, sh_h = 0;
    unsigned char *sh_rgba[6];

    if (spherical_harmonics_textures.size() == 6)
    {

        for (unsigned i = 0; i < 6; i++)
        {
            sh_w = std::max(sh_w, spherical_harmonics_textures[i]->getSize().Width);
            sh_h = std::max(sh_h, spherical_harmonics_textures[i]->getSize().Height);
        }

        for (unsigned i = 0; i < 6; i++)
            sh_rgba[i] = new unsigned char[sh_w * sh_h * 4];
        for (unsigned i = 0; i < 6; i++)
        {
            unsigned idx = texture_permutation[i];

            video::IImage* image = video_driver->createImageFromData(
                spherical_harmonics_textures[idx]->getColorFormat(),
                spherical_harmonics_textures[idx]->getSize(),
                spherical_harmonics_textures[idx]->lock(),
                false
                );
            spherical_harmonics_textures[idx]->unlock();

            image->copyToScaling(sh_rgba[i], sh_w, sh_h);
            delete image;
        }

    }
    else
    {
        sh_w = 16;
        sh_h = 16;

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
    }

    // Convert to float texture
    Color *FloatTexCube[6];
    for (unsigned i = 0; i < 6; i++)
    {
        FloatTexCube[i] = new Color[sh_w * sh_h];
        for (unsigned j = 0; j < sh_w * sh_h; j++)
        {
            FloatTexCube[i][j].Blue = powf(float(0xFF & sh_rgba[i][4 * j]) / 255.f, 2.2f);
            FloatTexCube[i][j].Green = powf(float(0xFF & sh_rgba[i][4 * j + 1]) / 255.f, 2.2f);
            FloatTexCube[i][j].Red = powf(float(0xFF & sh_rgba[i][4 * j + 2]) / 255.f, 2.2f);
        }
    }

    generateSphericalHarmonics(FloatTexCube, sh_w, m_blue_SH_coeff, m_green_SH_coeff, m_red_SH_coeff);

    for (unsigned i = 0; i < 6; i++)
    {
        delete[] sh_rgba[i];
        delete[] FloatTexCube[i];
    }

    if (spherical_harmonics_textures.size() != 6)
    {
        // Diffuse env map is x 0.25, compensate
        for (unsigned i = 0; i < 9; i++)
        {
            m_blue_SH_coeff[i] *= 4;
            m_green_SH_coeff[i] *= 4;
            m_red_SH_coeff[i] *= 4;
        }
    }
}   // generateDiffuseCoefficients


GLuint Skybox::generateCubeMapFromTextures(const std::vector<video::ITexture *> &skybox_textures)
{
    //TODO
    
    
    
    
}


Skybox::Skybox(video::IVideoDriver *video_driver,
               const std::vector<video::ITexture *> &skybox_textures,
               const std::vector<video::ITexture *> &spherical_harmonics_textures,
               const video::SColor &ambient)
{
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    generateDiffuseCoefficients(video_driver, spherical_harmonics_textures, ambient);
    if (!skybox_textures.empty())
    {
        m_cube_map = generateCubeMapFromTextures(skybox_textures);
        //m_specular_probe = generateSpecularCubemap(m_cube_map);
    }    
}

Skybox::~Skybox()
{
    //TODOskybox
}



