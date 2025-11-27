//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2025 Alayan & SuperTuxKart team
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

#ifndef __HEADER_GRAPHICAL_PRESETS_HPP__
#define __HEADER_GRAPHICAL_PRESETS_HPP__

#include <vector>

namespace GraphicalPresets
{
    struct GFXPreset
    {
        bool lights;
        int shadows;
        bool bloom;
        bool lightshaft;
        bool glow;
        bool mlaa;
        bool ssao;
        bool light_scatter;
        bool animatedCharacters;
        int particles;
        int image_quality;
        bool degraded_ibl;
        int geometry_detail;
        bool pc_soft_shadows;
        bool ssr;
    };

    struct BlurPreset
    {
        bool motionblur;
        /** Depth of field */
        bool dof;
    };

    struct ScaleRttsCustomPreset
    {
        float value;
    };

    extern bool init_done;
    extern std::vector<GFXPreset> gfx_presets;
    extern std::vector<BlurPreset> blur_presets;
    extern std::vector<ScaleRttsCustomPreset> scale_rtts_presets;

    void initPresets();
    /* The find and apply preset do NOT check for the active renderer! */
    int findCurrentGFXPreset();
    void applyGFXPreset(int preset);
    int getImageQuality();
    void setImageQuality(int quality);
}

#endif