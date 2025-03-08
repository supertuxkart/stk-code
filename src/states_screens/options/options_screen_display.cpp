 
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

// Manages includes common to all or most options screens
#include "states_screens/options/options_common.hpp"

#include "graphics/camera/camera.hpp"
#include "graphics/camera/camera_normal.hpp"
#include "graphics/irr_driver.hpp"
#include "modes/world.hpp"
#include "states_screens/dialogs/custom_camera_settings.hpp"

#ifndef SERVER_ONLY
#include <ge_main.hpp>
#include <ge_vulkan_driver.hpp>
#include <ge_vulkan_texture_descriptor.hpp>
#include <SDL_video.h>
#include "../../lib/irrlicht/source/Irrlicht/CIrrDeviceSDL.h"
#endif

#include <IrrlichtDevice.h>

using namespace GUIEngine;
bool OptionsScreenDisplay::m_fullscreen_checkbox_focus = false;

// --------------------------------------------------------------------------------------------

OptionsScreenDisplay::OptionsScreenDisplay() : Screen("options/options_display.stkgui")
{
    m_inited = false;
}   // OptionsScreenDisplay

// --------------------------------------------------------------------------------------------

void OptionsScreenDisplay::loadedFromFile()
{
    m_inited = false;

    // Setup splitscreen spinner
    GUIEngine::SpinnerWidget* splitscreen_method = getWidget<GUIEngine::SpinnerWidget>("splitscreen_method");
    splitscreen_method->m_properties[PROP_WRAP_AROUND] = "true";
    splitscreen_method->clearLabels();
    //I18N: In the UI options, splitscreen_method in the race UI
    splitscreen_method->addLabel( core::stringw(_("Vertical")));
    //I18N: In the UI options, splitscreen_method position in the race UI
    splitscreen_method->addLabel( core::stringw(_("Horizontal")));
    splitscreen_method->m_properties[GUIEngine::PROP_MIN_VALUE] = "0";
    splitscreen_method->m_properties[GUIEngine::PROP_MAX_VALUE] = "1";

    // Setup camera spinner
    GUIEngine::SpinnerWidget* camera_preset = getWidget<GUIEngine::SpinnerWidget>("camera_preset");
    assert( camera_preset != NULL );

    camera_preset->m_properties[PROP_WRAP_AROUND] = "true";
    camera_preset->clearLabels();
    //I18N: In the UI options, Camera setting: Custom
    camera_preset->addLabel( core::stringw(_("Custom")));
    //I18N: In the UI options, Camera setting: Standard
    camera_preset->addLabel( core::stringw(_("Standard")));
    //I18N: In the UI options, Camera setting: Drone chase
    camera_preset->addLabel( core::stringw(_("Drone chase")));
    camera_preset->m_properties[GUIEngine::PROP_MIN_VALUE] = "0";
    camera_preset->m_properties[GUIEngine::PROP_MAX_VALUE] = "2";
    updateCamera();
}   // loadedFromFile

// --------------------------------------------------------------------------------------------

void OptionsScreenDisplay::init()
{
    Screen::init();
    OptionsCommon::setTabStatus();

    RibbonWidget* ribbon = getWidget<RibbonWidget>("options_choice");
    assert(ribbon != NULL);
    ribbon->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    ribbon->select( "tab_display", PLAYER_ID_GAME_MASTER );

    GUIEngine::ButtonWidget* applyBtn =
        getWidget<GUIEngine::ButtonWidget>("apply_resolution");
    assert( applyBtn != NULL );

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

    bool is_vulkan_fullscreen_desktop = false;
#ifndef SERVER_ONLY
    is_vulkan_fullscreen_desktop =
        GE::getDriver()->getDriverType() == video::EDT_VULKAN &&
        GE::getGEConfig()->m_fullscreen_desktop;
#endif

    configResolutionsList();

    // ---- forbid changing resolution or animation settings from in-game
    // (we need to disable them last because some items can't be edited when
    // disabled)
    bool in_game = StateManager::get()->getGameState() == GUIEngine::INGAME_MENU;

    res->setActive(!in_game || is_vulkan_fullscreen_desktop);
    full->setActive(!in_game || is_vulkan_fullscreen_desktop);
    applyBtn->setActive(!in_game);

#if defined(MOBILE_STK) || defined(__SWITCH__)
    applyBtn->setVisible(false);
    full->setVisible(false);
    getWidget<LabelWidget>("fullscreenText")->setVisible(false);
    rememberWinpos->setVisible(false);
    getWidget<LabelWidget>("rememberWinposText")->setVisible(false);
#endif

    updateResolutionsList();

    // --- select the right camera in the spinner
    GUIEngine::SpinnerWidget* camera_preset = getWidget<GUIEngine::SpinnerWidget>("camera_preset");
    assert( camera_preset != NULL );

    camera_preset->setValue(UserConfigParams::m_camera_present); // use the saved camera
    updateCamera();

    // ---- splitscreen mode
    GUIEngine::SpinnerWidget* splitscreen_method = getWidget<GUIEngine::SpinnerWidget>("splitscreen_method");
    assert( splitscreen_method != NULL );
    splitscreen_method->setValue(UserConfigParams::m_split_screen_horizontally ? 1 : 0);
}   // init

// --------------------------------------------------------------------------------------------

void OptionsScreenDisplay::onResize()
{
    Screen::onResize();
    configResolutionsList();
    if (m_fullscreen_checkbox_focus)
    {
        m_fullscreen_checkbox_focus = false;
        Widget* full = getWidget("fullscreen");
        if (full->isActivated() && full->isVisible())
            full->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    }
}   // onResize

// --------------------------------------------------------------------------------------------

void OptionsScreenDisplay::configResolutionsList()
{
    DynamicRibbonWidget* res = getWidget<DynamicRibbonWidget>("resolutions");
    if (res == NULL)
        return;

    bool is_fullscreen_desktop = false;
#ifndef SERVER_ONLY
    is_fullscreen_desktop =
        GE::getGEConfig()->m_fullscreen_desktop;
#endif

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

    res->updateItemDisplay();

    // ---- select current resolution every time
    char searching_for[32];
    snprintf(searching_for, 32, "%ix%i", (int)UserConfigParams::m_real_width,
                                         (int)UserConfigParams::m_real_height);


    if (!res->setSelection(searching_for, PLAYER_ID_GAME_MASTER,
                          false /* focus it */, true /* even if deactivated*/))
    {
        Log::error("OptionsScreenDisplay", "Cannot find resolution %s", searching_for);
    }

}   // configResolutionsList

// -----------------------------------------------------------------------------
void OptionsScreenDisplay::updateCamera()
{
    bool in_game = StateManager::get()->getGameState() == GUIEngine::INGAME_MENU;
    if (in_game)
    {
        (Camera::getActiveCamera()->getCameraSceneNode())->setFOV(DEGREE_TO_RAD * UserConfigParams::m_camera_fov);
        CameraNormal *camera = dynamic_cast<CameraNormal*>(Camera::getActiveCamera());
        if (camera)
        {
            camera->setDistanceToKart(UserConfigParams::m_camera_distance);
        }
    }
} // updateCamera

// --------------------------------------------------------------------------------------------

void OptionsScreenDisplay::updateResolutionsList()
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

void OptionsScreenDisplay::onScrollResolutionsList(void* data)
{
    OptionsScreenDisplay* screen = (OptionsScreenDisplay*)data;
    screen->updateResolutionsList();
}

// --------------------------------------------------------------------------------------------
extern "C" void update_fullscreen_desktop(int val);
extern "C" void reset_network_body();

void OptionsScreenDisplay::eventCallback(Widget* widget, const std::string& name,
                                       const int playerID)
{
    if (name == "options_choice")
    {
        std::string selection = ((RibbonWidget*)widget)->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        if (selection != "tab_display")
			OptionsCommon::switchTab(selection);
    }
    else if(name == "back")
    {
        StateManager::get()->escapePressed();
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
            Log::error("OptionsScreenDisplay", "Failed to decode resolution %s", res.c_str());
            return;
        }

        CheckBoxWidget* w2 = getWidget<CheckBoxWidget>("fullscreen");
        assert(w2 != NULL);

        irr_driver->changeResolution(w, h, w2->getState());
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
            OptionsScreenDisplay::m_fullscreen_checkbox_focus = true;
        }
        else
            updateResolutionsList();
#endif
    } // fullscreen
    else if (name == "camera_preset")
    {
        GUIEngine::SpinnerWidget* camera_preset = getWidget<GUIEngine::SpinnerWidget>("camera_preset");
        assert( camera_preset != NULL );
        unsigned int i = camera_preset->getValue();
        UserConfigParams::m_camera_present = i;
        if (i == 1) // Standard
        {
            UserConfigParams::m_camera_fov = UserConfigParams::m_standard_camera_fov;
            UserConfigParams::m_camera_distance = UserConfigParams::m_standard_camera_distance;
            UserConfigParams::m_camera_forward_up_angle = UserConfigParams::m_standard_camera_forward_up_angle;
            UserConfigParams::m_camera_forward_smooth_position = UserConfigParams::m_standard_camera_forward_smooth_position;
            UserConfigParams::m_camera_forward_smooth_rotation = UserConfigParams::m_standard_camera_forward_smooth_rotation;
            UserConfigParams::m_camera_backward_distance = UserConfigParams::m_standard_camera_backward_distance;
            UserConfigParams::m_camera_backward_up_angle = UserConfigParams::m_standard_camera_backward_up_angle;
            UserConfigParams::m_reverse_look_use_soccer_cam = UserConfigParams::m_standard_reverse_look_use_soccer_cam;
        }
        else if (i == 2) // Drone chase
        {
            UserConfigParams::m_camera_fov = UserConfigParams::m_drone_camera_fov;
            UserConfigParams::m_camera_distance = UserConfigParams::m_drone_camera_distance;
            UserConfigParams::m_camera_forward_up_angle = UserConfigParams::m_drone_camera_forward_up_angle;
            UserConfigParams::m_camera_forward_smooth_position = UserConfigParams::m_drone_camera_forward_smooth_position;
            UserConfigParams::m_camera_forward_smooth_rotation = UserConfigParams::m_drone_camera_forward_smooth_rotation;
            UserConfigParams::m_camera_backward_distance = UserConfigParams::m_drone_camera_backward_distance;
            UserConfigParams::m_camera_backward_up_angle = UserConfigParams::m_drone_camera_backward_up_angle;
            UserConfigParams::m_reverse_look_use_soccer_cam = UserConfigParams::m_drone_reverse_look_use_soccer_cam;
        }
        else // Custom
        {
            UserConfigParams::m_camera_fov = UserConfigParams::m_saved_camera_fov;
            UserConfigParams::m_camera_distance = UserConfigParams::m_saved_camera_distance;
            UserConfigParams::m_camera_forward_up_angle = UserConfigParams::m_saved_camera_forward_up_angle;
            UserConfigParams::m_camera_forward_smooth_position = UserConfigParams::m_saved_camera_forward_smooth_position;
            UserConfigParams::m_camera_forward_smooth_rotation = UserConfigParams::m_saved_camera_forward_smooth_rotation;
            UserConfigParams::m_camera_backward_distance = UserConfigParams::m_saved_camera_backward_distance;
            UserConfigParams::m_camera_backward_up_angle = UserConfigParams::m_saved_camera_backward_up_angle;
            UserConfigParams::m_reverse_look_use_soccer_cam = UserConfigParams::m_saved_reverse_look_use_soccer_cam;
        }
        updateCamera();
    }
    else if(name == "custom_camera")
    {
        new CustomCameraSettingsDialog(0.8f, 0.95f);
    }
    else if (name == "splitscreen_method")
    {
        GUIEngine::SpinnerWidget* splitscreen_method = getWidget<GUIEngine::SpinnerWidget>("splitscreen_method");
        assert( splitscreen_method != NULL );
        UserConfigParams::m_split_screen_horizontally = (splitscreen_method->getValue() == 1);
        if (World::getWorld())
        {
            for (unsigned i = 0; i < Camera::getNumCameras(); i++)
                Camera::getCamera(i)->setupCamera();
        }
    }
}   // eventCallback

// --------------------------------------------------------------------------------------------

void OptionsScreenDisplay::tearDown()
{
#ifndef SERVER_ONLY
    Screen::tearDown();
    // save changes when leaving screen
    user_config->saveConfig();
#endif
}   // tearDown

// --------------------------------------------------------------------------------------------

bool OptionsScreenDisplay::onEscapePressed()
{
    GUIEngine::focusNothingForPlayer(PLAYER_ID_GAME_MASTER);
    return true;
}

// --------------------------------------------------------------------------------------------

void OptionsScreenDisplay::unloaded()
{
    m_inited = false;
}   // unloaded

// --------------------------------------------------------------------------------------------