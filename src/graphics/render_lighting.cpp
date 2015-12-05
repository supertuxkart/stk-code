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


#include "config/user_config.hpp"
#include "graphics/callbacks.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/light.hpp"
#include "graphics/post_processing.hpp"
#include "graphics/rtts.hpp"
#include "graphics/shaders.hpp"
#include "graphics/shadow_matrices.hpp"
#include "graphics/shared_gpu_objects.hpp"
#include "modes/world.hpp"
#include "tracks/track.hpp"
#include "utils/profiler.hpp"

#define MAX2(a, b) ((a) > (b) ? (a) : (b))
#define MIN2(a, b) ((a) > (b) ? (b) : (a))







// ----------------------------------------------------------------------------
/** Upload lighting info to the dedicated uniform buffer
 */
void IrrDriver::uploadLightingData()
{
    float Lighting[36];
    Lighting[0] = m_sun_direction.X;
    Lighting[1] = m_sun_direction.Y;
    Lighting[2] = m_sun_direction.Z;
    Lighting[4] = m_suncolor.getRed();
    Lighting[5] = m_suncolor.getGreen();
    Lighting[6] = m_suncolor.getBlue();
    Lighting[7] = 0.54f;

    const SHCoefficients* sh_coeff = irr_driver->getSHCoefficients();

    if(sh_coeff) {
        memcpy(&Lighting[8], sh_coeff->blue_SH_coeff, 9 * sizeof(float));
        memcpy(&Lighting[17], sh_coeff->green_SH_coeff, 9 * sizeof(float));
        memcpy(&Lighting[26], sh_coeff->red_SH_coeff, 9 * sizeof(float));
    }

    glBindBuffer(GL_UNIFORM_BUFFER, SharedGPUObjects::getLightingDataUBO());
    glBufferSubData(GL_UNIFORM_BUFFER, 0, 36 * sizeof(float), Lighting);
}   // uploadLightingData


// ----------------------------------------------------------------------------
void IrrDriver::renderSSAO()
{
    m_rtts->getFBO(FBO_SSAO).bind();
    glClearColor(1., 1., 1., 1.);
    glClear(GL_COLOR_BUFFER_BIT);
    m_post_processing->renderSSAO();
    // Blur it to reduce noise.
    FrameBuffer::Blit(m_rtts->getFBO(FBO_SSAO),
                      m_rtts->getFBO(FBO_HALF1_R), 
                      GL_COLOR_BUFFER_BIT, GL_LINEAR);
    m_post_processing->renderGaussian17TapBlur(irr_driver->getFBO(FBO_HALF1_R), 
                                               irr_driver->getFBO(FBO_HALF2_R));

}   // renderSSAO


