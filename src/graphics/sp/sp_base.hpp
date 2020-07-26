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
inline uint8_t srgb255ToLinear(unsigned color_srgb_255)
{
    static unsigned srgb_linear_map[256] =
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2,
        2, 2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4,
        4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7,
        7, 8, 8, 8, 9, 9, 9, 10, 10, 10, 11, 11,
        11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
        16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 22, 22,
        23, 23, 24, 24, 25, 26, 26, 27, 27, 28, 29, 29,
        30, 31, 31, 32, 33, 33, 34, 35, 36, 36, 37, 38,
        38, 39, 40, 41, 42, 42, 43, 44, 45, 46, 47, 47,
        48, 49, 50, 51, 52, 53, 54, 55, 55, 56, 57, 58,
        59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 70, 71,
        72, 73, 74, 75, 76, 77, 78, 80, 81, 82, 83, 84,
        85, 87, 88, 89, 90, 92, 93, 94, 95, 97, 98, 99,
        101, 102, 103, 105, 106, 107, 109, 110, 112, 113, 114, 116,
        117, 119, 120, 122, 123, 125, 126, 128, 129, 131, 132, 134,
        135, 137, 139, 140, 142, 144, 145, 147, 148, 150, 152, 153,
        155, 157, 159, 160, 162, 164, 166, 167, 169, 171, 173, 175,
        176, 178, 180, 182, 184, 186, 188, 190, 192, 193, 195, 197,
        199, 201, 203, 205, 207, 209, 211, 213, 215, 218, 220, 222,
        224, 226, 228, 230, 232, 235, 237, 239, 241, 243, 245, 248,
        250, 252, 255
    };
    return uint8_t(srgb_linear_map[color_srgb_255]);
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
