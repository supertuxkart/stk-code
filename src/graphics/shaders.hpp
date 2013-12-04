//  SuperTuxKart - a fun racing game with go-kart
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

#ifndef HEADER_SHADERS_HPP
#define HEADER_SHADERS_HPP

#include <IShaderConstantSetCallBack.h>
#include <IMeshSceneNode.h>
#include <vector>
using namespace irr;

#define FOREACH_SHADER(ACT) \
    ACT(ES_NORMAL_MAP) \
    ACT(ES_NORMAL_MAP_LIGHTMAP) \
    ACT(ES_SPLATTING) \
    ACT(ES_WATER) \
    ACT(ES_WATER_SURFACE) \
    ACT(ES_SPHERE_MAP) \
    ACT(ES_GRASS) \
    ACT(ES_GRASS_REF) \
    ACT(ES_BUBBLES) \
    ACT(ES_RAIN) \
    ACT(ES_SNOW) \
    ACT(ES_MOTIONBLUR) \
    ACT(ES_GAUSSIAN3H) \
    ACT(ES_GAUSSIAN3V) \
    ACT(ES_MIPVIZ) \
    ACT(ES_FLIP) \
    ACT(ES_FLIP_ADDITIVE) \
    ACT(ES_COLOR_LEVELS) \
    ACT(ES_BLOOM) \
    ACT(ES_GAUSSIAN6H) \
    ACT(ES_GAUSSIAN6V) \
    ACT(ES_COLORIZE) \
    ACT(ES_COLORIZE_REF) \
    ACT(ES_PASS) \
    ACT(ES_PASS_ADDITIVE) \
    ACT(ES_GLOW) \
    ACT(ES_OBJECTPASS) \
    ACT(ES_OBJECTPASS_REF) \
    ACT(ES_LIGHTBLEND) \
    ACT(ES_POINTLIGHT) \
    ACT(ES_SUNLIGHT) \
    ACT(ES_SUNLIGHT_SHADOW) \
    ACT(ES_OBJECTPASS_RIMLIT) \
    ACT(ES_MLAA_COLOR1) \
    ACT(ES_MLAA_BLEND2) \
    ACT(ES_MLAA_NEIGH3) \
    ACT(ES_SSAO) \
    ACT(ES_GODFADE) \
    ACT(ES_GODRAY) \
    ACT(ES_SHADOWPASS) \
    ACT(ES_SHADOW_IMPORTANCE) \
    ACT(ES_COLLAPSE) \
    ACT(ES_SHADOW_WARPH) \
    ACT(ES_SHADOW_WARPV) \
    ACT(ES_BLOOM_POWER) \
    ACT(ES_BLOOM_BLEND) \
    ACT(ES_MULTIPLY_ADD) \
    ACT(ES_PENUMBRAH) \
    ACT(ES_PENUMBRAV) \
    ACT(ES_SHADOWGEN) \
    ACT(ES_CAUSTICS) \
    ACT(ES_DISPLACE) \
    ACT(ES_PPDISPLACE) \
    ACT(ES_PASSFAR) \
    ACT(ES_FOG)

#define ENUM(a) a,
#define STR(a) #a,

enum ShaderType
{
    FOREACH_SHADER(ENUM)

    ES_COUNT
};

#ifdef SHADER_NAMES
static const char *shader_names[] = {
    FOREACH_SHADER(STR)
};
#endif

class Shaders
{
public:
    Shaders();
    ~Shaders();

    video::E_MATERIAL_TYPE getShader(const ShaderType num) const;

    video::IShaderConstantSetCallBack * m_callbacks[ES_COUNT];

private:
    void check(const int num) const;

    int m_shaders[ES_COUNT];
};

#undef ENUM
#undef STR
#undef FOREACH_SHADER

#endif
