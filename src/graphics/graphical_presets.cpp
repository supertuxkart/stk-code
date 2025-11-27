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

#include "graphics/graphical_presets.hpp"

namespace GraphicalPresets
{
    bool init_done = false;
    std::vector<GFXPreset> gfx_presets;
    std::vector<BlurPreset> blur_presets;
    std::vector<ScaleRttsCustomPreset> scale_rtts_presets;

    // --------------------------------------------------------------------------------------------
    void initPresets()
    {
        if (init_done)
            return;

        gfx_presets.push_back // Level 1
        ({
            false /* light */, 0 /* shadow */, false /* bloom */, false /* lightshaft */,
            false /* glow */, false /* mlaa */, false /* ssao */, false /* light scatter */,
            false /* animatedCharacters */, 1 /* particles */, 0 /* image_quality */,
            true /* degraded IBL */, 0 /* Geometry Detail */, false /* PCSS */, false /* ssr */
        });
    
        gfx_presets.push_back // Level 2
        ({
            false /* light */, 0 /* shadow */, false /* bloom */, false /* lightshaft */,
            false /* glow */, false /* mlaa */, false /* ssao */, false /* light scatter */,
            true /* animatedCharacters */, 2 /* particles */, 1 /* image_quality */,
            true /* degraded IBL */, 1 /* Geometry Detail */, false /* PCSS */, false /* ssr */
        });
    
        gfx_presets.push_back // Level 3
        ({
            true /* light */, 0 /* shadow */, false /* bloom */, false /* lightshaft */,
            false /* glow */, false /* mlaa */, false /* ssao */, false /* light scatter */,
            true /* animatedCharacters */, 2 /* particles */, 2 /* image_quality */,
            true /* degraded IBL */, 2 /* Geometry Detail */, false /* PCSS */, false /* ssr */
        });
    
        gfx_presets.push_back // Level 4
        ({
            true /* light */, 0 /* shadow */, false /* bloom */, false /* lightshaft */,
            true /* glow */, true /* mlaa */, false /* ssao */, true /* light scatter */,
            true /* animatedCharacters */, 2 /* particles */, 2 /* image_quality */,
            true /* degraded IBL */, 2 /* Geometry Detail */, false /* PCSS */, true /* ssr */
        });
    
        gfx_presets.push_back // Level 5
        ({
            true /* light */, 512 /* shadow */, true /* bloom */, true /* lightshaft */,
            true /* glow */, true /* mlaa */, false /* ssao */, true /* light scatter */,
            true /* animatedCharacters */, 2 /* particles */, 3 /* image_quality */,
            false /* degraded IBL */, 3 /* Geometry Detail */, false /* PCSS */, true /* ssr */
        });
    
        gfx_presets.push_back // Level 6
        ({
            true /* light */, 1024 /* shadow */, true /* bloom */, true /* lightshaft */,
            true /* glow */, true /* mlaa */, true /* ssao */, true /* light scatter */,
            true /* animatedCharacters */, 2 /* particles */, 3 /* image_quality */,
            false /* degraded IBL */, 4 /* Geometry Detail */, false /* PCSS */, true /* ssr */
        });
    
        gfx_presets.push_back // Level 7
        ({
            true /* light */, 2048 /* shadow */, true /* bloom */, true /* lightshaft */,
            true /* glow */, true /* mlaa */, true /* ssao */, true /* light scatter */,
            true /* animatedCharacters */, 2 /* particles */, 3 /* image_quality */,
            false /* degraded IBL */, 5 /* Geometry Detail */, true /* PCSS */, true /* ssr */
        });
    
        blur_presets.push_back // Level 0
        ({
            false /* motionblur */, false /* depth of field */
        });
    
        blur_presets.push_back // Level 1
        ({
            true  /* motionblur */, false /* depth of field */
        });
    
        blur_presets.push_back // Level 2
        ({
            true  /* motionblur */, true  /* depth of field */
        });
    
        scale_rtts_presets.push_back({ 0.3f });
        scale_rtts_presets.push_back({ 0.35f });
        scale_rtts_presets.push_back({ 0.4f });
        scale_rtts_presets.push_back({ 0.45f });
        scale_rtts_presets.push_back({ 0.5f });
        scale_rtts_presets.push_back({ 0.55f });
        scale_rtts_presets.push_back({ 0.6f });
        scale_rtts_presets.push_back({ 0.65f });
        scale_rtts_presets.push_back({ 0.7f });
        scale_rtts_presets.push_back({ 0.75f });
        scale_rtts_presets.push_back({ 0.8f });
        scale_rtts_presets.push_back({ 0.85f });
        scale_rtts_presets.push_back({ 0.9f });
        scale_rtts_presets.push_back({ 0.95f });
        scale_rtts_presets.push_back({ 1.0f });
        scale_rtts_presets.push_back({ 1.25f });
        scale_rtts_presets.push_back({ 1.5f });
        scale_rtts_presets.push_back({ 2.0f });

        init_done = true;
    }   // initPresets
}   // GraphicalPresets