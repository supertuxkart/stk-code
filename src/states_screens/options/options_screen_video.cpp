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

    m_prev_adv_pipline = UserConfigParams::m_dynamic_lights;
    RibbonWidget* ribbon = getWidget<RibbonWidget>("options_choice");
    assert(ribbon != NULL);
    ribbon->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    ribbon->select( "tab_video", PLAYER_ID_GAME_MASTER );

    GUIEngine::SpinnerWidget* gfx = getWidget<GUIEngine::SpinnerWidget>("gfx_level");
    assert( gfx != NULL );
    
    GUIEngine::SpinnerWidget* vsync = getWidget<GUIEngine::SpinnerWidget>("vsync");
    assert( vsync != NULL );

    vsync->clearLabels();
    //I18N: In the video options
    vsync->addLabel(_("Vertical Sync"));
#ifdef MOBILE_STK
    std::set<int> fps = { 30, 60, 120 };
#else
    std::set<int> fps = { 30, 60, 120, 180, 250, 500, 1000 };
#endif
    fps.insert(UserConfigParams::m_max_fps);
    for (auto& i : fps)
        vsync->addLabel(core::stringw(i));
    if (UserConfigParams::m_swap_interval > 1)
        UserConfigParams::m_swap_interval = 1;

    if (UserConfigParams::m_swap_interval == 1)
        vsync->setValue(0);
    else
    {
        auto it = fps.find(UserConfigParams::m_max_fps);
        assert(it != fps.end());
        vsync->setValue(1 + std::distance(fps.begin(), it));
    }
    //I18N: in the graphical options. The \n is a newline character, place it where appropriate, two can be used if required.
    core::stringw vsync_tooltip = _("Vsync forces the graphics card to supply a new frame\nonly when the monitor is ready to display it.");

    //I18N: in the graphical options.
    vsync_tooltip = vsync_tooltip + L"\n" + _("Vsync will not work if your drivers don't support it.");

    vsync->setTooltip(vsync_tooltip);

    // Setup Render Resolution (scale_rtts) spinner
    GUIEngine::SpinnerWidget* scale_rtts = getWidget<GUIEngine::SpinnerWidget>("scale_rtts");
    assert( scale_rtts != NULL );

    scale_rtts->clearLabels();
    scale_rtts->addLabel("30%");
    scale_rtts->addLabel("35%");
    scale_rtts->addLabel("40%");
    scale_rtts->addLabel("45%");
    scale_rtts->addLabel("50%");
    scale_rtts->addLabel("55%");
    scale_rtts->addLabel("60%");
    scale_rtts->addLabel("65%");
    scale_rtts->addLabel("70%");
    scale_rtts->addLabel("75%");
    scale_rtts->addLabel("80%");
    scale_rtts->addLabel("85%");
    scale_rtts->addLabel("90%");
    scale_rtts->addLabel("95%");
    scale_rtts->addLabel("100%");
    scale_rtts->addLabel("125%");
    scale_rtts->addLabel("150%");
    scale_rtts->addLabel("200%");

    // --- set gfx settings values
    updateGfxSlider(); // Also updates the RTTS slider
    updateBlurSlider();

    // ---- forbid changing graphic settings from in-game
    // (we need to disable them last because some items can't be edited when
    // disabled)
    bool in_game = StateManager::get()->getGameState() == GUIEngine::INGAME_MENU;

    gfx->setActive(!in_game && CVS->isGLSL());
    // Outside of pauses, the GFX slider update already overwrites the pause tooltip.
    // Running updatePauseTooltip would incorrectly disable the list of gfx settings.
    if (in_game && CVS->isGLSL())
        OptionsCommon::updatePauseTooltip(gfx, true);

    getWidget<ButtonWidget>("custom")->setActive(!in_game || !CVS->isGLSL());
    OptionsCommon::updatePauseTooltip(getWidget<ButtonWidget>("custom"), in_game && CVS->isGLSL());

    if (scale_rtts->isActivated())
    {
        scale_rtts->setActive(!in_game || GE::getDriver()->getDriverType() == video::EDT_VULKAN);
        OptionsCommon::updatePauseTooltip(scale_rtts,
            in_game && GE::getDriver()->getDriverType() != video::EDT_VULKAN);
    }

    getWidget<ButtonWidget>("benchmarkCurrent")->setActive(!in_game);
    // Handle the setting/unsetting as we use a custom tooltip message
    if (in_game)
        getWidget<ButtonWidget>("benchmarkCurrent")->setTooltip(_("Performance tests are not possible during a race."));
    else
        getWidget<ButtonWidget>("benchmarkCurrent")->unsetTooltip();

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
    GUIEngine::SpinnerWidget* gfx = getWidget<GUIEngine::SpinnerWidget>("gfx_level");
    assert( gfx != NULL );
    int preset = findCurrentGFXPreset();
    if (preset == -1) // Current settings don't match a preset
    {
        //I18N: custom video settings
        gfx->setCustomText( _("Custom") );
    }
    else
    {
        gfx->setValue(preset);
    }

    // Enable the blur slider if the modern renderer is used
    getWidget<GUIEngine::SpinnerWidget>("blur_level")->
        setActive(UserConfigParams::m_dynamic_lights && CVS->isGLSL());
    // Same with Render resolution slider
    updateScaleRTTsSlider();

    updateTooltip();
} // updateGfxSlider

// --------------------------------------------------------------------------------------------
void OptionsScreenVideo::updateBlurSlider()
{
    GUIEngine::SpinnerWidget* blur = getWidget<GUIEngine::SpinnerWidget>("blur_level");
    assert( blur != NULL );

    bool found = false;
    for (unsigned int l = 0; l < blur_presets.size(); l++)
    {
        if (blur_presets[l].motionblur == UserConfigParams::m_motionblur &&
            blur_presets[l].dof == UserConfigParams::m_dof)
        {
            blur->setValue(l);
            found = true;
            break;
        }
    }

    if (!found)
    {
        //I18N: custom video settings
        blur->setCustomText( _("Custom") );
    }

    updateBlurTooltip();
} // updateBlurSlider

// --------------------------------------------------------------------------------------------
void OptionsScreenVideo::updateScaleRTTsSlider()
{
    bool rtts_on = (UserConfigParams::m_dynamic_lights && CVS->isGLSL()) ||
        GE::getDriver()->getDriverType() == video::EDT_VULKAN;

    GUIEngine::SpinnerWidget* rtts_slider = getWidget<GUIEngine::SpinnerWidget>("scale_rtts");
    assert( rtts_slider != NULL );

    rtts_slider->setActive(rtts_on);

    bool found = false;
    float rtts_value = (rtts_on) ? UserConfigParams::m_scale_rtts_factor : 1.0f;
    for (unsigned int l = 0; l < scale_rtts_presets.size(); l++)
    {
        if (scale_rtts_presets[l].value == rtts_value)
        {
            rtts_slider->setValue(l);
            found = true;
            if (scale_rtts_presets[l].value > 1.0f)
                rtts_slider->markAsIncorrect();
            else
                rtts_slider->markAsCorrect();

            break;
        }
    }

    if (!found)
    {
        //I18N: custom video settings
        rtts_slider->setCustomText( _("Custom") );
    }
} // updateScaleRTTsSlider

// --------------------------------------------------------------------------------------------
void OptionsScreenVideo::updateTooltip()
{
    GUIEngine::SpinnerWidget* gfx = getWidget<GUIEngine::SpinnerWidget>("gfx_level");
    assert( gfx != NULL );

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

    gfx->setTooltip(tooltip);
}   // updateTooltip

// --------------------------------------------------------------------------------------------
void OptionsScreenVideo::updateBlurTooltip()
{
    GUIEngine::SpinnerWidget* blur = getWidget<GUIEngine::SpinnerWidget>("blur_level");
    assert( blur != NULL );

    core::stringw tooltip;

    //I18N: in the graphical options
    tooltip = UserConfigParams::m_motionblur ? _("Motion blur: Enabled") :
                                               _("Motion blur: Disabled");

    //I18N: in the graphical options
    tooltip = tooltip + L"\n" +
        (UserConfigParams::m_dof ? _("Depth of field: Enabled") :
                                   _("Depth of field: Disabled"));

    blur->setTooltip(tooltip);
}   // updateBlurTooltip

// --------------------------------------------------------------------------------------------
extern "C" void update_swap_interval(int swap_interval);
extern "C" void reset_network_body();

void OptionsScreenVideo::eventCallback(Widget* widget, const std::string& name,
                                       const int playerID)
{
    if (name == "options_choice")
    {
        std::string selection = ((RibbonWidget*)widget)->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        if (selection != "tab_video")
            OptionsCommon::switchTab(selection);
    }
    else if(name == "back")
    {
        StateManager::get()->escapePressed();
    }
    else if(name == "custom")
    {
        new CustomVideoSettingsDialog(0.8f, 0.9f);
    }
    else if (name == "gfx_level")
    {
        GUIEngine::SpinnerWidget* gfx_level =
            getWidget<GUIEngine::SpinnerWidget>("gfx_level");
        assert( gfx_level != NULL );

        const int level = gfx_level->getValue();

        // Enable the blur spinner only if the new renderer is on
        getWidget<GUIEngine::SpinnerWidget>("blur_level")->setActive(level >= 3);

        // Same with Render resolution slider
        getWidget<GUIEngine::SpinnerWidget>("scale_rtts")->
            setActive(UserConfigParams::m_dynamic_lights ||
            GE::getDriver()->getDriverType() == video::EDT_VULKAN);

        applyGFXPreset(level);
        updateImageQuality(false /* force reload textures */);
        updateGfxSlider();
        setSSR();
    }
    else if (name == "blur_level")
    {
        GUIEngine::SpinnerWidget* blur_level =
            getWidget<GUIEngine::SpinnerWidget>("blur_level");
        assert( blur_level != NULL );

        const int level = blur_level->getValue();

        if (UserConfigParams::m_dynamic_lights)
        {
            UserConfigParams::m_motionblur = blur_presets[level].motionblur;
            UserConfigParams::m_dof = blur_presets[level].dof;
        }

        updateBlurSlider();
    }
    else if (name == "vsync") // Also handles the FPS limiter
    {
        GUIEngine::SpinnerWidget* vsync = getWidget<GUIEngine::SpinnerWidget>("vsync");
        assert( vsync != NULL );
        int swap = vsync->getValue();
        if (swap == 0)
        {
            UserConfigParams::m_swap_interval = 1;
            UserConfigParams::m_max_fps.revertToDefaults();
        }
        else
        {
            UserConfigParams::m_swap_interval = 0;
            std::string fps = StringUtils::wideToUtf8(vsync->getStringValue());
            UserConfigParams::m_max_fps.revertToDefaults();
            int max_fps = UserConfigParams::m_max_fps;
            StringUtils::fromString(fps, max_fps);
            UserConfigParams::m_max_fps = max_fps;
        }
#if defined(_IRR_COMPILE_WITH_SDL_DEVICE_)
        update_swap_interval(UserConfigParams::m_swap_interval);
#endif
    } // vSync
    else if (name == "scale_rtts")
    {
        GUIEngine::SpinnerWidget* rtts_slider =
            getWidget<GUIEngine::SpinnerWidget>("scale_rtts");
        assert( rtts_slider != NULL );

        const int level = rtts_slider->getValue();
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
    else if (name == "benchmarkCurrent")
    {
        // To avoid crashes and ensure the proper settings are used during the benchmark,
        // we apply the settings. If this doesn't require restarting the screen, we start
        // the benchmark immediately, otherwise we schedule it to start after the restart.
        if (applySettings() == 0)
            profiler.startBenchmark();
        else
            RaceManager::get()->scheduleBenchmark();
    } // benchmarkCurrent
    /*else if (name == "benchmarkRecommend")
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