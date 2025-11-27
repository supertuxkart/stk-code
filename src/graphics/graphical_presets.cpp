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

#include "config/user_config.hpp"

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

    // --------------------------------------------------------------------------------------------
    /* This function returns the currently active GFX preset, if any. If custom settings are
    * currently used, it returns -1. It does NOT check the active renderer. */
    int findCurrentGFXPreset()
    {
        for (unsigned int l = 0; l < gfx_presets.size(); l++)
        {
            if (gfx_presets[l].animatedCharacters == UserConfigParams::m_animated_characters &&
                gfx_presets[l].particles          == UserConfigParams::m_particles_effects   &&
                gfx_presets[l].image_quality      == getImageQuality()                       &&
                gfx_presets[l].bloom              == UserConfigParams::m_bloom               &&
                gfx_presets[l].glow               == UserConfigParams::m_glow                &&
                gfx_presets[l].lights             == UserConfigParams::m_dynamic_lights      &&
                gfx_presets[l].lightshaft         == UserConfigParams::m_light_shaft         &&
                gfx_presets[l].mlaa               == UserConfigParams::m_mlaa                &&
                gfx_presets[l].shadows            == UserConfigParams::m_shadows_resolution  &&
                gfx_presets[l].ssao               == UserConfigParams::m_ssao                &&
                gfx_presets[l].light_scatter      == UserConfigParams::m_light_scatter       &&
                gfx_presets[l].degraded_ibl       == UserConfigParams::m_degraded_IBL        &&
                gfx_presets[l].geometry_detail    == UserConfigParams::m_geometry_level      &&
                gfx_presets[l].pc_soft_shadows    == UserConfigParams::m_pcss                &&
                gfx_presets[l].ssr                == UserConfigParams::m_ssr)
            {
                return (l + 1);
            }
        }
        return -1;
    }   // findCurrentGFXPreset

    // --------------------------------------------------------------------------------------------
    void applyGFXPreset(int level)
    {
        level--;
        level = std::min((int)gfx_presets.size(), std::max(0, level));
        UserConfigParams::m_animated_characters = gfx_presets[level].animatedCharacters;
        UserConfigParams::m_particles_effects = gfx_presets[level].particles;
        setImageQuality(gfx_presets[level].image_quality);
        UserConfigParams::m_bloom              = gfx_presets[level].bloom;
        UserConfigParams::m_glow               = gfx_presets[level].glow;
        UserConfigParams::m_dynamic_lights     = gfx_presets[level].lights;
        UserConfigParams::m_light_shaft        = gfx_presets[level].lightshaft;
        UserConfigParams::m_mlaa               = gfx_presets[level].mlaa;
        UserConfigParams::m_shadows_resolution = gfx_presets[level].shadows;
        UserConfigParams::m_ssao               = gfx_presets[level].ssao;
        UserConfigParams::m_light_scatter      = gfx_presets[level].light_scatter;
        UserConfigParams::m_degraded_IBL       = gfx_presets[level].degraded_ibl;
        UserConfigParams::m_geometry_level     = gfx_presets[level].geometry_detail;
        UserConfigParams::m_pcss               = gfx_presets[level].pc_soft_shadows;
        UserConfigParams::m_ssr                = gfx_presets[level].ssr;
    }   // applyGFXPreset

    // --------------------------------------------------------------------------------------------
    int getImageQuality()
    {
        if (UserConfigParams::m_anisotropic == 4 &&
            (UserConfigParams::m_high_definition_textures & 0x01) == 0x00 &&
            UserConfigParams::m_hq_mipmap == false)
            return 0;
        if (UserConfigParams::m_anisotropic == 16 &&
            (UserConfigParams::m_high_definition_textures & 0x01) == 0x00 &&
            UserConfigParams::m_hq_mipmap == false)
            return 1;
        if (UserConfigParams::m_anisotropic == 16 &&
            (UserConfigParams::m_high_definition_textures & 0x01) == 0x01 &&
            UserConfigParams::m_hq_mipmap == false)
            return 2;
        if (UserConfigParams::m_anisotropic == 16 &&
            (UserConfigParams::m_high_definition_textures & 0x01) == 0x01 &&
            UserConfigParams::m_hq_mipmap == true)
            return 3;
        return 1;
    }   // getImageQuality

    // --------------------------------------------------------------------------------------------
    /* This function updates the user config parameters to a new config level.
    * It does NOT ensure that the new parameters are properly applied. */
    void setImageQuality(int quality)
    {
        assert (quality >= 0 && quality <= 3);
        UserConfigParams::m_anisotropic              = (quality >= 1) ? 16   : 4;
        UserConfigParams::m_high_definition_textures = (quality >= 2) ? 0x03 : 0x02;
        UserConfigParams::m_hq_mipmap                = (quality >= 3) ? true : false;
    }   // setImageQuality
}   // GraphicalPresets