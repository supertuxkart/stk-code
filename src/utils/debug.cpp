//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 Lionel Fuentes
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

#include "debug.hpp"

#include "config/user_config.hpp"
#include "font/bold_face.hpp"
#include "font/digit_face.hpp"
#include "font/font_manager.hpp"
#include "font/regular_face.hpp"
#include "graphics/camera_debug.hpp"
#include "graphics/camera_fps.hpp"
#include "karts/explosion_animation.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/light.hpp"
#include "graphics/shaders.hpp"
#include "graphics/stk_tex_manager.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"
#include "items/powerup_manager.hpp"
#include "items/attachment.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/kart_properties.hpp"
#include "karts/controller/controller.hpp"
#include "modes/cutscene_world.hpp"
#include "modes/world.hpp"
#include "physics/irr_debug_drawer.hpp"
#include "physics/physics.hpp"
#include "race/history.hpp"
#include "main_loop.hpp"
#include "replay/replay_recorder.hpp"
#include "scriptengine/script_engine.hpp"
#include "states_screens/dialogs/debug_slider.hpp"
#include "states_screens/dialogs/general_text_field_dialog.hpp"
#include "tracks/track_manager.hpp"
#include "utils/constants.hpp"
#include "utils/log.hpp"
#include "utils/profiler.hpp"

#include <IGUIEnvironment.h>
#include <IGUIContextMenu.h>

#include <cmath>

using namespace irr;
using namespace gui;

namespace Debug {

/** This is to let mouse input events go through when the debug menu is
 *  visible, otherwise GUI events would be blocked while in a race...
 */
static bool g_debug_menu_visible = false;

// -----------------------------------------------------------------------------
// Commands for the debug menu
enum DebugMenuCommand
{
    //! graphics commands
    DEBUG_GRAPHICS_RELOAD_SHADERS,
    DEBUG_GRAPHICS_RESET,
    DEBUG_GRAPHICS_WIREFRAME,
    DEBUG_GRAPHICS_MIPMAP_VIZ,
    DEBUG_GRAPHICS_NORMALS_VIZ,
    DEBUG_GRAPHICS_SSAO_VIZ,
    DEBUG_GRAPHICS_RSM_VIZ,
    DEBUG_GRAPHICS_RH_VIZ,
    DEBUG_GRAPHICS_GI_VIZ,
    DEBUG_GRAPHICS_SHADOW_VIZ,
    DEBUG_GRAPHICS_LIGHT_VIZ,
    DEBUG_GRAPHICS_DISTORT_VIZ,
    DEBUG_GRAPHICS_BULLET_1,
    DEBUG_GRAPHICS_BULLET_2,
    DEBUG_GRAPHICS_BOUNDING_BOXES_VIZ,
    DEBUG_PROFILER,
    DEBUG_PROFILER_WRITE_REPORT,
    DEBUG_FONT_DUMP_GLYPH_PAGE,
    DEBUG_FONT_RELOAD,
    DEBUG_FPS,
    DEBUG_SAVE_REPLAY,
    DEBUG_SAVE_HISTORY,
    DEBUG_POWERUP_BOWLING,
    DEBUG_POWERUP_BUBBLEGUM,
    DEBUG_POWERUP_CAKE,
    DEBUG_POWERUP_PARACHUTE,
    DEBUG_POWERUP_PLUNGER,
    DEBUG_POWERUP_RUBBERBALL,
    DEBUG_POWERUP_SWATTER,
    DEBUG_POWERUP_SWITCH,
    DEBUG_POWERUP_ZIPPER,
    DEBUG_POWERUP_NITRO,
    DEBUG_ATTACHMENT_PARACHUTE,
    DEBUG_ATTACHMENT_BOMB,
    DEBUG_ATTACHMENT_ANVIL,
    DEBUG_ATTACHMENT_EXPLOSION,
    DEBUG_GUI_TOGGLE,
    DEBUG_GUI_HIDE_KARTS,
    DEBUG_GUI_CAM_FREE,
    DEBUG_GUI_CAM_TOP,
    DEBUG_GUI_CAM_WHEEL,
    DEBUG_GUI_CAM_BEHIND_KART,
    DEBUG_GUI_CAM_SIDE_OF_KART,
    DEBUG_GUI_CAM_NORMAL,
    DEBUG_GUI_CAM_SMOOTH,
    DEBUG_GUI_CAM_ATTACH,
    DEBUG_VIEW_KART_ONE,
    DEBUG_VIEW_KART_TWO,
    DEBUG_VIEW_KART_THREE,
    DEBUG_VIEW_KART_FOUR,
    DEBUG_VIEW_KART_FIVE,
    DEBUG_VIEW_KART_SIX,
    DEBUG_VIEW_KART_SEVEN,
    DEBUG_VIEW_KART_EIGHT,
    DEBUG_HIDE_KARTS,
    DEBUG_THROTTLE_FPS,
    DEBUG_VISUAL_VALUES,
    DEBUG_PRINT_START_POS,
    DEBUG_ADJUST_LIGHTS,
    DEBUG_SCRIPT_CONSOLE,
    DEBUG_RUN_CUTSCENE,
    DEBUG_TEXTURE_CONSOLE,
    DEBUG_START_RECORDING,
    DEBUG_STOP_RECORDING
};   // DebugMenuCommand

// -----------------------------------------------------------------------------
/** Add powerup selected from debug menu for all player karts */
void addPowerup(PowerupManager::PowerupType powerup)
{
    World* world = World::getWorld();
    if (!world) return;
    for(unsigned int i = 0; i < race_manager->getNumLocalPlayers(); i++)
    {
        AbstractKart* kart = world->getLocalPlayerKart(i);
        kart->setPowerup(powerup, 10000);
    }
}   // addPowerup

// ----------------------------------------------------------------------------
void addAttachment(Attachment::AttachmentType type)
{
    World* world = World::getWorld();
    if (world == NULL) return;
    for (unsigned int i = 0; i < world->getNumKarts(); i++)
    {
        AbstractKart *kart = world->getKart(i);
        if (!kart->getController()->isLocalPlayerController())
            continue;
        if (type == Attachment::ATTACH_ANVIL)
        {
            kart->getAttachment()
                ->set(type, kart->getKartProperties()->getAnvilDuration());
            kart->adjustSpeed(kart->getKartProperties()->getAnvilSpeedFactor());
            kart->updateWeight();
        }
        else if (type == Attachment::ATTACH_PARACHUTE)
        {
            kart->getAttachment()
                ->set(type, kart->getKartProperties()->getParachuteDuration());
        }
        else if (type == Attachment::ATTACH_BOMB)
        {
            kart->getAttachment()
                ->set(type, stk_config->m_bomb_time);
        }
    }

}   // addAttachment

// ----------------------------------------------------------------------------
void changeCameraTarget(u32 num)
{
    World* world = World::getWorld();
    Camera *cam = Camera::getActiveCamera();
    if (world == NULL || cam == NULL) return;

    if (num < (world->getNumKarts() + 1))
    {
        AbstractKart* kart = world->getKart(num - 1);
        if (kart == NULL) return;
        cam->setMode(Camera::CM_NORMAL);
        cam->setKart(kart);
    }

}   // changeCameraTarget

// -----------------------------------------------------------------------------

/** returns the light node with the lowest distance to the player kart (excluding
 * nitro emitters) */
LightNode* findNearestLight()
{

    Camera* camera = Camera::getActiveCamera();
    if (camera == NULL) {
        Log::error("[Debug Menu]", "No camera found.");
        return NULL;
    }

    core::vector3df cam_pos = camera->getCameraSceneNode()->getAbsolutePosition();
    LightNode* nearest = 0;
    float nearest_dist = 1000000.0; // big enough
    for (unsigned int i = 0; i < irr_driver->getLights().size(); i++)
    {
        LightNode* light = irr_driver->getLights()[i];

        // Avoid modifying the nitro emitter or another invisible light
        if (std::string(light->getName()).find("nitro emitter") == 0 || !light->isVisible())
            continue;

        core::vector3df light_pos = light->getAbsolutePosition();
        if ( cam_pos.getDistanceFrom(light_pos) < nearest_dist)
        {
            nearest      = irr_driver->getLights()[i];
            nearest_dist = cam_pos.getDistanceFrom(light_pos);
        }
    }

    return nearest;
}

// ----------------------------------------------------------------------------

bool handleContextMenuAction(s32 cmd_id)
{

    World *world = World::getWorld();
    Physics *physics = Physics::getInstance();
    switch(cmd_id)
    {
    case DEBUG_GRAPHICS_RELOAD_SHADERS:
#ifndef SERVER_ONLY
        Log::info("Debug", "Reloading shaders...");
        ShaderFilesManager::getInstance()->clean();
        ShaderBase::updateShaders();
#endif
        break;
    case DEBUG_GRAPHICS_RESET:
        if (physics)
            physics->setDebugMode(IrrDebugDrawer::DM_NONE);

        irr_driver->resetDebugModes();
        break;
    case DEBUG_GRAPHICS_WIREFRAME:
        if (physics)
            physics->setDebugMode(IrrDebugDrawer::DM_NONE);

        irr_driver->resetDebugModes();
        irr_driver->toggleWireframe();
        break;
    case DEBUG_GRAPHICS_MIPMAP_VIZ:
        if (physics)
            physics->setDebugMode(IrrDebugDrawer::DM_NONE);

        irr_driver->resetDebugModes();
        irr_driver->toggleMipVisualization();
        break;
    case DEBUG_GRAPHICS_NORMALS_VIZ:
        if (physics)
            physics->setDebugMode(IrrDebugDrawer::DM_NONE);

        irr_driver->resetDebugModes();
        irr_driver->toggleNormals();
        break;
    case DEBUG_GRAPHICS_SSAO_VIZ:
        if (physics)
            physics->setDebugMode(IrrDebugDrawer::DM_NONE);

        irr_driver->resetDebugModes();
        irr_driver->toggleSSAOViz();
        break;
    case DEBUG_GRAPHICS_RSM_VIZ:
        if (physics)
            physics->setDebugMode(IrrDebugDrawer::DM_NONE);

        irr_driver->resetDebugModes();
        irr_driver->toggleRSM();
        break;
    case DEBUG_GRAPHICS_RH_VIZ:
        if (physics)
            physics->setDebugMode(IrrDebugDrawer::DM_NONE);

        irr_driver->resetDebugModes();
        irr_driver->toggleRH();
        break;
    case DEBUG_GRAPHICS_GI_VIZ:
        if (physics)
            physics->setDebugMode(IrrDebugDrawer::DM_NONE);

        irr_driver->resetDebugModes();
        irr_driver->toggleGI();
        break;
    case DEBUG_GRAPHICS_SHADOW_VIZ:
        if (physics)
            physics->setDebugMode(IrrDebugDrawer::DM_NONE);

        irr_driver->resetDebugModes();
        irr_driver->toggleShadowViz();
        break;
    case DEBUG_GRAPHICS_LIGHT_VIZ:
        if (physics)
            physics->setDebugMode(IrrDebugDrawer::DM_NONE);

        irr_driver->resetDebugModes();
        irr_driver->toggleLightViz();
        break;
    case DEBUG_GRAPHICS_DISTORT_VIZ:
        if (physics)
            physics->setDebugMode(IrrDebugDrawer::DM_NONE);

        irr_driver->resetDebugModes();
        irr_driver->toggleDistortViz();
        break;
    case DEBUG_GRAPHICS_BULLET_1:
        irr_driver->resetDebugModes();

        if (!world) return false;
        physics->setDebugMode(IrrDebugDrawer::DM_KARTS_PHYSICS);
        break;
    case DEBUG_GRAPHICS_BULLET_2:
    {
        irr_driver->resetDebugModes();

        Physics *physics = Physics::getInstance();
        if (!physics) return false;
        physics->setDebugMode(IrrDebugDrawer::DM_NO_KARTS_GRAPHICS);
        break;
    }
    case DEBUG_GRAPHICS_BOUNDING_BOXES_VIZ:
        irr_driver->resetDebugModes();
        irr_driver->toggleBoundingBoxesViz();
        break;
    case DEBUG_PROFILER:
        profiler.toggleStatus();
        break;
    case DEBUG_PROFILER_WRITE_REPORT:
        profiler.writeToFile();
        break;
    case DEBUG_THROTTLE_FPS:
        main_loop->setThrottleFPS(false);
        break;
    case DEBUG_FONT_DUMP_GLYPH_PAGE:
        font_manager->getFont<BoldFace>()->dumpGlyphPage("bold");
        font_manager->getFont<DigitFace>()->dumpGlyphPage("digit");
        font_manager->getFont<RegularFace>()->dumpGlyphPage("regular");
        break;
    case DEBUG_FONT_RELOAD:
        font_manager->getFont<BoldFace>()->reset();
        font_manager->getFont<DigitFace>()->reset();
        font_manager->getFont<RegularFace>()->reset();
        break;
    case DEBUG_FPS:
        UserConfigParams::m_display_fps =
            !UserConfigParams::m_display_fps;
        break;
    case DEBUG_SAVE_REPLAY:
        ReplayRecorder::get()->save();
        break;
    case DEBUG_SAVE_HISTORY:
        history->Save();
        break;
    case DEBUG_POWERUP_BOWLING:
        addPowerup(PowerupManager::POWERUP_BOWLING);
        break;
    case DEBUG_POWERUP_BUBBLEGUM:
        addPowerup(PowerupManager::POWERUP_BUBBLEGUM);
        break;
    case DEBUG_POWERUP_CAKE:
        addPowerup(PowerupManager::POWERUP_CAKE);
        break;
    case DEBUG_POWERUP_PARACHUTE:
        addPowerup(PowerupManager::POWERUP_PARACHUTE);
        break;
    case DEBUG_POWERUP_PLUNGER:
        addPowerup(PowerupManager::POWERUP_PLUNGER);
        break;
    case DEBUG_POWERUP_RUBBERBALL:
        addPowerup(PowerupManager::POWERUP_RUBBERBALL);
        break;
    case DEBUG_POWERUP_SWATTER:
        addPowerup(PowerupManager::POWERUP_SWATTER);
        break;
    case DEBUG_POWERUP_SWITCH:
        addPowerup(PowerupManager::POWERUP_SWITCH);
        break;
    case DEBUG_POWERUP_ZIPPER:
        addPowerup(PowerupManager::POWERUP_ZIPPER);
        break;
    case DEBUG_POWERUP_NITRO:
    {
        if (!world) return false;
        const unsigned int num_local_players =
            race_manager->getNumLocalPlayers();
        for (unsigned int i = 0; i < num_local_players; i++)
        {
            AbstractKart* kart = world->getLocalPlayerKart(i);
            kart->setEnergy(100.0f);
        }
        break;
    }
    case DEBUG_ATTACHMENT_ANVIL:
        addAttachment(Attachment::ATTACH_ANVIL);
        break;
    case DEBUG_ATTACHMENT_BOMB:
        addAttachment(Attachment::ATTACH_BOMB);
        break;
    case DEBUG_ATTACHMENT_PARACHUTE:
        addAttachment(Attachment::ATTACH_PARACHUTE);
        break;
    case DEBUG_ATTACHMENT_EXPLOSION:
        for (unsigned int i = 0; i < race_manager->getNumLocalPlayers(); i++)
        {
            AbstractKart* kart = world->getLocalPlayerKart(i);
            ExplosionAnimation::create(kart, kart->getXYZ(), true);
        }
        break;
    case DEBUG_GUI_TOGGLE:
    {
        if (!world) return false;
        RaceGUIBase* gui = world->getRaceGUI();
        if (gui != NULL) gui->m_enabled = !gui->m_enabled;
        break;
    }
    case DEBUG_GUI_HIDE_KARTS:
        if (!world) return false;
        for (unsigned int n = 0; n<world->getNumKarts(); n++)
        {
            AbstractKart* kart = world->getKart(n);
            if (kart->getController()->isPlayerController())
                kart->getNode()->setVisible(false);
        }
        break;
    case DEBUG_GUI_CAM_TOP:
        CameraDebug::setDebugType(CameraDebug::CM_DEBUG_TOP_OF_KART);
        Camera::changeCamera(0, Camera::CM_TYPE_DEBUG);
        irr_driver->getDevice()->getCursorControl()->setVisible(true);
        break;
    case DEBUG_GUI_CAM_WHEEL:
        CameraDebug::setDebugType(CameraDebug::CM_DEBUG_GROUND);
        Camera::changeCamera(0, Camera::CM_TYPE_DEBUG);
        irr_driver->getDevice()->getCursorControl()->setVisible(true);
        break;
    case DEBUG_GUI_CAM_BEHIND_KART:
        CameraDebug::setDebugType(CameraDebug::CM_DEBUG_BEHIND_KART);
        Camera::changeCamera(0, Camera::CM_TYPE_DEBUG);
        irr_driver->getDevice()->getCursorControl()->setVisible(true);
        break;
    case DEBUG_GUI_CAM_SIDE_OF_KART:
        CameraDebug::setDebugType(CameraDebug::CM_DEBUG_SIDE_OF_KART);
        Camera::changeCamera(0, Camera::CM_TYPE_DEBUG);
        irr_driver->getDevice()->getCursorControl()->setVisible(true);
        break;
    case DEBUG_GUI_CAM_FREE:
    {
        Camera *camera = Camera::getActiveCamera();
        Camera::changeCamera(camera->getIndex(), Camera::CM_TYPE_FPS);
        irr_driver->getDevice()->getCursorControl()->setVisible(false);
        // Reset camera rotation
        CameraFPS *cam = dynamic_cast<CameraFPS*>(Camera::getActiveCamera());
        if(cam)
        {
            cam->setDirection(vector3df(0, 0, 1));
            cam->setUpVector(vector3df(0, 1, 0));
        }
        break;
    }
    case DEBUG_GUI_CAM_NORMAL:
    {
        Camera *camera = Camera::getActiveCamera();
        Camera::changeCamera(camera->getIndex(), Camera::CM_TYPE_NORMAL);
        irr_driver->getDevice()->getCursorControl()->setVisible(true);
        break;
    }
    case DEBUG_GUI_CAM_SMOOTH:
    {
        CameraFPS *cam = dynamic_cast<CameraFPS*>(Camera::getActiveCamera());
        if(cam)
        {
            cam->setSmoothMovement(!cam->getSmoothMovement());
        }
        break;
    }
    case DEBUG_GUI_CAM_ATTACH:
    {
        CameraFPS *cam = dynamic_cast<CameraFPS*>(Camera::getActiveCamera());
        if(cam)
        {
            cam->setAttachedFpsCam(!cam->getAttachedFpsCam());
        }
        break;
    }
    case DEBUG_VIEW_KART_ONE:
        changeCameraTarget(1);
        break;
    case DEBUG_VIEW_KART_TWO:
        changeCameraTarget(2);
        break;
    case DEBUG_VIEW_KART_THREE:
        changeCameraTarget(3);
        break;
    case DEBUG_VIEW_KART_FOUR:
        changeCameraTarget(4);
        break;
    case DEBUG_VIEW_KART_FIVE:
        changeCameraTarget(5);
        break;
    case DEBUG_VIEW_KART_SIX:
        changeCameraTarget(6);
        break;
    case DEBUG_VIEW_KART_SEVEN:
        changeCameraTarget(7);
        break;
    case DEBUG_VIEW_KART_EIGHT:
        changeCameraTarget(8);
        break;
    case DEBUG_PRINT_START_POS:
        if (!world) return false;
        for (unsigned int i = 0; i<world->getNumKarts(); i++)
        {
            AbstractKart *kart = world->getKart(i);
            Log::warn(kart->getIdent().c_str(),
                "<start position=\"%d\" x=\"%f\" y=\"%f\" z=\"%f\" h=\"%f\"/>",
                i, kart->getXYZ().getX(), kart->getXYZ().getY(),
                kart->getXYZ().getZ(), kart->getHeading()*RAD_TO_DEGREE
                );
        }
        break;
    case DEBUG_VISUAL_VALUES:
    {
        DebugSliderDialog *dsd = new DebugSliderDialog();
        dsd->setSliderHook("red_slider", 0, 255,
            [](){ return int(irr_driver->getAmbientLight().r * 255.f); },
            [](int v){
            video::SColorf ambient = irr_driver->getAmbientLight();
            ambient.setColorComponentValue(0, v / 255.f);
            irr_driver->setAmbientLight(ambient); }
        );
        dsd->setSliderHook("green_slider", 0, 255,
            [](){ return int(irr_driver->getAmbientLight().g * 255.f); },
            [](int v){
            video::SColorf ambient = irr_driver->getAmbientLight();
            ambient.setColorComponentValue(1, v / 255.f);
            irr_driver->setAmbientLight(ambient); }
        );
        dsd->setSliderHook("blue_slider", 0, 255,
            [](){ return int(irr_driver->getAmbientLight().b * 255.f); },
            [](int v){
            video::SColorf ambient = irr_driver->getAmbientLight();
            ambient.setColorComponentValue(2, v / 255.f);
            irr_driver->setAmbientLight(ambient); }
        );
        dsd->setSliderHook("ssao_radius", 0, 100,
            [](){ return int(irr_driver->getSSAORadius() * 10.f); },
            [](int v){irr_driver->setSSAORadius(v / 10.f); }
        );
        dsd->setSliderHook("ssao_k", 0, 100,
            [](){ return int(irr_driver->getSSAOK() * 10.f); },
            [](int v){irr_driver->setSSAOK(v / 10.f); }
        );
        dsd->setSliderHook("ssao_sigma", 0, 100,
            [](){ return int(irr_driver->getSSAOSigma() * 10.f); },
            [](int v){irr_driver->setSSAOSigma(v / 10.f); }
        );
    }
    break;
    case DEBUG_ADJUST_LIGHTS:
    {
        // Some sliders use multipliers because the spinner widget
        // only supports integers
        DebugSliderDialog *dsd = new DebugSliderDialog();
        dsd->changeLabel("Red", "Red (x10)");
        dsd->setSliderHook("red_slider", 0, 100,
            []()
            {
                return int(findNearestLight()->getColor().X * 100);
            },
            [](int intensity)
            {
                LightNode* nearest = findNearestLight();
                core::vector3df color = nearest->getColor();
                nearest->setColor(intensity / 100.0f, color.Y, color.Z);
            }
        );
        dsd->changeLabel("Green", "Green (x10)");
        dsd->setSliderHook("green_slider", 0, 100,
            []()
            {
                return int(findNearestLight()->getColor().Y * 100);
            },
            [](int intensity)
            {
                LightNode* nearest = findNearestLight();
                core::vector3df color = nearest->getColor();
                nearest->setColor(color.X, intensity / 100.0f, color.Z);
            }
        );
        dsd->changeLabel("Blue", "Blue (x10)");
        dsd->setSliderHook("blue_slider", 0, 100,
            []()
            {
                return int(findNearestLight()->getColor().Z * 100);
            },
            [](int intensity)
            {
                LightNode* nearest = findNearestLight();
                core::vector3df color = nearest->getColor();
                nearest->setColor(color.X, color.Y, intensity / 100.0f);
            }
        );
        dsd->changeLabel("SSAO radius", "energy (x10)");
        dsd->setSliderHook("ssao_radius", 0, 100,
            []()     { return int(findNearestLight()->getEnergy() * 10);  },
            [](int v){        findNearestLight()->setEnergy(v / 10.0f); }
        );
        dsd->changeLabel("SSAO k", "radius");
        dsd->setSliderHook("ssao_k", 0, 100,
            []()     { return int(findNearestLight()->getRadius());  },
            [](int v){        findNearestLight()->setRadius(float(v)); }
        );
        dsd->changeLabel("SSAO Sigma", "[None]");
        break;
    }
    case DEBUG_SCRIPT_CONSOLE:
        new GeneralTextFieldDialog(L"Run Script", []
            (const irr::core::stringw& text) {},
            [] (GUIEngine::LabelWidget* lw, GUIEngine::TextBoxWidget* tb)->bool
            {
                Scripting::ScriptEngine* engine =
                    Scripting::ScriptEngine::getInstance();
                if (engine == NULL)
                {
                    Log::warn("Debug", "No scripting engine loaded!");
                    return true;
                }
                engine->evalScript(StringUtils::wideToUtf8(tb->getText()));
                tb->setText(L"");
                // Don't close the console after each run
                return false;
            });
        break;
    case DEBUG_RUN_CUTSCENE:
        new GeneralTextFieldDialog(
            L"Enter the cutscene names (separate parts by space)", []
            (const irr::core::stringw& text)
            {
                if (World::getWorld())
                {
                    Log::warn("Debug", "Please run cutscene in main menu");
                    return;
                }
                if (text.empty()) return;
                std::vector<std::string> parts =
                    StringUtils::split(StringUtils::wideToUtf8(text), ' ');
                for (const std::string& track : parts)
                {
                    Track* t = track_manager->getTrack(track);
                    if (t == NULL)
                    {
                        Log::warn("Debug", "Cutscene %s not found!",
                            track.c_str());
                        return;
                    }
                }
                CutsceneWorld::setUseDuration(true);
                StateManager::get()->enterGameState();
                race_manager->setMinorMode(RaceManager::MINOR_MODE_CUTSCENE);
                race_manager->setNumKarts(0);
                race_manager->setNumPlayers(0);
                race_manager->startSingleRace(parts.front(), 999, false);
                ((CutsceneWorld*)World::getWorld())->setParts(parts);
            });
        break;
        case DEBUG_TEXTURE_CONSOLE:
        new GeneralTextFieldDialog(
            L"Enter the texture filename(s) (separate names by ;)"
            " to be reloaded (empty to reload all)\n"
            "Press tus; for texture usage stats (shown in console)", []
            (const irr::core::stringw& text) {},
            [] (GUIEngine::LabelWidget* lw, GUIEngine::TextBoxWidget* tb)->bool
            {
#ifndef SERVER_ONLY
                core::stringw t = tb->getText();
                STKTexManager* stktm = STKTexManager::getInstance();
                if (t == "tus;")
                {
                    stktm->dumpAllTexture(false/*mesh_texture*/);
                    stktm->dumpTextureUsage();
                    return false;
                }
                lw->setText(stktm->reloadTexture(t), true);
#endif
                // Don't close the dialog after each run
                return false;
            });
        break;
        case DEBUG_START_RECORDING:
            irr_driver->setRecording(true);
        break;
        case DEBUG_STOP_RECORDING:
            irr_driver->setRecording(false);
        break;
    }   // switch
    return false;
}

// -----------------------------------------------------------------------------
/** Debug menu handling */
bool onEvent(const SEvent &event)
{
    // Only activated in artist debug mode
    if(!UserConfigParams::m_artist_debug_mode)
        return true;    // keep handling the events

    if (event.EventType == EET_MOUSE_INPUT_EVENT)
    {
        // Create the menu (only one menu at a time)
        #ifdef ANDROID
        if (event.MouseInput.X < 30 && event.MouseInput.Y < 30 &&
        #else
        if (event.MouseInput.Event == EMIE_RMOUSE_PRESSED_DOWN &&
        #endif
            !g_debug_menu_visible)
        {
            irr_driver->getDevice()->getCursorControl()->setVisible(true);

            // root menu
            gui::IGUIEnvironment* guienv = irr_driver->getGUI();
            core::rect<s32> r(100, 50, 150, 500);
            IGUIContextMenu* mnu = guienv->addContextMenu(r, NULL);
            int graphicsMenuIndex = mnu->addItem(L"Graphics >",-1,true,true);

            // graphics menu
            IGUIContextMenu* sub = mnu->getSubMenu(graphicsMenuIndex);

            sub->addItem(L"Reload shaders", DEBUG_GRAPHICS_RELOAD_SHADERS );
            sub->addItem(L"Wireframe", DEBUG_GRAPHICS_WIREFRAME );
            sub->addItem(L"Mipmap viz", DEBUG_GRAPHICS_MIPMAP_VIZ );
            sub->addItem(L"Normals viz", DEBUG_GRAPHICS_NORMALS_VIZ );
            sub->addItem(L"SSAO viz", DEBUG_GRAPHICS_SSAO_VIZ );
            sub->addItem(L"RSM viz", DEBUG_GRAPHICS_RSM_VIZ);
            sub->addItem(L"RH viz", DEBUG_GRAPHICS_RH_VIZ);
            sub->addItem(L"GI viz", DEBUG_GRAPHICS_GI_VIZ);
            sub->addItem(L"Shadow viz", DEBUG_GRAPHICS_SHADOW_VIZ );
            sub->addItem(L"Light viz", DEBUG_GRAPHICS_LIGHT_VIZ );
            sub->addItem(L"Distort viz", DEBUG_GRAPHICS_DISTORT_VIZ );
            sub->addItem(L"Physics debug", DEBUG_GRAPHICS_BULLET_1);
            sub->addItem(L"Physics debug (no kart)", DEBUG_GRAPHICS_BULLET_2);
            sub->addItem(L"Bounding Boxes viz", DEBUG_GRAPHICS_BOUNDING_BOXES_VIZ);
            sub->addItem(L"Reset debug views", DEBUG_GRAPHICS_RESET );

            mnu->addItem(L"Items >",-1,true,true);
            sub = mnu->getSubMenu(1);
            sub->addItem(L"Basketball", DEBUG_POWERUP_RUBBERBALL );
            sub->addItem(L"Bowling", DEBUG_POWERUP_BOWLING );
            sub->addItem(L"Bubblegum", DEBUG_POWERUP_BUBBLEGUM );
            sub->addItem(L"Cake", DEBUG_POWERUP_CAKE );
            sub->addItem(L"Parachute", DEBUG_POWERUP_PARACHUTE );
            sub->addItem(L"Plunger", DEBUG_POWERUP_PLUNGER );
            sub->addItem(L"Swatter", DEBUG_POWERUP_SWATTER );
            sub->addItem(L"Switch", DEBUG_POWERUP_SWITCH );
            sub->addItem(L"Zipper", DEBUG_POWERUP_ZIPPER );
            sub->addItem(L"Nitro", DEBUG_POWERUP_NITRO );

            mnu->addItem(L"Attachments >",-1,true, true);
            sub = mnu->getSubMenu(2);
            sub->addItem(L"Bomb", DEBUG_ATTACHMENT_BOMB);
            sub->addItem(L"Anvil", DEBUG_ATTACHMENT_ANVIL);
            sub->addItem(L"Parachute", DEBUG_ATTACHMENT_PARACHUTE);
            sub->addItem(L"Explosion", DEBUG_ATTACHMENT_EXPLOSION);

            mnu->addItem(L"GUI >",-1,true, true);
            sub = mnu->getSubMenu(3);
            sub->addItem(L"Toggle GUI", DEBUG_GUI_TOGGLE);
            sub->addItem(L"Hide karts", DEBUG_GUI_HIDE_KARTS);
            sub->addItem(L"Top view", DEBUG_GUI_CAM_TOP);
            sub->addItem(L"Behind wheel view", DEBUG_GUI_CAM_WHEEL);
            sub->addItem(L"Behind kart view", DEBUG_GUI_CAM_BEHIND_KART);
            sub->addItem(L"Side of kart view", DEBUG_GUI_CAM_SIDE_OF_KART);
            sub->addItem(L"First person view (Ctrl + F1)", DEBUG_GUI_CAM_FREE);
            sub->addItem(L"Normal view (Ctrl + F2)", DEBUG_GUI_CAM_NORMAL);
            sub->addItem(L"Toggle smooth camera", DEBUG_GUI_CAM_SMOOTH);
            sub->addItem(L"Attach fps camera to kart", DEBUG_GUI_CAM_ATTACH);

            mnu->addItem(L"Recording >",-1,true, true);
            sub = mnu->getSubMenu(4);
            sub->addItem(L"Start recording", DEBUG_START_RECORDING);
            sub->addItem(L"Stop recording", DEBUG_STOP_RECORDING);

            mnu->addItem(L"Change camera target >",-1,true, true);
            sub = mnu->getSubMenu(5);
            sub->addItem(L"To kart one", DEBUG_VIEW_KART_ONE);
            sub->addItem(L"To kart two", DEBUG_VIEW_KART_TWO);
            sub->addItem(L"To kart three", DEBUG_VIEW_KART_THREE);
            sub->addItem(L"To kart four", DEBUG_VIEW_KART_FOUR);
            sub->addItem(L"To kart five", DEBUG_VIEW_KART_FIVE);
            sub->addItem(L"To kart six", DEBUG_VIEW_KART_SIX);
            sub->addItem(L"To kart seven", DEBUG_VIEW_KART_SEVEN);
            sub->addItem(L"To kart eight", DEBUG_VIEW_KART_EIGHT);

            mnu->addItem(L"Font >",-1,true, true);
            sub = mnu->getSubMenu(6);
            sub->addItem(L"Dump glyph pages of fonts", DEBUG_FONT_DUMP_GLYPH_PAGE);
            sub->addItem(L"Reload all fonts", DEBUG_FONT_RELOAD);

            mnu->addItem(L"Adjust values", DEBUG_VISUAL_VALUES);

            mnu->addItem(L"Profiler", DEBUG_PROFILER);
            if (UserConfigParams::m_profiler_enabled)
                mnu->addItem(L"Save profiler report",
                             DEBUG_PROFILER_WRITE_REPORT);
            mnu->addItem(L"Do not limit FPS", DEBUG_THROTTLE_FPS);
            mnu->addItem(L"Toggle FPS", DEBUG_FPS);
            mnu->addItem(L"Save replay", DEBUG_SAVE_REPLAY);
            mnu->addItem(L"Save history", DEBUG_SAVE_HISTORY);
            mnu->addItem(L"Print position", DEBUG_PRINT_START_POS);
            mnu->addItem(L"Adjust Lights", DEBUG_ADJUST_LIGHTS);
            mnu->addItem(L"Scripting console", DEBUG_SCRIPT_CONSOLE);
            mnu->addItem(L"Run cutscene(s)", DEBUG_RUN_CUTSCENE);
            mnu->addItem(L"Texture console", DEBUG_TEXTURE_CONSOLE);

            g_debug_menu_visible = true;
            irr_driver->showPointer();
        }

        // Let Irrlicht handle the event while the menu is visible.
        // Otherwise in a race the GUI events won't be generated
        if(g_debug_menu_visible)
            return false;
    }

    if (event.EventType == EET_GUI_EVENT)
    {
        if (event.GUIEvent.Caller != NULL &&
            event.GUIEvent.Caller->getType() == EGUIET_CONTEXT_MENU )
        {
            IGUIContextMenu *menu = (IGUIContextMenu*)event.GUIEvent.Caller;
            s32 cmdID = menu->getItemCommandId(menu->getSelectedItem());

            if (event.GUIEvent.EventType == EGET_ELEMENT_CLOSED)
            {
                g_debug_menu_visible = false;
            }
            else if (event.GUIEvent.EventType == gui::EGET_MENU_ITEM_SELECTED)
            {
                return handleContextMenuAction(cmdID);
            }
            return false;
        }
    }
    return true;    // continue event handling
}   // onEvent

// ----------------------------------------------------------------------------

bool handleStaticAction(int key)
{
    if (key == IRR_KEY_F1)
    {
        handleContextMenuAction(DEBUG_GUI_CAM_FREE);
    }
    else if (key == IRR_KEY_F2)
    {
        handleContextMenuAction(DEBUG_GUI_CAM_NORMAL);
    }
    else if (key == IRR_KEY_F3)
    {
        STKTexManager::getInstance()->reloadTexture("");
        return true;
    }
    // TODO: create more keyboard shortcuts

    return false;
}

// ----------------------------------------------------------------------------
/** Returns if the debug menu is visible.
 */
bool isOpen()
{
    return g_debug_menu_visible;
}   // isOpen

}  // namespace Debug
