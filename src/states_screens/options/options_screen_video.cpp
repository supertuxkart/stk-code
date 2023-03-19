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

#include "states_screens/options/options_screen_video.hpp"

#include "audio/sfx_manager.hpp"
#include "audio/sfx_base.hpp"
#include "config/user_config.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "guiengine/widget.hpp"
#include "io/file_manager.hpp"
#include "states_screens/dialogs/custom_video_settings.hpp"
#include "states_screens/options/options_screen_audio.hpp"
#include "states_screens/options/options_screen_general.hpp"
#include "states_screens/options/options_screen_input.hpp"
#include "states_screens/options/options_screen_language.hpp"
#include "states_screens/options/options_screen_ui.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/options/user_screen.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#ifndef SERVER_ONLY
#include <ge_main.hpp>
#include <ge_vulkan_driver.hpp>
#include <ge_vulkan_texture_descriptor.hpp>
#include <SDL_video.h>
#include "../../lib/irrlicht/source/Irrlicht/CIrrDeviceSDL.h"
#endif

#include <IrrlichtDevice.h>

#include <functional>
#include <iostream>
#include <iterator>
#include <sstream>

using namespace GUIEngine;
bool OptionsScreenVideo::m_fullscreen_checkbox_focus = false;

// --------------------------------------------------------------------------------------------
void OptionsScreenVideo::initPresets()
{
    m_presets.push_back // Level 1
    ({
        false /* light */, 0 /* shadow */, false /* bloom */, false /* lightshaft */,
        false /* glow */, false /* mlaa */, false /* ssao */, false /* light scatter */,
        false /* animatedCharacters */, 1 /* particles */, 0 /* image_quality */,
        true /* degraded IBL */
    });

    m_presets.push_back // Level 2
    ({
        false /* light */, 0 /* shadow */, false /* bloom */, false /* lightshaft */,
        false /* glow */, false /* mlaa */, false /* ssao */, false /* light scatter */,
        true /* animatedCharacters */, 2 /* particles */, 0 /* image_quality */,
        true /* degraded IBL */
    });

    m_presets.push_back // Level 3
    ({
        true /* light */, 0 /* shadow */, false /* bloom */, false /* lightshaft */,
        false /* glow */, false /* mlaa */, false /* ssao */, false /* light scatter */,
        true /* animatedCharacters */, 2 /* particles */, 1 /* image_quality */,
        true /* degraded IBL */
    });

    m_presets.push_back // Level 4
    ({
        true /* light */, 0 /* shadow */, false /* bloom */, true /* lightshaft */,
        true /* glow */, true /* mlaa */, false /* ssao */, true /* light scatter */,
        true /* animatedCharacters */, 2 /* particles */, 1 /* image_quality */,
        false /* degraded IBL */
    });

    m_presets.push_back // Level 5
    ({
        true /* light */, 512 /* shadow */, true /* bloom */, true /* lightshaft */,
        true /* glow */, true /* mlaa */, false /* ssao */, true /* light scatter */,
        true /* animatedCharacters */, 2 /* particles */, 2 /* image_quality */,
        false /* degraded IBL */
    });

    m_presets.push_back // Level 6
    ({
        true /* light */, 1024 /* shadow */, true /* bloom */, true /* lightshaft */,
        true /* glow */, true /* mlaa */, true /* ssao */, true /* light scatter */,
        true /* animatedCharacters */, 2 /* particles */, 2 /* image_quality */,
        false /* degraded IBL */
    });

    m_blur_presets.push_back
    ({
        false /* motionblur */, false /* depth of field */
    });

    m_blur_presets.push_back
    ({
        true  /* motionblur */, false /* depth of field */
    });

    m_blur_presets.push_back
    ({
        true  /* motionblur */, true  /* depth of field */
    });

    m_scale_rtts_custom_presets.push_back({ 0.3f });
    m_scale_rtts_custom_presets.push_back({ 0.35f });
    m_scale_rtts_custom_presets.push_back({ 0.4f });
    m_scale_rtts_custom_presets.push_back({ 0.45f });
    m_scale_rtts_custom_presets.push_back({ 0.5f });
    m_scale_rtts_custom_presets.push_back({ 0.55f });
    m_scale_rtts_custom_presets.push_back({ 0.6f });
    m_scale_rtts_custom_presets.push_back({ 0.65f });
    m_scale_rtts_custom_presets.push_back({ 0.7f });
    m_scale_rtts_custom_presets.push_back({ 0.75f });
    m_scale_rtts_custom_presets.push_back({ 0.8f });
    m_scale_rtts_custom_presets.push_back({ 0.85f });
    m_scale_rtts_custom_presets.push_back({ 0.9f });
    m_scale_rtts_custom_presets.push_back({ 0.95f });
    m_scale_rtts_custom_presets.push_back({ 1.0f });

}   // initPresets

// --------------------------------------------------------------------------------------------
int OptionsScreenVideo::getImageQuality()
{
    if (UserConfigParams::m_anisotropic == 2 &&
        (UserConfigParams::m_high_definition_textures & 0x01) == 0x00 &&
        UserConfigParams::m_hq_mipmap == false)
        return 0;
    if (UserConfigParams::m_anisotropic == 4 &&
        (UserConfigParams::m_high_definition_textures & 0x01) == 0x01 &&
        UserConfigParams::m_hq_mipmap == false)
        return 1;
    if (UserConfigParams::m_anisotropic == 16 &&
        (UserConfigParams::m_high_definition_textures & 0x01) == 0x01 &&
        UserConfigParams::m_hq_mipmap == true)
        return 2;
    return 1;
}   // getImageQuality

// --------------------------------------------------------------------------------------------
void OptionsScreenVideo::setImageQuality(int quality)
{
#ifndef SERVER_ONLY
    GE::GEVulkanTextureDescriptor* td = NULL;
    if (GE::getVKDriver())
        td = GE::getVKDriver()->getMeshTextureDescriptor();
    switch (quality)
    {
        case 0:
            UserConfigParams::m_anisotropic = 2;
            UserConfigParams::m_high_definition_textures = 0x02;
            UserConfigParams::m_hq_mipmap = false;
            if (td)
                td->setSamplerUse(GE::GVS_3D_MESH_MIPMAP_2);
            break;
        case 1:
            UserConfigParams::m_anisotropic = 4;
            UserConfigParams::m_high_definition_textures = 0x03;
            UserConfigParams::m_hq_mipmap = false;
            if (td)
                td->setSamplerUse(GE::GVS_3D_MESH_MIPMAP_4);
            break;
        case 2:
            UserConfigParams::m_anisotropic = 16;
            UserConfigParams::m_high_definition_textures = 0x03;
            UserConfigParams::m_hq_mipmap = true;
            if (td)
                td->setSamplerUse(GE::GVS_3D_MESH_MIPMAP_16);
            break;
        default:
            assert(false);
    }
#endif
}   // setImageQuality

// --------------------------------------------------------------------------------------------

OptionsScreenVideo::OptionsScreenVideo() : Screen("options_video.stkgui"),
                                           m_prev_adv_pipline(false),
                                           m_prev_img_quality(-1)
{
    m_resizable = true;
    m_inited = false;
    initPresets();
}   // OptionsScreenVideo

// --------------------------------------------------------------------------------------------

void OptionsScreenVideo::loadedFromFile()
{
    m_inited = false;
    assert(m_presets.size() == 6);
    assert(m_blur_presets.size() == 3);

    GUIEngine::SpinnerWidget* gfx =
        getWidget<GUIEngine::SpinnerWidget>("gfx_level");
    gfx->m_properties[GUIEngine::PROP_MAX_VALUE] =
        StringUtils::toString(m_presets.size());

    GUIEngine::SpinnerWidget* blur =
        getWidget<GUIEngine::SpinnerWidget>("blur_level");
    blur->m_properties[GUIEngine::PROP_MAX_VALUE] =
        StringUtils::toString(m_blur_presets.size() - 1);
    blur->m_properties[GUIEngine::PROP_MIN_VALUE] =
        StringUtils::toString(0);
}   // loadedFromFile

// --------------------------------------------------------------------------------------------

void OptionsScreenVideo::init()
{
    GUIEngine::getDevice()->setResizable(
        StateManager::get()->getGameState() == GUIEngine::MENU);
    Screen::init();
    m_prev_adv_pipline = UserConfigParams::m_dynamic_lights;
    m_prev_img_quality = getImageQuality();
    RibbonWidget* ribbon = getWidget<RibbonWidget>("options_choice");
    assert(ribbon != NULL);
    ribbon->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    ribbon->select( "tab_video", PLAYER_ID_GAME_MASTER );

    GUIEngine::ButtonWidget* applyBtn =
        getWidget<GUIEngine::ButtonWidget>("apply_resolution");
    assert( applyBtn != NULL );

    GUIEngine::SpinnerWidget* gfx =
        getWidget<GUIEngine::SpinnerWidget>("gfx_level");
    assert( gfx != NULL );
    
    GUIEngine::SpinnerWidget* vsync = getWidget<GUIEngine::SpinnerWidget>("vsync");
    assert( vsync != NULL );

    vsync->clearLabels();
    //I18N: In the video options
    vsync->addLabel(_("Vertical Sync"));
    std::set<int> fps = { 30, 60, 120, 180, 250, 500, 1000 };
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
    //I18N: in graphical options. The \n is a newline character, place it where appropriate, two can be used if required.
    core::stringw vsync_tooltip = _("Vsync forces the graphics card to supply a new frame\nonly when the monitor is ready to display it.");

    //I18N: in graphical options.
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

    // ---- video modes
    DynamicRibbonWidget* res = getWidget<DynamicRibbonWidget>("resolutions");
    assert(res != NULL);
    res->registerScrollCallback(onScrollResolutionsList, this);

    CheckBoxWidget* full = getWidget<CheckBoxWidget>("fullscreen");
    assert( full != NULL );
    full->setState( UserConfigParams::m_fullscreen );
    
    CheckBoxWidget* rememberWinpos = getWidget<CheckBoxWidget>("rememberWinpos");
    assert( rememberWinpos != NULL );
    rememberWinpos->setState(UserConfigParams::m_remember_window_location);
    rememberWinpos->setActive(!UserConfigParams::m_fullscreen);
#ifdef DEBUG
    LabelWidget* full_text = getWidget<LabelWidget>("fullscreenText");
    assert( full_text != NULL );

    LabelWidget* rememberWinposText = 
                                   getWidget<LabelWidget>("rememberWinposText");
    assert( rememberWinposText != NULL );
#endif

    bool is_fullscreen_desktop = false;
    bool is_vulkan_fullscreen_desktop = false;
#ifndef SERVER_ONLY
    is_fullscreen_desktop =
        GE::getGEConfig()->m_fullscreen_desktop;
    is_vulkan_fullscreen_desktop =
        GE::getDriver()->getDriverType() == video::EDT_VULKAN &&
        is_fullscreen_desktop;
#endif

    // --- get resolution list from irrlicht the first time
    if (!m_inited)
    {
        res->clearItems();

        const std::vector<IrrDriver::VideoMode>& modes =
                                                irr_driver->getVideoModes();
        const int amount = (int)modes.size();

        m_resolutions.clear();
        Resolution r;

        bool found_config_res = false;

        // for some odd reason, irrlicht sometimes fails to report the good
        // old standard resolutions
        // those are always useful for windowed mode
        bool found_1024_768 = false;
        bool found_1280_720 = false;

        for (int n=0; n<amount; n++)
        {
            r.width  = modes[n].getWidth();
            r.height = modes[n].getHeight();
            r.fullscreen = true;
            m_resolutions.push_back(r);

            if (r.width  == UserConfigParams::m_real_width &&
                r.height == UserConfigParams::m_real_height)
            {
                found_config_res = true;
            }

            if (r.width == 1024 && r.height == 768)
            {
                found_1024_768 = true;
            }
            if (r.width == 1280 && r.height == 720)
            {
                found_1280_720 = true;
            }
        }

        // Use fullscreen desktop so only show current screen size
        if (is_fullscreen_desktop)
        {
            found_config_res = false;
            m_resolutions.clear();
            found_1024_768 = true;
            found_1280_720 = true;
        }

        if (!found_config_res)
        {
            r.width  = UserConfigParams::m_real_width;
            r.height = UserConfigParams::m_real_height;
            r.fullscreen = is_fullscreen_desktop;
            m_resolutions.push_back(r);

            if (r.width == 1024 && r.height == 768)
            {
                found_1024_768 = true;
            }
            if (r.width == 1280 && r.height == 720)
            {
                found_1280_720 = true;
            }
        } // next found resolution

#if !defined(MOBILE_STK) && !defined(__SWITCH__)
        // Add default resolutions that were not found by irrlicht
        if (!found_1024_768)
        {
            r.width  = 1024;
            r.height = 768;
            r.fullscreen = false;
            m_resolutions.push_back(r);
        }

        if (!found_1280_720)
        {
            r.width  = 1280;
            r.height = 720;
            r.fullscreen = false;
            m_resolutions.push_back(r);
        }
#endif

        // Sort resolutions by size
        std::sort(m_resolutions.begin(), m_resolutions.end());

        // Add resolutions list
        for(std::vector<Resolution>::iterator it = m_resolutions.begin();
            it != m_resolutions.end(); it++)
        {
            const float ratio = it->getRatio();
            char name[32];
            sprintf(name, "%ix%i", it->width, it->height);

            core::stringw label;
            label += it->width;
            label += L"\u00D7";
            label += it->height;

#define ABOUT_EQUAL(a , b) (fabsf( a - b ) < 0.01)

            if      (ABOUT_EQUAL( ratio, (5.0f/4.0f) ))
                res->addItem(label, name, "/gui/icons/screen54.png");
            else if (ABOUT_EQUAL( ratio, (4.0f/3.0f) ))
                res->addItem(label, name, "/gui/icons/screen43.png");
            else if (ABOUT_EQUAL( ratio, (16.0f/10.0f)))
                res->addItem(label, name, "/gui/icons/screen1610.png");
            else if (ABOUT_EQUAL( ratio, (5.0f/3.0f) ))
                res->addItem(label, name, "/gui/icons/screen53.png");
            else if (ABOUT_EQUAL( ratio, (3.0f/2.0f) ))
                res->addItem(label, name, "/gui/icons/screen32.png");
            else if (ABOUT_EQUAL( ratio, (16.0f/9.0f) ))
                res->addItem(label, name, "/gui/icons/screen169.png");
            else
                res->addItem(label, name, "/gui/icons/screen_other.png");
#undef ABOUT_EQUAL
        } // add next resolution
    } // end if not inited

    res->updateItemDisplay();

    // ---- select current resolution every time
    char searching_for[32];
    snprintf(searching_for, 32, "%ix%i", (int)UserConfigParams::m_real_width,
                                         (int)UserConfigParams::m_real_height);


    if (!res->setSelection(searching_for, PLAYER_ID_GAME_MASTER,
                          false /* focus it */, true /* even if deactivated*/))
    {
        Log::error("OptionsScreenVideo", "Cannot find resolution %s", searching_for);
    }


    // --- set gfx settings values
    updateGfxSlider();
    updateBlurSlider();
    updateScaleRTTsSlider();

    // ---- forbid changing resolution or animation settings from in-game
    // (we need to disable them last because some items can't be edited when
    // disabled)
    bool in_game = StateManager::get()->getGameState() == GUIEngine::INGAME_MENU;

    res->setActive(!in_game || is_vulkan_fullscreen_desktop);
    full->setActive(!in_game || is_vulkan_fullscreen_desktop);
    applyBtn->setActive(!in_game);
#ifndef SERVER_ONLY
    gfx->setActive(!in_game && CVS->isGLSL());
    getWidget<ButtonWidget>("custom")->setActive(!in_game || !CVS->isGLSL());
    if (getWidget<SpinnerWidget>("scale_rtts")->isActivated())
    {
        getWidget<SpinnerWidget>("scale_rtts")->setActive(!in_game ||
            GE::getDriver()->getDriverType() == video::EDT_VULKAN);
    }
#endif

#if defined(MOBILE_STK) || defined(__SWITCH__)
    applyBtn->setVisible(false);
    full->setVisible(false);
    getWidget<LabelWidget>("fullscreenText")->setVisible(false);
    rememberWinpos->setVisible(false);
    getWidget<LabelWidget>("rememberWinposText")->setVisible(false);
#endif

    updateResolutionsList();

    if (m_fullscreen_checkbox_focus)
    {
        m_fullscreen_checkbox_focus = false;
        getWidget("fullscreen")->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    }
}   // init

// --------------------------------------------------------------------------------------------

void OptionsScreenVideo::updateResolutionsList()
{
    CheckBoxWidget* full = getWidget<CheckBoxWidget>("fullscreen");
    assert(full != NULL);
    bool fullscreen_selected = full->getState();
    
    for (auto resolution : m_resolutions)
    {
        DynamicRibbonWidget* drw = getWidget<DynamicRibbonWidget>("resolutions");
        assert(drw != NULL);
        assert(drw->m_rows.size() == 1);
        
        char name[128];
        sprintf(name, "%ix%i", resolution.width, resolution.height);
        
        Widget* w = drw->m_rows[0].findWidgetNamed(name);
        
        if (w != NULL)
        {
            bool active = !fullscreen_selected || resolution.fullscreen;
            w->setActive(active);
        }
    }
}

// --------------------------------------------------------------------------------------------

void OptionsScreenVideo::onScrollResolutionsList(void* data)
{
    OptionsScreenVideo* screen = (OptionsScreenVideo*)data;
    screen->updateResolutionsList();
}

// --------------------------------------------------------------------------------------------

void OptionsScreenVideo::updateGfxSlider()
{
    GUIEngine::SpinnerWidget* gfx = getWidget<GUIEngine::SpinnerWidget>("gfx_level");
    assert( gfx != NULL );

    bool found = false;
    for (unsigned int l = 0; l < m_presets.size(); l++)
    {
        if (m_presets[l].animatedCharacters == UserConfigParams::m_animated_characters &&
            m_presets[l].particles == UserConfigParams::m_particles_effects &&
            m_presets[l].image_quality == getImageQuality() &&
            m_presets[l].bloom == UserConfigParams::m_bloom &&
            m_presets[l].glow == UserConfigParams::m_glow &&
            m_presets[l].lights == UserConfigParams::m_dynamic_lights &&
            m_presets[l].lightshaft == UserConfigParams::m_light_shaft &&
            m_presets[l].mlaa == UserConfigParams::m_mlaa &&
            m_presets[l].shadows == UserConfigParams::m_shadows_resolution &&
            m_presets[l].ssao == UserConfigParams::m_ssao &&
            m_presets[l].light_scatter == UserConfigParams::m_light_scatter &&
            m_presets[l].degraded_ibl == UserConfigParams::m_degraded_IBL)
        {
            gfx->setValue(l + 1);
            found = true;
            break;
        }
    }

    if (!found)
    {
        //I18N: custom video settings
        gfx->setCustomText( _("Custom") );
    }

#ifndef SERVER_ONLY
    // Enable the blur slider if the modern renderer is used
    getWidget<GUIEngine::SpinnerWidget>("blur_level")->
        setActive(UserConfigParams::m_dynamic_lights && CVS->isGLSL());
    // Same with Render resolution slider
    getWidget<GUIEngine::SpinnerWidget>("scale_rtts")->
        setActive((UserConfigParams::m_dynamic_lights && CVS->isGLSL()) ||
        GE::getDriver()->getDriverType() == video::EDT_VULKAN);

    updateTooltip();
#endif
} // updateGfxSlider

// --------------------------------------------------------------------------------------------

void OptionsScreenVideo::updateBlurSlider()
{
    GUIEngine::SpinnerWidget* blur = getWidget<GUIEngine::SpinnerWidget>("blur_level");
    assert( blur != NULL );

    bool found = false;
    for (unsigned int l = 0; l < m_blur_presets.size(); l++)
    {
        if (m_blur_presets[l].motionblur == UserConfigParams::m_motionblur &&
            m_blur_presets[l].dof == UserConfigParams::m_dof)
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
    GUIEngine::SpinnerWidget* scale_rtts_level = 
        getWidget<GUIEngine::SpinnerWidget>("scale_rtts");
    assert( scale_rtts_level != NULL );

    bool found = false;
    for (unsigned int l = 0; l < m_scale_rtts_custom_presets.size(); l++)
    {
        if (m_scale_rtts_custom_presets[l].value == UserConfigParams::m_scale_rtts_factor)
        {
            scale_rtts_level->setValue(l);
            found = true;
            break;
        }
    }

    if (!found)
    {
        //I18N: custom video settings
        scale_rtts_level->setCustomText( _("Custom") );
    }
} // updateScaleRTTsSlider

// --------------------------------------------------------------------------------------------

void OptionsScreenVideo::updateTooltip()
{
    GUIEngine::SpinnerWidget* gfx = getWidget<GUIEngine::SpinnerWidget>("gfx_level");
    assert( gfx != NULL );

    core::stringw tooltip;

    //I18N: in the graphical options tooltip;
    // indicates a graphical feature is enabled
    const core::stringw enabled = _("Enabled");
    //I18N: in the graphical options tooltip;
    // indicates a graphical feature is disabled
    const core::stringw disabled = _("Disabled");
    //I18N: if only important particles effects is enabled
    const core::stringw important_only = _("Important only");

    //I18N: in the graphical options tooltip;
    // indicates the rendered image quality is very low
    const core::stringw very_low = _("Very Low");
    //I18N: in the graphical options tooltip;
    // indicates the rendered image quality is low
    const core::stringw low = _("Low");
    //I18N: in the graphical options tooltip;
    // indicates the rendered image quality is high
    const core::stringw high = _("High");

    //I18N: in graphical options
    tooltip = _("Particles Effects: %s",
        UserConfigParams::m_particles_effects == 2 ? enabled :
        UserConfigParams::m_particles_effects == 1 ? important_only :
        disabled);

    //I18N: in graphical options
    tooltip = tooltip + L"\n" + _("Animated Characters: %s",
        UserConfigParams::m_animated_characters ? enabled : disabled);
    //I18N: in graphical options
    tooltip = tooltip + L"\n" + _("Dynamic lights: %s",
        UserConfigParams::m_dynamic_lights ? enabled : disabled);
    //I18N: in graphical options
    tooltip = tooltip + L"\n" + _("Light scattering: %s",
        UserConfigParams::m_light_scatter ? enabled : disabled);
    //I18N: in graphical options
    tooltip = tooltip + L"\n" + _("Anti-aliasing: %s",
        UserConfigParams::m_mlaa ? enabled : disabled);
    //I18N: in graphical options
    tooltip = tooltip + L"\n" + _("Ambient occlusion: %s",
        UserConfigParams::m_ssao ? enabled : disabled);
    //I18N: in graphical options
    if (UserConfigParams::m_shadows_resolution == 0)
        tooltip = tooltip + L"\n" + _("Shadows: %s", disabled);
    else
        tooltip = tooltip + L"\n" + _("Shadows: %i", UserConfigParams::m_shadows_resolution);

    //I18N: in graphical options
    tooltip = tooltip + L"\n" + _("Bloom: %s",
        UserConfigParams::m_bloom ? enabled : disabled);

    //I18N: in graphical options
    tooltip = tooltip + L"\n" + _("Glow (outlines): %s",
        UserConfigParams::m_glow ? enabled : disabled);

    //I18N: in graphical options
    tooltip = tooltip + L"\n" + _("Light shaft (God rays): %s",
        UserConfigParams::m_light_shaft ? enabled : disabled);

    //I18N: in graphical options
    int quality = getImageQuality();
    tooltip = tooltip + L"\n" + _("Rendered image quality: %s",
        quality == 0 ? very_low : quality == 1 ? low : high);

    gfx->setTooltip(tooltip);
}   // updateTooltip

// --------------------------------------------------------------------------------------------

void OptionsScreenVideo::updateBlurTooltip()
{
    GUIEngine::SpinnerWidget* blur = getWidget<GUIEngine::SpinnerWidget>("blur_level");
    assert( blur != NULL );

    core::stringw tooltip;

    const core::stringw enabled = _("Enabled");
    const core::stringw disabled = _("Disabled");

    //I18N: in graphical options
    tooltip = tooltip + _("Motion blur: %s",
        UserConfigParams::m_motionblur ? enabled : disabled);

    //I18N: in graphical options
    tooltip = tooltip + L"\n" + _("Depth of field: %s",
        UserConfigParams::m_dof ? enabled : disabled);

    blur->setTooltip(tooltip);
}   // updateBlurTooltip

// --------------------------------------------------------------------------------------------
extern "C" void update_swap_interval(int swap_interval);
extern "C" void update_fullscreen_desktop(int val);
extern "C" void reset_network_body();

void OptionsScreenVideo::eventCallback(Widget* widget, const std::string& name,
                                       const int playerID)
{
    if (name == "options_choice")
    {
        std::string selection = ((RibbonWidget*)widget)->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        Screen *screen = NULL;
        if (selection == "tab_audio")
            screen = OptionsScreenAudio::getInstance();
        //else if (selection == "tab_video")
        //    screen = OptionsScreenVideo::getInstance();
        else if (selection == "tab_players")
            screen = TabbedUserScreen::getInstance();
        else if (selection == "tab_controls")
            screen = OptionsScreenInput::getInstance();
        else if (selection == "tab_ui")
            screen = OptionsScreenUI::getInstance();
        else if (selection == "tab_general")
            screen = OptionsScreenGeneral::getInstance();
        else if (selection == "tab_language")
            screen = OptionsScreenLanguage::getInstance();
        if(screen)
            StateManager::get()->replaceTopMostScreen(screen);
    }
    else if(name == "back")
    {
        StateManager::get()->escapePressed();
    }
    else if(name == "custom")
    {
        new CustomVideoSettingsDialog(0.8f, 0.9f);
    }
    else if(name == "apply_resolution")
    {
        using namespace GUIEngine;

        DynamicRibbonWidget* w1=getWidget<DynamicRibbonWidget>("resolutions");
        assert(w1 != NULL);
        assert(w1->m_rows.size() == 1);
        
        int index = w1->m_rows[0].getSelection(PLAYER_ID_GAME_MASTER);
        Widget* selected_widget = &w1->m_rows[0].getChildren()[index];
        
        if (!selected_widget->isActivated())
            return;

        const std::string& res =
            w1->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        int w = -1, h = -1;
        if (sscanf(res.c_str(), "%ix%i", &w, &h) != 2 || w == -1 || h == -1)
        {
            Log::error("OptionsScreenVideo", "Failed to decode resolution %s", res.c_str());
            return;
        }

        CheckBoxWidget* w2 = getWidget<CheckBoxWidget>("fullscreen");
        assert(w2 != NULL);


        irr_driver->changeResolution(w, h, w2->getState());
    }
    else if (name == "gfx_level")
    {
        GUIEngine::SpinnerWidget* gfx_level =
            getWidget<GUIEngine::SpinnerWidget>("gfx_level");
        assert( gfx_level != NULL );

        const int level = gfx_level->getValue() - 1;

        // Enable the blur spinner only if the new renderer is on
        getWidget<GUIEngine::SpinnerWidget>("blur_level")->setActive(level >= 2);

        // Same with Render resolution slider
#ifndef SERVER_ONLY
        getWidget<GUIEngine::SpinnerWidget>("scale_rtts")->
            setActive(UserConfigParams::m_dynamic_lights ||
            GE::getDriver()->getDriverType() == video::EDT_VULKAN);
#endif
        UserConfigParams::m_animated_characters = m_presets[level].animatedCharacters;
        UserConfigParams::m_particles_effects = m_presets[level].particles;
        setImageQuality(m_presets[level].image_quality);
        UserConfigParams::m_bloom = m_presets[level].bloom;
        UserConfigParams::m_glow = m_presets[level].glow;
        UserConfigParams::m_dynamic_lights = m_presets[level].lights;
        UserConfigParams::m_light_shaft = m_presets[level].lightshaft;
        UserConfigParams::m_mlaa = m_presets[level].mlaa;
        UserConfigParams::m_shadows_resolution = m_presets[level].shadows;
        UserConfigParams::m_ssao = m_presets[level].ssao;
        UserConfigParams::m_light_scatter = m_presets[level].light_scatter;
        UserConfigParams::m_degraded_IBL = m_presets[level].degraded_ibl;

        updateGfxSlider();
    }
    else if (name == "blur_level")
    {
        GUIEngine::SpinnerWidget* blur_level =
            getWidget<GUIEngine::SpinnerWidget>("blur_level");
        assert( blur_level != NULL );

        const int level = blur_level->getValue();

        if (UserConfigParams::m_dynamic_lights)
        {
            UserConfigParams::m_motionblur = m_blur_presets[level].motionblur;
            UserConfigParams::m_dof = m_blur_presets[level].dof;
        }

        updateBlurSlider();
    }
    else if (name == "vsync")
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
#if !defined(SERVER_ONLY) && defined(_IRR_COMPILE_WITH_SDL_DEVICE_)
        update_swap_interval(UserConfigParams::m_swap_interval);
#endif
    }
    else if (name == "scale_rtts")
    {
        GUIEngine::SpinnerWidget* scale_rtts_level =
            getWidget<GUIEngine::SpinnerWidget>("scale_rtts");
        assert( scale_rtts_level != NULL );

        const int level = scale_rtts_level->getValue();
        assert(level < (int)m_scale_rtts_custom_presets.size());

        UserConfigParams::m_scale_rtts_factor = m_scale_rtts_custom_presets[level].value;
#ifndef SERVER_ONLY
        GE::GEVulkanDriver* gevk = GE::getVKDriver();
        if (gevk && GE::getGEConfig()->m_render_scale != UserConfigParams::m_scale_rtts_factor)
        {
            GE::getGEConfig()->m_render_scale = UserConfigParams::m_scale_rtts_factor;
            gevk->updateDriver();
        }
#endif
        updateScaleRTTsSlider();
    }
    else if (name == "rememberWinpos")
    {
        CheckBoxWidget* rememberWinpos = getWidget<CheckBoxWidget>("rememberWinpos");
        UserConfigParams::m_remember_window_location = rememberWinpos->getState();
    }
    else if (name == "fullscreen")
    {
        CheckBoxWidget* fullscreen = getWidget<CheckBoxWidget>("fullscreen");
        CheckBoxWidget* rememberWinpos = getWidget<CheckBoxWidget>("rememberWinpos");

        rememberWinpos->setActive(!fullscreen->getState());
#ifndef SERVER_ONLY
        GE::GEVulkanDriver* gevk = GE::getVKDriver();
        if (gevk && GE::getGEConfig()->m_fullscreen_desktop)
        {
            UserConfigParams::m_fullscreen = fullscreen->getState();
            update_fullscreen_desktop(UserConfigParams::m_fullscreen);
            if (StateManager::get()->getGameState() == GUIEngine::INGAME_MENU)
            {
                StateManager::get()->popMenu();
                std::function<Screen*()> screen_function =
                    getNewScreenPointer();
                int new_width = 0;
                int new_height = 0;
                SDL_GetWindowSize(gevk->getSDLWindow(), &new_width,
                    &new_height);
                static_cast<CIrrDeviceSDL*>(gevk->getIrrlichtDevice())
                    ->handleNewSize(new_width, new_height);
                irr_driver->handleWindowResize();
                Screen* new_screen = screen_function();
                OptionsScreenVideo::m_fullscreen_checkbox_focus = true;
                new_screen->push();
            }
            else
                OptionsScreenVideo::m_fullscreen_checkbox_focus = true;
        }
        else
            updateResolutionsList();
#endif
    }
}   // eventCallback

// --------------------------------------------------------------------------------------------

void OptionsScreenVideo::tearDown()
{
    if (getWidget("fullscreen")->isVisible() &&
        getWidget("fullscreen")->isFocusedForPlayer(PLAYER_ID_GAME_MASTER))
        OptionsScreenVideo::m_fullscreen_checkbox_focus = true;

    GUIEngine::getDevice()->setResizable(false);
#ifndef SERVER_ONLY
    if (m_prev_adv_pipline != UserConfigParams::m_dynamic_lights &&
        CVS->isGLSL())
    {
        irr_driver->sameRestart();
    }
    else if (m_prev_img_quality != getImageQuality())
    {
        irr_driver->setMaxTextureSize();
    }
    Screen::tearDown();
    // save changes when leaving screen
    user_config->saveConfig();
#endif
}   // tearDown

// --------------------------------------------------------------------------------------------

bool OptionsScreenVideo::onEscapePressed()
{
    GUIEngine::focusNothingForPlayer(PLAYER_ID_GAME_MASTER);
    return true;
}

// --------------------------------------------------------------------------------------------

void OptionsScreenVideo::unloaded()
{
    m_inited = false;
}   // unloaded

// --------------------------------------------------------------------------------------------
