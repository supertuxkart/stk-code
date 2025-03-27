//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2018 SuperTuxKart-Team
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

#ifndef HEADER_SP_BASE_HPP
#define HEADER_SP_BASE_HPP

#include "graphics/gl_headers.hpp"
#include "utils/constants.hpp"
#include "utils/no_copy.hpp"

#include "irrMath.h"
#include "vector3d.h"

#include <array>
#include <atomic>
#include <cmath>
#include <functional>
#include <ostream>
#include <memory>
#include <string>
#include <vector>

namespace irr
{
    namespace scene { class ICameraSceneNode; class IMesh; }
    namespace video { class SColor; }
}

class ShaderBasedRenderer;

namespace SP
{
class SPMesh;

enum DrawCallType: unsigned int
{
    DCT_NORMAL = 0,
    DCT_SHADOW1,
    DCT_SHADOW2,
    DCT_SHADOW3,
    DCT_SHADOW4,
    DCT_TRANSPARENT,
    DCT_FOR_VAO
};

inline std::ostream& operator<<(std::ostream& os, const DrawCallType& dct)
{
    switch (dct)
    {
        case DCT_NORMAL:
            return os << "normal";
        case DCT_TRANSPARENT:
            return os << "transparent";
        case DCT_SHADOW1:
            return os << "shadow cam 1";
        case DCT_SHADOW2:
            return os << "shadow cam 2";
        case DCT_SHADOW3:
            return os << "shadow cam 3";
        case DCT_SHADOW4:
            return os << "shadow cam 4";
        default:
            return os;
    }
}

enum SamplerType: unsigned int;
enum RenderPass: unsigned int;
class SPDynamicDrawCall;
class SPMaterial;
class SPMeshNode;
class SPShader;
class SPMeshBuffer;

extern GLuint sp_mat_ubo[MAX_PLAYER_COUNT][3];
extern GLuint sp_fog_ubo;
extern std::array<GLuint, 1> sp_prefilled_tex;
extern std::atomic<uint32_t> sp_max_texture_size;
extern unsigned sp_solid_poly_count;
extern unsigned sp_shadow_poly_count;
extern int sp_cur_shadow_cascade;
extern bool sp_culling;
extern bool sp_debug_view;
extern bool sp_apitrace;
extern unsigned sp_cur_player;
extern unsigned sp_cur_buf_id[MAX_PLAYER_COUNT];
extern irr::core::vector3df sp_wind_dir;
// ----------------------------------------------------------------------------
void init();
// ----------------------------------------------------------------------------
void destroy();
// ----------------------------------------------------------------------------
GLuint getSampler(SamplerType);
// ----------------------------------------------------------------------------
SPShader* getNormalVisualizer();
// ----------------------------------------------------------------------------
SPShader* getGlowShader();
// ----------------------------------------------------------------------------
bool skinningUseTBO();
// ----------------------------------------------------------------------------
void prepareDrawCalls();
// ----------------------------------------------------------------------------
void draw(RenderPass, DrawCallType dct = DCT_NORMAL);
// ----------------------------------------------------------------------------
void drawGlow();
// ----------------------------------------------------------------------------
void drawSPDebugView();
// ----------------------------------------------------------------------------
void addObject(SPMeshNode*);
// ----------------------------------------------------------------------------
void initSTKRenderer(ShaderBasedRenderer*);
// ----------------------------------------------------------------------------
void initSamplers();
// ----------------------------------------------------------------------------
void prepareScene();
// ----------------------------------------------------------------------------
void handleDynamicDrawCall();
// ----------------------------------------------------------------------------
void addDynamicDrawCall(std::shared_ptr<SPDynamicDrawCall>);
// ----------------------------------------------------------------------------
void updateModelMatrix();
// ----------------------------------------------------------------------------
void uploadAll();
// ----------------------------------------------------------------------------
void resetEmptyFogColor();
// ----------------------------------------------------------------------------
void drawBoundingBoxes();
// ----------------------------------------------------------------------------
void loadShaders();
// ----------------------------------------------------------------------------
SPMesh* convertEVTStandard(irr::scene::IMesh* mesh,
                           const irr::video::SColor* color = NULL);
// ----------------------------------------------------------------------------
void uploadSPM(irr::scene::IMesh* mesh);
// ----------------------------------------------------------------------------
#ifdef SERVER_ONLY
inline void setMaxTextureSize() {}
#else
void setMaxTextureSize();
#endif
// ----------------------------------------------------------------------------
inline void unsetMaxTextureSize()          { sp_max_texture_size.store(2048); }
// ----------------------------------------------------------------------------
inline uint8_t srgbToLinear(float color_srgb)
{
    int ret;
    if (color_srgb <= 0.04045f)
    {
        ret = (int)(255.0f * (color_srgb / 12.92f));
    }
    else
    {
        ret = (int)(255.0f * (powf((color_srgb + 0.055f) / 1.055f, 2.4f)));
    }
    return uint8_t(irr::core::clamp(ret, 0, 255));
}
// ----------------------------------------------------------------------------
inline uint8_t linearToSrgb(float color_linear)
{
    if (color_linear <= 0.0031308f)
    {
        color_linear = color_linear * 12.92f;
    }
    else
    {
        color_linear = 1.055f * powf(color_linear, 1.0f / 2.4f) - 0.055f;
    }
    return uint8_t(irr::core::clamp(int(color_linear * 255.0f), 0, 255));
}
// ----------------------------------------------------------------------------
ShaderBasedRenderer* getRenderer();
}


#endif
