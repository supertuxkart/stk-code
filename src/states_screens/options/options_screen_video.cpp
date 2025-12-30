//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015 Marianne Gagnon
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

#ifndef SERVER_ONLY

// Manages includes common to all options screens
#include "states_screens/options/options_common.hpp"

#include "graphics/central_settings.hpp"
#include "graphics/graphical_presets.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/shader.hpp"
#include "graphics/sp/sp_base.hpp"
#include "graphics/sp/sp_texture_manager.hpp"
#include "graphics/stk_tex_manager.hpp"
#include "io/file_manager.hpp"
#include "states_screens/dialogs/custom_video_settings.hpp"
#include "states_screens/dialogs/recommend_video_settings.hpp"
#include "utils/profiler.hpp"


#include <ge_main.hpp>
#include <ge_vulkan_driver.hpp>
#include <ge_vulkan_texture_descriptor.hpp>


#include <IrrlichtDevice.h>

using namespace GUIEngine;
using namespace GraphicalPresets;

// --------------------------------------------------------------------------------------------
void OptionsScreenVideo::updateImageQuality(bool force_reload_texture)
{
    core::dimension2du prev_max_size = irr_driver->getVideoDriver()
        ->getDriverAttributes().getAttributeAsDimension2d("MAX_TEXTURE_SIZE");
    GE::GEVulkanTextureDescriptor* td = NULL;
    if (GE::getVKDriver())
        td = GE::getVKDriver()->getMeshTextureDescriptor();

    if (td)
    {
        if (UserConfigParams::m_anisotropic == 4)
            td->setSamplerUse(GE::GVS_3D_MESH_MIPMAP_4);
        if (UserConfigParams::m_anisotropic == 16)
            td->setSamplerUse(GE::GVS_3D_MESH_MIPMAP_16);
    }

    irr_driver->setMaxTextureSize();
    SP::setMaxTextureSize();
    core::dimension2du cur_max_size = irr_driver->getVideoDriver()
        ->getDriverAttributes().getAttributeAsDimension2d("MAX_TEXTURE_SIZE");

    if (CVS->isGLSL())
    {
        ShaderBase::killShaders();
        SP::initSamplers();
        if (prev_max_size != cur_max_size || force_reload_texture)
            SP::SPTextureManager::get()->reloadTexture("");
    }
    else if (prev_max_size != cur_max_size || force_reload_texture)
        STKTexManager::getInstance()->reloadAllTextures(true/*mesh_texture_only*/);
}   // updateImageQuality

// --------------------------------------------------------------------------------------------
OptionsScreenVideo::OptionsScreenVideo() : Screen("options/options_video.stkgui"),
                                           m_prev_adv_pipline(false)
{
}   // OptionsScreenVideo

// --------------------------------------------------------------------------------------------
void OptionsScreenVideo::loadedFromFile()
{
    assert(gfx_presets.size() == 7);
    assert(blur_presets.size() == 3);

    // Note: m_widgets.bind() is called later in init(), so use getWidget here
    GUIEngine::SpinnerWidget* gfx =
        getWidget<GUIEngine::SpinnerWidget>("gfx_level");
    gfx->m_properties[GUIEngine::PROP_MAX_VALUE] =
        StringUtils::toString(gfx_presets.size());

    GUIEngine::SpinnerWidget* blur =
        getWidget<GUIEngine::SpinnerWidget>("blur_level");
    blur->m_properties[GUIEngine::PROP_MAX_VALUE] =
        StringUtils::toString(blur_presets.size() - 1);
    blur->m_properties[GUIEngine::PROP_MIN_VALUE] =
        StringUtils::toString(0);
}   // loadedFromFile

// --------------------------------------------------------------------------------------------
void OptionsScreenVideo::init()
{
    Screen::init();
    OptionsCommon::setTabStatus();

    // Bind typed widget pointers (one-time lookup)
    m_widgets.bind(this);

    m_prev_adv_pipline = UserConfigParams::m_dynamic_lights;
    m_widgets.options_choice->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    m_widgets.options_choice->select( "tab_video", PLAYER_ID_GAME_MASTER );

    m_widgets.vsync->clearLabels();
    //I18N: In the video options
    m_widgets.vsync->addLabel(_("Vertical Sync"));
#ifdef MOBILE_STK
    std::set<int> fps = { 30, 60, 120 };
#else
    std::set<int> fps = { 30, 60, 120, 180, 250, 500, 1000 };
#endif
    fps.insert(UserConfigParams::m_max_fps);
    for (auto& i : fps)
        m_widgets.vsync->addLabel(core::stringw(i));
    if (UserConfigParams::m_swap_interval > 1)
        UserConfigParams::m_swap_interval = 1;

    if (UserConfigParams::m_swap_interval == 1)
        m_widgets.vsync->setValue(0);
    else
    {
        auto it = fps.find(UserConfigParams::m_max_fps);
        assert(it != fps.end());
        m_widgets.vsync->setValue(1 + std::distance(fps.begin(), it));
    }
    //I18N: in the graphical options. The \n is a newline character, place it where appropriate, two can be used if required.
    core::stringw vsync_tooltip = _("Vsync forces the graphics card to supply a new frame\nonly when the monitor is ready to display it.");

    //I18N: in the graphical options.
    vsync_tooltip = vsync_tooltip + L"\n" + _("Vsync will not work if your drivers don't support it.");

    m_widgets.vsync->setTooltip(vsync_tooltip);

    // Setup Render Resolution (scale_rtts) spinner
    m_widgets.scale_rtts->clearLabels();
    m_widgets.scale_rtts->addLabel("30%");
    m_widgets.scale_rtts->addLabel("35%");
    m_widgets.scale_rtts->addLabel("40%");
    m_widgets.scale_rtts->addLabel("45%");
    m_widgets.scale_rtts->addLabel("50%");
    m_widgets.scale_rtts->addLabel("55%");
    m_widgets.scale_rtts->addLabel("60%");
    m_widgets.scale_rtts->addLabel("65%");
    m_widgets.scale_rtts->addLabel("70%");
    m_widgets.scale_rtts->addLabel("75%");
    m_widgets.scale_rtts->addLabel("80%");
    m_widgets.scale_rtts->addLabel("85%");
    m_widgets.scale_rtts->addLabel("90%");
    m_widgets.scale_rtts->addLabel("95%");
    m_widgets.scale_rtts->addLabel("100%");
    m_widgets.scale_rtts->addLabel("125%");
    m_widgets.scale_rtts->addLabel("150%");
    m_widgets.scale_rtts->addLabel("200%");

    // --- set gfx settings values
    updateGfxSlider();
    updateBlurSlider();
    updateScaleRTTsSlider();

    // ---- forbid changing graphic settings from in-game
    // (we need to disable them last because some items can't be edited when
    // disabled)
    bool in_game = StateManager::get()->getGameState() == GUIEngine::INGAME_MENU;

    m_widgets.gfx_level->setActive(!in_game && CVS->isGLSL());
    m_widgets.custom->setActive(!in_game || !CVS->isGLSL());
    if (m_widgets.scale_rtts->isActivated())
    {
        m_widgets.scale_rtts->setActive(!in_game ||
            GE::getDriver()->getDriverType() == video::EDT_VULKAN);
    }
    m_widgets.benchmarkCurrent->setActive(!in_game);

    // If a benchmark was requested and the game had to reload
    // the graphics engine, start the benchmark when the
    // video settings screen is loaded back afterwards.
    if (RaceManager::get()->isBenchmarkScheduled())
        profiler.startBenchmark();
}   // init

// --------------------------------------------------------------------------------------------
void OptionsScreenVideo::onResize()
{
    Screen::onResize();
}   // onResize

// --------------------------------------------------------------------------------------------
void OptionsScreenVideo::updateGfxSlider()
{
    int preset = findCurrentGFXPreset();
    if (preset == -1) // Current settings don't match a preset
    {
        //I18N: custom video settings
        m_widgets.gfx_level->setCustomText( _("Custom") );
    }
    else
    {
        m_widgets.gfx_level->setValue(preset);
    }

    // Enable the blur slider if the modern renderer is used
    m_widgets.blur_level->setActive(UserConfigParams::m_dynamic_lights && CVS->isGLSL());
    // Same with Render resolution slider
    updateScaleRTTsSlider();

    updateTooltip();
} // updateGfxSlider

// --------------------------------------------------------------------------------------------
void OptionsScreenVideo::updateBlurSlider()
{
    bool found = false;
    for (unsigned int l = 0; l < blur_presets.size(); l++)
    {
        if (blur_presets[l].motionblur == UserConfigParams::m_motionblur &&
            blur_presets[l].dof == UserConfigParams::m_dof)
        {
            m_widgets.blur_level->setValue(l);
            found = true;
            break;
        }
    }

    if (!found)
    {
        //I18N: custom video settings
        m_widgets.blur_level->setCustomText( _("Custom") );
    }

    updateBlurTooltip();
} // updateBlurSlider

// --------------------------------------------------------------------------------------------
void OptionsScreenVideo::updateScaleRTTsSlider()
{
    bool rtts_on = (UserConfigParams::m_dynamic_lights && CVS->isGLSL()) ||
        GE::getDriver()->getDriverType() == video::EDT_VULKAN;

    m_widgets.scale_rtts->setActive(rtts_on);

    bool found = false;
    float rtts_value = (rtts_on) ? UserConfigParams::m_scale_rtts_factor : 1.0f;
    for (unsigned int l = 0; l < scale_rtts_presets.size(); l++)
    {
        if (scale_rtts_presets[l].value == rtts_value)
        {
            m_widgets.scale_rtts->setValue(l);
            found = true;
            if (scale_rtts_presets[l].value > 1.0f)
                m_widgets.scale_rtts->markAsIncorrect();
            else
                m_widgets.scale_rtts->markAsCorrect();

            break;
        }
    }

    if (!found)
    {
        //I18N: custom video settings
        m_widgets.scale_rtts->setCustomText( _("Custom") );
    }
} // updateScaleRTTsSlider

// --------------------------------------------------------------------------------------------
void OptionsScreenVideo::updateTooltip()
{
    core::stringw tooltip;

    //I18N: in the graphical options
    tooltip = UserConfigParams::m_dynamic_lights ? _("Dynamic lights: Enabled") :
                                                   _("Dynamic lights: Disabled");
    //I18N: in the graphical options
    if (UserConfigParams::m_shadows_resolution == 0)
    {
        tooltip = tooltip + L"\n" + _("Shadows: %s", _C("Shadows", "Disabled"));
        tooltip = tooltip + L"\n" + _("Soft shadows: Disabled");
    }
    else
    {
        tooltip = tooltip + L"\n" + _("Shadows: %i", UserConfigParams::m_shadows_resolution);
        tooltip = tooltip + L"\n" + 
            (UserConfigParams::m_pcss ?  _("Soft shadows: Enabled") :
                                         _("Soft shadows: Disabled"));
    }

    //I18N: in the graphical options
    tooltip = tooltip + L"\n" + 
        (UserConfigParams::m_mlaa ? _("Anti-aliasing: Enabled") :
                                    _("Anti-aliasing: Disabled"));
    //I18N: in the graphical options
    tooltip = tooltip + L"\n" +
        (!UserConfigParams::m_degraded_IBL ? _("Image-based lighting: Enabled") :
                                             _("Image-based lighting: Disabled"));
    //I18N: in the graphical options
    tooltip = tooltip + L"\n" +
        (UserConfigParams::m_light_scatter ? _("Light scattering: Enabled") :
                                             _("Light scattering: Disabled"));
    //I18N: in the graphical options
    tooltip = tooltip + L"\n" +
        (UserConfigParams::m_glow ? _("Glow (outlines): Enabled") :
                                    _("Glow (outlines): Disabled"));
    //I18N: in the graphical options
    tooltip = tooltip + L"\n" +
        (UserConfigParams::m_light_shaft ? _("Light shaft (God rays): Enabled") :
                                           _("Light shaft (God rays): Disabled"));
    //I18N: in the graphical options
    tooltip = tooltip + L"\n" +
        (UserConfigParams::m_bloom ? _("Bloom: Enabled") :
                                     _("Bloom: Disabled"));
    //I18N: in the graphical options
    tooltip = tooltip + L"\n" +
        (UserConfigParams::m_ssao ? _("Ambient occlusion: Enabled") :
                                    _("Ambient occlusion: Disabled"));
    tooltip = tooltip + L"\n" +
        (UserConfigParams::m_ssr ? _("Screen space reflection: Enabled") :
                                   _("Screen space reflection: Disabled"));
    //I18N: in the graphical options
    tooltip = tooltip + L"\n" +
        (UserConfigParams::m_animated_characters ? _("Animated characters: Enabled") :
                                                   _("Animated characters: Disabled"));
    //I18N: in the graphical options
    tooltip = tooltip + L"\n" + _("Particle effects: %s",
        UserConfigParams::m_particles_effects == 2 ? _C("Particle effects", "Enabled")        :
        UserConfigParams::m_particles_effects == 1 ? _C("Particle effects", "Important only") :
                                                     _C("Particle effects", "Disabled"));

    //I18N: in the graphical options
    int quality = getImageQuality();
    tooltip = tooltip + L"\n" + _("Rendered image quality: %s",
        quality == 0 ? _C("Image quality", "Very low") :
        quality == 1 ? _C("Image quality", "Low")      :
                       _C("Image quality", "High"));

    //I18N: in the graphical options
    int geometry_detail = UserConfigParams::m_geometry_level;
    tooltip = tooltip + L"\n" + _("Geometry detail: %s",
        geometry_detail == 0 ?  _C("Geometry level", "Very low")  :
        geometry_detail == 1 ?  _C("Geometry level", "Low")       :
        geometry_detail == 2 ?  _C("Geometry level", "Medium")    :
        geometry_detail == 3 ?  _C("Geometry level", "High")      :
        geometry_detail == 4 ?  _C("Geometry level", "Very high") :
                                _C("Geometry level", "Ultra high"));

    m_widgets.gfx_level->setTooltip(tooltip);
}   // updateTooltip

// --------------------------------------------------------------------------------------------
void OptionsScreenVideo::updateBlurTooltip()
{
    core::stringw tooltip;

    //I18N: in the graphical options
    tooltip = UserConfigParams::m_motionblur ? _("Motion blur: Enabled") :
                                               _("Motion blur: Disabled");

    //I18N: in the graphical options
    tooltip = tooltip + L"\n" +
        (UserConfigParams::m_dof ? _("Depth of field: Enabled") :
                                   _("Depth of field: Disabled"));

    m_widgets.blur_level->setTooltip(tooltip);
}   // updateBlurTooltip

// --------------------------------------------------------------------------------------------
extern "C" void update_swap_interval(int swap_interval);
extern "C" void reset_network_body();

void OptionsScreenVideo::eventCallback(Widget* widget, const std::string& name,
                                       const int playerID)
{
    if (widget == m_widgets.options_choice)
    {
        std::string selection = m_widgets.options_choice->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        if (selection != "tab_video")
            OptionsCommon::switchTab(selection);
    }
    else if (widget == m_widgets.back)
    {
        StateManager::get()->escapePressed();
    }
    else if (widget == m_widgets.custom)
    {
        new CustomVideoSettingsDialog(0.8f, 0.9f);
    }
    else if (widget == m_widgets.gfx_level)
    {
        const int level = m_widgets.gfx_level->getValue();

        // Enable the blur spinner only if the new renderer is on
        m_widgets.blur_level->setActive(level >= 3);

        // Same with Render resolution slider
        m_widgets.scale_rtts->setActive(UserConfigParams::m_dynamic_lights ||
            GE::getDriver()->getDriverType() == video::EDT_VULKAN);

        applyGFXPreset(level);
        updateImageQuality(false /* force reload textures */);
        updateGfxSlider();
        setSSR();
    }
    else if (widget == m_widgets.blur_level)
    {
        const int level = m_widgets.blur_level->getValue();

        if (UserConfigParams::m_dynamic_lights)
        {
            UserConfigParams::m_motionblur = blur_presets[level].motionblur;
            UserConfigParams::m_dof = blur_presets[level].dof;
        }

        updateBlurSlider();
    }
    else if (widget == m_widgets.vsync) // Also handles the FPS limiter
    {
        int swap = m_widgets.vsync->getValue();
        if (swap == 0)
        {
            UserConfigParams::m_swap_interval = 1;
            UserConfigParams::m_max_fps.revertToDefaults();
        }
        else
        {
            UserConfigParams::m_swap_interval = 0;
            std::string fps = StringUtils::wideToUtf8(m_widgets.vsync->getStringValue());
            UserConfigParams::m_max_fps.revertToDefaults();
            int max_fps = UserConfigParams::m_max_fps;
            StringUtils::fromString(fps, max_fps);
            UserConfigParams::m_max_fps = max_fps;
        }
#if defined(_IRR_COMPILE_WITH_SDL_DEVICE_)
        update_swap_interval(UserConfigParams::m_swap_interval);
#endif
    } // vSync
    else if (widget == m_widgets.scale_rtts)
    {
        const int level = m_widgets.scale_rtts->getValue();
        assert(level < (int)scale_rtts_presets.size());

        UserConfigParams::m_scale_rtts_factor = scale_rtts_presets[level].value;

        GE::GEVulkanDriver* gevk = GE::getVKDriver();
        if (gevk && GE::getGEConfig()->m_render_scale != UserConfigParams::m_scale_rtts_factor)
        {
            GE::getGEConfig()->m_render_scale = UserConfigParams::m_scale_rtts_factor;
            gevk->updateDriver();
        }
        updateScaleRTTsSlider();
    } // scale_rtts
    else if (widget == m_widgets.benchmarkCurrent)
    {
        // To avoid crashes and ensure the proper settings are used during the benchmark,
        // we apply the settings. If this doesn't require restarting the screen, we start
        // the benchmark immediately, otherwise we schedule it to start after the restart.
        if (applySettings() == 0)
            profiler.startBenchmark();
        else
            RaceManager::get()->scheduleBenchmark();
    } // benchmarkCurrent
    /*else if (widget == m_widgets.benchmarkRecommend)
    {
        new RecommendVideoSettingsDialog(0.8f, 0.9f);
    }*/ // benchmarkRecommend
}   // eventCallback

// --------------------------------------------------------------------------------------------
void OptionsScreenVideo::tearDown()
{
    applySettings();
    Screen::tearDown();
    // save changes when leaving screen
    user_config->saveConfig();
}   // tearDown

// --------------------------------------------------------------------------------------------
/* Returns 1 or 2 if a restart will be done, 0 otherwise */
int OptionsScreenVideo::applySettings()
{
    if (m_prev_adv_pipline != UserConfigParams::m_dynamic_lights && CVS->isGLSL())
    {
        irr_driver->sameRestart();
        return 1;
    }
    return 0;
}   // applySettings

// --------------------------------------------------------------------------------------------
bool OptionsScreenVideo::onEscapePressed()
{
    GUIEngine::focusNothingForPlayer(PLAYER_ID_GAME_MASTER);
    return true;
}

// --------------------------------------------------------------------------------------------
void OptionsScreenVideo::setSSR()
{
    if (!UserConfigParams::m_ssr)
        GE::getGEConfig()->m_screen_space_reflection_type = GE::GSSRT_DISABLED;
    else
    {
        int val = UserConfigParams::m_geometry_level;
        switch (val)
        {
        case 3:
            GE::getGEConfig()->m_screen_space_reflection_type = GE::GSSRT_HIZ100;
            break;
        case 4:
            GE::getGEConfig()->m_screen_space_reflection_type = GE::GSSRT_HIZ200;
            break;
        case 5:
            GE::getGEConfig()->m_screen_space_reflection_type = GE::GSSRT_HIZ400;
            break;
        default:
            GE::getGEConfig()->m_screen_space_reflection_type = GE::GSSRT_FAST;
            break;
        }
    }
}   // setSSR

#endif // ifndef SERVER_ONLY