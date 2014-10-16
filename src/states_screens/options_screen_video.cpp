//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2013 Marianne Gagnon
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

#include "states_screens/options_screen_video.hpp"

#include "audio/music_manager.hpp"
#include "audio/sfx_manager.hpp"
#include "audio/sfx_base.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "guiengine/widget.hpp"
#include "io/file_manager.hpp"
#include "states_screens/dialogs/custom_video_settings.hpp"
#include "states_screens/options_screen_audio.hpp"
#include "states_screens/options_screen_input.hpp"
#include "states_screens/options_screen_ui.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/user_screen.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <iostream>
#include <sstream>

using namespace GUIEngine;

DEFINE_SCREEN_SINGLETON( OptionsScreenVideo );

struct GFXPreset
{
    bool lights;
    int shadows;
    bool bloom;
    bool motionblur;
    bool lightshaft;
    bool glow;
    bool mlaa;
    bool ssao;
    bool weather;
    bool animatedScenery;
    int animatedCharacters;
    int anisotropy;
    /** Depth of field */
    bool dof;
    bool global_illumination;
};

static GFXPreset GFX_PRESETS[] =
{
    {
        false /* light */, 0 /* shadow */, false /* bloom */, false /* motionblur */,
        false /* lightshaft */, false /* glow */, false /* mlaa */, false /* ssao */, false /* weather */,
        false /* animatedScenery */, 0 /* animatedCharacters */, 0 /* anisotropy */,
        false /* depth of field */, false /* global illumination */
    },

    {
        false /* light */, 0 /* shadow */, false /* bloom */, false /* motionblur */,
        false /* lightshaft */, false /* glow */, false /* mlaa */, false /* ssao */, false /* weather */,
        true /* animatedScenery */, 1 /* animatedCharacters */, 4 /* anisotropy */,
        false /* depth of field */, false /* global illumination */
    },

    {
        true /* light */, 0 /* shadow */, false /* bloom */, false /* motionblur */,
        false /* lightshaft */, false /* glow */, false /* mlaa */, false /* ssao */, true /* weather */,
        true /* animatedScenery */, 1 /* animatedCharacters */, 4 /* anisotropy */,
        false /* depth of field */, false /* global illumination */
    },

    {
        true /* light */, 0 /* shadow */, false /* bloom */, true /* motionblur */,
        true /* lightshaft */, true /* glow */, true /* mlaa */, false /* ssao */, true /* weather */,
        true /* animatedScenery */, 1 /* animatedCharacters */, 8 /* anisotropy */,
        false /* depth of field */, false /* global illumination */
    },

    {
        true /* light */, 2 /* shadow */, true /* bloom */, true /* motionblur */,
        true /* lightshaft */, true /* glow */, true /* mlaa */, true /* ssao */, true /* weather */,
        true /* animatedScenery */, 2 /* animatedCharacters */, 8 /* anisotropy */,
        true /* depth of field */, true /* global illumination */
    }
};

static const int  GFX_LEVEL_AMOUNT = 5;

// ----------------------------------------------------------------------------

OptionsScreenVideo::OptionsScreenVideo() : Screen("options_video.stkgui")
{
    m_inited = false;
}   // OptionsScreenVideo

// ----------------------------------------------------------------------------

void OptionsScreenVideo::loadedFromFile()
{
    m_inited = false;


    GUIEngine::SpinnerWidget* gfx =
        getWidget<GUIEngine::SpinnerWidget>("gfx_level");
    gfx->m_properties[GUIEngine::PROP_MAX_VALUE] =
        StringUtils::toString(GFX_LEVEL_AMOUNT);

}   // loadedFromFile

// ----------------------------------------------------------------------------

void OptionsScreenVideo::init()
{
    Screen::init();
    RibbonWidget* ribbon = getWidget<RibbonWidget>("options_choice");
    if (ribbon != NULL)  ribbon->select( "tab_video", PLAYER_ID_GAME_MASTER );

    ribbon->getRibbonChildren()[1].setTooltip( _("Audio") );
    ribbon->getRibbonChildren()[2].setTooltip( _("User Interface") );
    ribbon->getRibbonChildren()[3].setTooltip( _("Players") );
    ribbon->getRibbonChildren()[4].setTooltip( _("Controls") );

    GUIEngine::ButtonWidget* applyBtn =
        getWidget<GUIEngine::ButtonWidget>("apply_resolution");
    assert( applyBtn != NULL );

    GUIEngine::SpinnerWidget* gfx =
        getWidget<GUIEngine::SpinnerWidget>("gfx_level");
    assert( gfx != NULL );

    GUIEngine::CheckBoxWidget* vsync =
        getWidget<GUIEngine::CheckBoxWidget>("vsync");
    assert( vsync != NULL );
    vsync->setState( UserConfigParams::m_vsync );


    // ---- video modes
    DynamicRibbonWidget* res = getWidget<DynamicRibbonWidget>("resolutions");
    assert( res != NULL );


    CheckBoxWidget* full = getWidget<CheckBoxWidget>("fullscreen");
    assert( full != NULL );
    full->setState( UserConfigParams::m_fullscreen );

    CheckBoxWidget* rememberWinpos = getWidget<CheckBoxWidget>("rememberWinpos");
    rememberWinpos->setState(UserConfigParams::m_remember_window_location);

    if (UserConfigParams::m_fullscreen) rememberWinpos->setDeactivated();
    else rememberWinpos->setActivated();


    // --- get resolution list from irrlicht the first time
    if (!m_inited)
    {
        res->clearItems();

        const std::vector<IrrDriver::VideoMode>& modes =
                                                irr_driver->getVideoModes();
        const int amount = (int)modes.size();

        bool found_config_res = false;

        // for some odd reason, irrlicht sometimes fails to report the good
        // old standard resolutions
        // those are always useful for windowed mode
        // allow 800x600 only for debug mode
#ifdef DEBUG
        bool found_800_600 = false;
#endif
        bool found_1024_640 = false;
        bool found_1024_768 = false;

        for (int n=0; n<amount; n++)
        {
            const int w = modes[n].getWidth();
            const int h = modes[n].getHeight();
            const float ratio = (float)w / h;

            if (w == UserConfigParams::m_width &&
                h == UserConfigParams::m_height)
            {
                found_config_res = true;
            }

            if (w == 800 && h == 600)
            {
#ifdef DEBUG
                found_800_600 = true;
#else
                continue;
#endif
            }
            else if (w == 1024 && h == 640)
            {
                found_1024_640 = true;
            }
            else if (w == 1024 && h == 768)
            {
                found_1024_768 = true;
            }

            char name[32];
            sprintf( name, "%ix%i", w, h );

            core::stringw label;
            label += w;
            label += L"\u00D7";
            label += h;

#define ABOUT_EQUAL(a , b) (fabsf( a - b ) < 0.01)

            if      (ABOUT_EQUAL( ratio, (5.0f/4.0f) ))
                res->addItem(label, name, "/gui/screen54.png");
            else if (ABOUT_EQUAL( ratio, (4.0f/3.0f) ))
                res->addItem(label, name, "/gui/screen43.png");
            else if (ABOUT_EQUAL( ratio, (16.0f/10.0f)))
                res->addItem(label, name, "/gui/screen1610.png");
            else if (ABOUT_EQUAL( ratio, (5.0f/3.0f) ))
                res->addItem(label, name, "/gui/screen53.png");
            else if (ABOUT_EQUAL( ratio, (3.0f/2.0f) ))
                res->addItem(label, name, "/gui/screen32.png");
            else if (ABOUT_EQUAL( ratio, (16.0f/9.0f) ))
                res->addItem(label, name, "/gui/screen169.png");
            else
                res->addItem(label, name, "/gui/screen_other.png");
#undef ABOUT_EQUAL
        } // next resolution

        if (!found_config_res)
        {
            const int w = UserConfigParams::m_width;
            const int h = UserConfigParams::m_height;
            const float ratio = (float)w / h;

            if (w == 800 && h == 600)
            {
#ifdef DEBUG
                found_800_600 = true;
#endif
            }
            else if (w == 1024 && h == 640)
            {
                found_1024_640 = true;
            }
            else if (w == 1024 && h == 768)
            {
                found_1024_768 = true;
            }

            char name[32];
            sprintf( name, "%ix%i", w, h );

            core::stringw label;
            label += w;
            label += L"\u00D7";
            label += h;

#define ABOUT_EQUAL(a , b) (fabsf( a - b ) < 0.01)

            if      (ABOUT_EQUAL( ratio, (5.0f/4.0f)   ))
                res->addItem(label, name, "/gui/screen54.png");
            else if (ABOUT_EQUAL( ratio, (4.0f/3.0f)   ))
                res->addItem(label, name, "/gui/screen43.png");
            else if (ABOUT_EQUAL( ratio, (16.0f/10.0f) ))
                res->addItem(label, name, "/gui/screen1610.png");
            else if (ABOUT_EQUAL( ratio, (5.0f/3.0f)   ))
                res->addItem(label, name, "/gui/screen53.png");
            else if (ABOUT_EQUAL( ratio, (3.0f/2.0f)   ))
                res->addItem(label, name, "/gui/screen32.png");
            else if (ABOUT_EQUAL( ratio, (16.0f/9.0f)   ))
                res->addItem(label, name, "/gui/screen169.png");
            else
                res->addItem(label, name, "/gui/screen_other.png");
#undef ABOUT_EQUAL
        }

#ifdef DEBUG
        if (!found_800_600)
        {
            res->addItem(L"800\u00D7600", "800x600", "/gui/screen43.png");
        }
#endif
        if (!found_1024_640)
        {
            res->addItem(L"1024\u00D7640", "1024x640", "/gui/screen1610.png");
        }
        if (!found_1024_768)
        {
            res->addItem(L"1024\u00D7768", "1024x768", "/gui/screen43.png");
        }

    } // end if not inited

    res->updateItemDisplay();

    // ---- select current resolution every time
    char searching_for[32];
    snprintf(searching_for, 32, "%ix%i", (int)UserConfigParams::m_width,
                                         (int)UserConfigParams::m_height);


    if (!res->setSelection(searching_for, PLAYER_ID_GAME_MASTER,
                          false /* focus it */, true /* even if deactivated*/))
    {
        Log::error("OptionsScreenVideo", "Cannot find resolution %s", searching_for);
    }


    // --- set gfx settings values
    updateGfxSlider();

    // ---- forbid changing resolution or animation settings from in-game
    // (we need to disable them last because some items can't be edited when
    // disabled)
    if (StateManager::get()->getGameState() == GUIEngine::INGAME_MENU)
    {
        res->setDeactivated();
        full->setDeactivated();
        applyBtn->setDeactivated();
        gfx->setDeactivated();
        getWidget<ButtonWidget>("custom")->setDeactivated();
    }
    else
    {
        // Enable back widgets if they were visited in-game previously
        res->setActivated();
        full->setActivated();
        applyBtn->setActivated();
        gfx->setActivated();
        getWidget<ButtonWidget>("custom")->setActivated();
    }
}   // init

// ----------------------------------------------------------------------------

void OptionsScreenVideo::updateGfxSlider()
{
    GUIEngine::SpinnerWidget* gfx =
    getWidget<GUIEngine::SpinnerWidget>("gfx_level");
    assert( gfx != NULL );

    bool found = false;
    for (int l=0; l<GFX_LEVEL_AMOUNT; l++)
    {
        if (GFX_PRESETS[l].animatedCharacters == UserConfigParams::m_show_steering_animations &&
            GFX_PRESETS[l].animatedScenery == UserConfigParams::m_graphical_effects &&
            GFX_PRESETS[l].anisotropy == UserConfigParams::m_anisotropic &&
            GFX_PRESETS[l].bloom == UserConfigParams::m_bloom &&
            GFX_PRESETS[l].glow == UserConfigParams::m_glow &&
            GFX_PRESETS[l].lights == UserConfigParams::m_dynamic_lights &&
            GFX_PRESETS[l].lightshaft == UserConfigParams::m_light_shaft &&
            GFX_PRESETS[l].mlaa == UserConfigParams::m_mlaa &&
            GFX_PRESETS[l].motionblur == UserConfigParams::m_motionblur &&
            //GFX_PRESETS[l].shaders == UserConfigParams::m_pixel_shaders
            GFX_PRESETS[l].shadows == UserConfigParams::m_shadows &&
            GFX_PRESETS[l].ssao == UserConfigParams::m_ssao &&
            GFX_PRESETS[l].weather == UserConfigParams::m_weather_effects &&
            GFX_PRESETS[l].dof == UserConfigParams::m_dof &&
            GFX_PRESETS[l].global_illumination == UserConfigParams::m_gi)
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

    updateTooltip();
}

// ----------------------------------------------------------------------------

void OptionsScreenVideo::updateTooltip()
{
    GUIEngine::SpinnerWidget* gfx =
        getWidget<GUIEngine::SpinnerWidget>("gfx_level");
    assert( gfx != NULL );

    core::stringw tooltip;

    //I18N: in the graphical options tooltip;
    // indicates a graphical feature is enabled
    const core::stringw enabled = _LTR("Enabled");
    //I18N: in the graphical options tooltip;
    // indicates a graphical feature is disabled
    const core::stringw disabled = _LTR("Disabled");
    //I18N: if all kart animations are enabled
    const core::stringw all = _LTR("All");
    //I18N: if some kart animations are enabled
    const core::stringw me = _LTR("Me Only");
    //I18N: if no kart animations are enabled
    const core::stringw none = _LTR("None");

    //I18N: in graphical options
//    tooltip = tooltip + L"\n" + _("Pixel shaders : %s",
//                                  UserConfigParams::m_pixel_shaders ? enabled : disabled);
    //I18N: in graphical options
    tooltip = _("Animated Scenery : %s",
        UserConfigParams::m_graphical_effects ? enabled : disabled);
    //I18N: in graphical options
    tooltip = tooltip + L"\n" + _("Weather Effects : %s",
        UserConfigParams::m_weather_effects ? enabled : disabled);
    //I18N: in graphical options
    tooltip = tooltip + L"\n" + _("Animated Characters : %s",
        UserConfigParams::m_show_steering_animations == 2
        ? all
        : (UserConfigParams::m_show_steering_animations == 1 ? me : none));
    //I18N: in graphical options
    tooltip = tooltip + L"\n" + _("Dynamic lights : %s",
        UserConfigParams::m_dynamic_lights ? enabled : disabled);
    //I18N: in graphical options
    tooltip = tooltip + L"\n" + _("Motion blur: %s",
        UserConfigParams::m_motionblur ? enabled : disabled);
    //I18N: in graphical options
    tooltip = tooltip + L"\n" + _("Anti-aliasing : %s",
        UserConfigParams::m_mlaa ? enabled : disabled);
    //I18N: in graphical options
    tooltip = tooltip + L"\n" + _("Ambient occlusion : %s",
        UserConfigParams::m_ssao ? enabled : disabled);
    //I18N: in graphical options
    tooltip = tooltip + L"\n" + _("Shadows: %s",
        UserConfigParams::m_shadows == 1 ? "low" : UserConfigParams::m_shadows == 2 ?
                                    "high" : disabled);
    //I18N: in graphical options
    tooltip = tooltip + L"\n" + _("Bloom : %s",
        UserConfigParams::m_bloom ? enabled : disabled);

    //I18N: in graphical options
    tooltip = tooltip + L"\n" + _("Glow (outlines) : %s",
        UserConfigParams::m_glow ? enabled : disabled);

    //I18N: in graphical options
    tooltip = tooltip + L"\n" + _("Light shaft (God rays) : %s",
        UserConfigParams::m_light_shaft ? enabled : disabled);

    //I18N: in graphical options
    tooltip = tooltip + L"\n" + _("Depth of field : %s",
        UserConfigParams::m_dof ? enabled : disabled);

    //I18N: in graphical options
    tooltip = tooltip + L"\n" + _("Global illumination : %s",
        UserConfigParams::m_gi ? enabled : disabled);

    gfx->setTooltip(tooltip);
}   // updateTooltip

// ----------------------------------------------------------------------------

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

        UserConfigParams::m_show_steering_animations = GFX_PRESETS[level].animatedCharacters;
        UserConfigParams::m_graphical_effects = GFX_PRESETS[level].animatedScenery;
        UserConfigParams::m_anisotropic = GFX_PRESETS[level].anisotropy;
        UserConfigParams::m_bloom = GFX_PRESETS[level].bloom;
        UserConfigParams::m_glow = GFX_PRESETS[level].glow;
        UserConfigParams::m_dynamic_lights = GFX_PRESETS[level].lights;
        UserConfigParams::m_light_shaft = GFX_PRESETS[level].lightshaft;
        UserConfigParams::m_mlaa = GFX_PRESETS[level].mlaa;
        UserConfigParams::m_motionblur = GFX_PRESETS[level].motionblur;
        //UserConfigParams::m_pixel_shaders = GFX_PRESETS[level].shaders;
        UserConfigParams::m_shadows = GFX_PRESETS[level].shadows;
        UserConfigParams::m_ssao = GFX_PRESETS[level].ssao;
        UserConfigParams::m_weather_effects = GFX_PRESETS[level].weather;
        UserConfigParams::m_dof = GFX_PRESETS[level].dof;
        UserConfigParams::m_gi = GFX_PRESETS[level].global_illumination;

        updateGfxSlider();
    }
    else if (name == "vsync")
    {
        GUIEngine::CheckBoxWidget* vsync =
            getWidget<GUIEngine::CheckBoxWidget>("vsync");
        assert( vsync != NULL );
        UserConfigParams::m_vsync = vsync->getState();
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

        if (fullscreen->getState()) rememberWinpos->setDeactivated();
        else rememberWinpos->setActivated();
    }
}   // eventCallback

// ----------------------------------------------------------------------------

void OptionsScreenVideo::tearDown()
{
    Screen::tearDown();
    // save changes when leaving screen
    user_config->saveConfig();
}   // tearDown

// ----------------------------------------------------------------------------

void OptionsScreenVideo::unloaded()
{
    m_inited = false;
}   // unloaded

// ----------------------------------------------------------------------------

