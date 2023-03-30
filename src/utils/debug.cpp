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
#include "graphics/central_settings.hpp"
#include "graphics/shader_based_renderer.hpp"
#include "graphics/sp/sp_base.hpp"
#include "graphics/stk_text_billboard.hpp"
#include "karts/explosion_animation.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/light.hpp"
#include "graphics/shader.hpp"
#include "graphics/sp/sp_base.hpp"
#include "graphics/sp/sp_shader_manager.hpp"
#include "graphics/sp/sp_shader.hpp"
#include "graphics/sp/sp_texture_manager.hpp"
#include "graphics/sp/sp_uniform_assigner.hpp"
#include "graphics/stk_tex_manager.hpp"
#include "guiengine/modaldialog.hpp"
#include "guiengine/screen_keyboard.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"
#include "items/powerup.hpp"
#include "items/powerup_manager.hpp"
#include "items/attachment.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/kart_properties.hpp"
#include "karts/controller/controller.hpp"
#include "karts/rescue_animation.hpp"
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
#include "states_screens/dialogs/tutorial_message_dialog.hpp"
#include "tracks/track_manager.hpp"
#include "utils/constants.hpp"
#include "utils/log.hpp"
#include "utils/profiler.hpp"
#include "utils/string_utils.hpp"

#include <IrrlichtDevice.h>
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
    DEBUG_GRAPHICS_RELOAD_SHADERS,
    DEBUG_GRAPHICS_RELOAD_TEXTURES,
    DEBUG_GRAPHICS_RESET,
    DEBUG_GRAPHICS_SSAO_VIZ,
    DEBUG_GRAPHICS_SHADOW_VIZ,
    DEBUG_GRAPHICS_BOUNDING_BOXES_VIZ,
    DEBUG_GRAPHICS_BULLET_1,
    DEBUG_GRAPHICS_BULLET_2,
    DEBUG_RENDER_NW_DEBUG,
    DEBUG_PROFILER,
    DEBUG_PROFILER_WRITE_REPORT,
    DEBUG_FONT_DUMP_GLYPH_PAGE,
    DEBUG_FONT_RELOAD,
    DEBUG_SP_RESET,
    DEBUG_SP_TOGGLE_CULLING,
    DEBUG_SP_WN_VIZ,
    DEBUG_SP_NORMALS_VIZ,
    DEBUG_SP_TANGENTS_VIZ,
    DEBUG_SP_BITANGENTS_VIZ,
    DEBUG_SP_WIREFRAME_VIZ,
    DEBUG_SP_TN_VIZ,
    DEBUG_FPS,
    DEBUG_SAVE_REPLAY,
    DEBUG_SAVE_HISTORY,
    DEBUG_SAVE_SCREENSHOT,
    DEBUG_DUMP_RTT,
    DEBUG_POWERUP_ANVIL,
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
    DEBUG_POWERUP_NOTHING,
    DEBUG_NITRO_CLEAR,
    DEBUG_POWERUP_SLIDER,
    DEBUG_ATTACHMENT_PARACHUTE,
    DEBUG_ATTACHMENT_BOMB,
    DEBUG_ATTACHMENT_ANVIL,
    DEBUG_ATTACHMENT_SQUASH,
    DEBUG_ATTACHMENT_PLUNGER,
    DEBUG_ATTACHMENT_EXPLOSION,
    DEBUG_ATTACHMENT_NOTHING,
    DEBUG_GUI_TOGGLE,
    DEBUG_GUI_HIDE_KARTS,
    DEBUG_GUI_CAM_FREE,
    DEBUG_GUI_CAM_TOP,
    DEBUG_GUI_CAM_WHEEL,
    DEBUG_GUI_CAM_BEHIND_KART,
    DEBUG_GUI_CAM_SIDE_OF_KART,
    DEBUG_GUI_CAM_INV_SIDE_OF_KART,
    DEBUG_GUI_CAM_FRONT_OF_KART,
    DEBUG_GUI_CAM_NORMAL,
    DEBUG_GUI_CAM_SMOOTH,
    DEBUG_GUI_CAM_ATTACH,
    DEBUG_VIEW_KART_PREVIOUS,
    DEBUG_VIEW_KART_NEXT,
    DEBUG_VIEW_KART_ONE,
    DEBUG_VIEW_KART_TWO,
    DEBUG_VIEW_KART_THREE,
    DEBUG_VIEW_KART_FOUR,
    DEBUG_VIEW_KART_FIVE,
    DEBUG_VIEW_KART_SIX,
    DEBUG_VIEW_KART_SEVEN,
    DEBUG_VIEW_KART_EIGHT,
    DEBUG_VIEW_KART_NINE,
    DEBUG_VIEW_KART_LAST,
    DEBUG_VIEW_KART_SLIDER,
    DEBUG_HIDE_KARTS,
    DEBUG_RESCUE_KART,
    DEBUG_PAUSE,
    DEBUG_THROTTLE_FPS,
    DEBUG_VISUAL_VALUES,
    DEBUG_PRINT_START_POS,
    DEBUG_ADJUST_LIGHTS,
    DEBUG_SCRIPT_CONSOLE,
    DEBUG_RUN_CUTSCENE,
    DEBUG_TEXTURE_CONSOLE,
    DEBUG_START_RECORDING,
    DEBUG_STOP_RECORDING,
    DEBUG_HELP
};   // DebugMenuCommand

// -----------------------------------------------------------------------------
/** Add powerup selected from debug menu for all player karts */
void addPowerup(PowerupManager::PowerupType powerup, int amount)
{
    World* world = World::getWorld();
    if (!world) return;
    for(unsigned int i = 0; i < RaceManager::get()->getNumLocalPlayers(); i++)
    {
        AbstractKart* kart = world->getLocalPlayerKart(i);
        kart->setPowerup(powerup, amount);
    }
}   // addPowerup

// ----------------------------------------------------------------------------
void setNitro(int amount)
{
    World* world = World::getWorld();
    if (!world) return;
    const unsigned int num_local_players =
        RaceManager::get()->getNumLocalPlayers();
    for (unsigned int i = 0; i < num_local_players; i++)
    {
        AbstractKart* kart = world->getLocalPlayerKart(i);
        kart->setEnergy(amount);
    }
}   // setNitro

// ----------------------------------------------------------------------------
void addAttachment(Attachment::AttachmentType type)
{
    World* world = World::getWorld();
    if (world == NULL) return;
    for (unsigned int i = 0; i < world->getNumKarts(); i++)
    {
        AbstractKart *kart = world->getLocalPlayerKart(i);
        if (kart == NULL)
           continue;
        //if (!kart->getController()->isLocalPlayerController())
        //    continue;
        if (type == Attachment::ATTACH_ANVIL)
        {
            kart->getAttachment()
                ->set(type,
                      stk_config->time2Ticks(kart->getKartProperties()
                                                 ->getAnvilDuration()) );
            kart->adjustSpeed(kart->getKartProperties()->getAnvilSpeedFactor());
        }
        else if (type == Attachment::ATTACH_PARACHUTE)
        {
            kart->getAttachment()
                ->set(type, stk_config->time2Ticks(
                kart->getKartProperties()->getParachuteDuration()));
        }
        else if (type == Attachment::ATTACH_BOMB)
        {
            kart->getAttachment()
                ->set(type, stk_config->time2Ticks(stk_config->m_bomb_time) );
        }
        else if (type == Attachment::ATTACH_NOTHING)
        {
            kart->getAttachment()
                ->reset();
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
    if (cmd_id == -1)
        return false;

    Camera* camera = Camera::getActiveCamera();
    unsigned int kart_num = 0;
    if (camera != NULL && camera->getKart() != NULL)
    {
        kart_num = camera->getKart()->getWorldKartId();
    }

    World *world = World::getWorld();
    Physics *physics = Physics::get();
    SP::SPShader* nv = NULL;
#ifndef SERVER_ONLY
    if (SP::getNormalVisualizer())
    {
        nv = SP::getNormalVisualizer();
    }
#endif

    switch(cmd_id)
    {
    case DEBUG_GRAPHICS_RELOAD_SHADERS:
#ifndef SERVER_ONLY
        Log::info("Debug", "Reloading shaders...");
        SP::SPShaderManager::get()->unloadAll();
        ShaderBase::killShaders();
        ShaderFilesManager::getInstance()->removeAllShaderFiles();
        SP::SPShaderManager::get()->initAll();
#endif
        break;
    case DEBUG_GRAPHICS_RELOAD_TEXTURES:
#ifndef SERVER_ONLY
        SP::SPTextureManager::get()->reloadTexture("");
#endif
        break;
    case DEBUG_GRAPHICS_RESET:
        if (physics)
            physics->setDebugMode(IrrDebugDrawer::DM_NONE);

        irr_driver->resetDebugModes();
        break;
    case DEBUG_GRAPHICS_SSAO_VIZ:
        if (physics)
            physics->setDebugMode(IrrDebugDrawer::DM_NONE);

        irr_driver->resetDebugModes();
        irr_driver->toggleSSAOViz();
        break;
    case DEBUG_GRAPHICS_SHADOW_VIZ:
        if (physics)
            physics->setDebugMode(IrrDebugDrawer::DM_NONE);

        irr_driver->resetDebugModes();
        irr_driver->toggleShadowViz();
        break;
    case DEBUG_GRAPHICS_BOUNDING_BOXES_VIZ:
        if (physics)
            physics->setDebugMode(IrrDebugDrawer::DM_NONE);

        irr_driver->resetDebugModes();
        irr_driver->toggleBoundingBoxesViz();
        break;
    case DEBUG_GRAPHICS_BULLET_1:
        irr_driver->resetDebugModes();

        if (!world) return false;
        physics->setDebugMode(IrrDebugDrawer::DM_KARTS_PHYSICS);
        break;
    case DEBUG_GRAPHICS_BULLET_2:
    {
        irr_driver->resetDebugModes();

        Physics *physics = Physics::get();
        if (!physics) return false;
        physics->setDebugMode(IrrDebugDrawer::DM_NO_KARTS_GRAPHICS);
        break;
    }
    case DEBUG_SP_RESET:
        irr_driver->resetDebugModes();
        if (physics)
            physics->setDebugMode(IrrDebugDrawer::DM_NONE);
#ifndef SERVER_ONLY
        SP::sp_culling = true;
#endif
        break;
    case DEBUG_SP_TOGGLE_CULLING:
#ifndef SERVER_ONLY
        SP::sp_culling = !SP::sp_culling;
#endif
        break;
    case DEBUG_SP_WN_VIZ:
        irr_driver->resetDebugModes();
        if (physics)
            physics->setDebugMode(IrrDebugDrawer::DM_NONE);
#ifndef SERVER_ONLY
        SP::sp_debug_view = true;
#endif
        break;
    case DEBUG_SP_NORMALS_VIZ:
    {
        irr_driver->resetDebugModes();
        if (physics)
            physics->setDebugMode(IrrDebugDrawer::DM_NONE);
#ifndef SERVER_ONLY
        SP::sp_debug_view = true;
        int normal = 0;
        if (nv)
        {
            SP::SPUniformAssigner* ua = nv->getUniformAssigner("enable_normals");
            if (ua)
            {
                ua->getValue(nv->getShaderProgram(SP::RP_1ST), normal);
                normal = normal == 0 ? 1 : 0;
                nv->use();
                ua->setValue(normal);
                glUseProgram(0);
            }
        }
#endif
        break;
    }
    case DEBUG_SP_TANGENTS_VIZ:
    {
        irr_driver->resetDebugModes();
        if (physics)
            physics->setDebugMode(IrrDebugDrawer::DM_NONE);
#ifndef SERVER_ONLY
        SP::sp_debug_view = true;
        int tangents = 0;
        if (nv)
        {
            SP::SPUniformAssigner* ua = nv->getUniformAssigner("enable_tangents");
            if (ua)
            {
                ua->getValue(nv->getShaderProgram(SP::RP_1ST), tangents);
                tangents = tangents == 0 ? 1 : 0;
                nv->use();
                ua->setValue(tangents);
                glUseProgram(0);
            }
        }
#endif
        break;
    }
    case DEBUG_SP_BITANGENTS_VIZ:
    {
        irr_driver->resetDebugModes();
        if (physics)
            physics->setDebugMode(IrrDebugDrawer::DM_NONE);
#ifndef SERVER_ONLY
        SP::sp_debug_view = true;
        int bitangents = 0;
        if (nv)
        {
            SP::SPUniformAssigner* ua = nv->getUniformAssigner("enable_bitangents");
            if (ua)
            {
                ua->getValue(nv->getShaderProgram(SP::RP_1ST), bitangents);
                bitangents = bitangents == 0 ? 1 : 0;
                nv->use();
                ua->setValue(bitangents);
                glUseProgram(0);
            }
        }
#endif
        break;
    }
    case DEBUG_SP_WIREFRAME_VIZ:
    {
        irr_driver->resetDebugModes();
        if (physics)
            physics->setDebugMode(IrrDebugDrawer::DM_NONE);
#ifndef SERVER_ONLY
        SP::sp_debug_view = true;
        int wireframe = 0;
        if (nv)
        {
            SP::SPUniformAssigner* ua = nv->getUniformAssigner("enable_wireframe");
            if (ua)
            {
                ua->getValue(nv->getShaderProgram(SP::RP_1ST), wireframe);
                wireframe = wireframe == 0 ? 1 : 0;
                nv->use();
                ua->setValue(wireframe);
                glUseProgram(0);
            }
        }
#endif
        break;
    }
    case DEBUG_SP_TN_VIZ:
    {
        irr_driver->resetDebugModes();
        if (physics)
            physics->setDebugMode(IrrDebugDrawer::DM_NONE);
#ifndef SERVER_ONLY
        SP::sp_debug_view = true;
        int triangle_normals = 0;
        if (nv)
        {
            SP::SPUniformAssigner* ua = nv->getUniformAssigner("enable_triangle_normals");
            if (ua)
            {
                ua->getValue(nv->getShaderProgram(SP::RP_1ST), triangle_normals);
                triangle_normals = triangle_normals == 0 ? 1 : 0;
                nv->use();
                ua->setValue(triangle_normals);
                glUseProgram(0);
            }
        }
#endif
        break;
    }
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
#ifndef SERVER_ONLY
        STKTextBillboard::updateAllTextBillboards();
#endif
        break;
    case DEBUG_FPS:
        UserConfigParams::m_display_fps = !UserConfigParams::m_display_fps;
        break;
    case DEBUG_SAVE_REPLAY:
        if (!world) return false;
        ReplayRecorder::get()->save();
        break;
    case DEBUG_SAVE_HISTORY:
        if (!world) return false;
        history->Save();
        break;
    case DEBUG_POWERUP_ANVIL:
        addPowerup(PowerupManager::POWERUP_ANVIL, 255);
        break;
    case DEBUG_POWERUP_BOWLING:
        addPowerup(PowerupManager::POWERUP_BOWLING, 255);
        break;
    case DEBUG_POWERUP_BUBBLEGUM:
        addPowerup(PowerupManager::POWERUP_BUBBLEGUM, 255);
        break;
    case DEBUG_POWERUP_CAKE:
        addPowerup(PowerupManager::POWERUP_CAKE, 255);
        break;
    case DEBUG_POWERUP_PARACHUTE:
        addPowerup(PowerupManager::POWERUP_PARACHUTE, 255);
        break;
    case DEBUG_POWERUP_PLUNGER:
        addPowerup(PowerupManager::POWERUP_PLUNGER, 255);
        break;
    case DEBUG_POWERUP_RUBBERBALL:
        addPowerup(PowerupManager::POWERUP_RUBBERBALL, 255);
        break;
    case DEBUG_POWERUP_SWATTER:
        addPowerup(PowerupManager::POWERUP_SWATTER, 255);
        break;
    case DEBUG_POWERUP_SWITCH:
        addPowerup(PowerupManager::POWERUP_SWITCH, 255);
        break;
    case DEBUG_POWERUP_ZIPPER:
        addPowerup(PowerupManager::POWERUP_ZIPPER, 255);
        break;
    case DEBUG_POWERUP_NITRO:
    {
        setNitro(100.0f);
        break;
    }
    case DEBUG_POWERUP_NOTHING:
        addPowerup(PowerupManager::POWERUP_NOTHING, 0);
        break;
    case DEBUG_NITRO_CLEAR:
    {
        setNitro(0.0f);
        break;
    }
    case DEBUG_POWERUP_SLIDER:
    {
        if (!world) return false;
        DebugSliderDialog *dsd = new DebugSliderDialog();
        dsd->changeLabel("Red", "Powerup count");
        dsd->setSliderHook("red_slider", 0, 255,
            [](){ return Camera::getActiveCamera()->getKart()->getNumPowerup(); },
            [](int new_pw_amt){Camera::getActiveCamera()->getKart()->
               getPowerup()->setNum(new_pw_amt); }
        );
        dsd->changeLabel("Green", "Nitro amount");
        dsd->setSliderHook("green_slider", 0, 100,
            [](){ return Camera::getActiveCamera()->getKart()->getEnergy(); },
            [](float new_nit_amt){Camera::getActiveCamera()->getKart()->setEnergy(new_nit_amt); }
        );
        dsd->changeLabel("Blue", "[None]");
        dsd->toggleSlider("blue_slider", false);
        dsd->changeLabel("SSAO radius", "[None]");
        dsd->toggleSlider("ssao_radius", false);
        dsd->changeLabel("SSAO k", "[None]");
        dsd->toggleSlider("ssao_k", false);
        dsd->changeLabel("SSAO Sigma", "[None]");
        dsd->toggleSlider("ssao_sigma", false);
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
     case DEBUG_ATTACHMENT_SQUASH:
        for (unsigned int i = 0; i < RaceManager::get()->getNumLocalPlayers(); i++)
        {
            AbstractKart* kart = world->getLocalPlayerKart(i);
            const KartProperties *kp = kart->getKartProperties();
            kart->setSquash(kp->getSwatterSquashDuration(), kp->getSwatterSquashSlowdown());
        }
        break;
    case DEBUG_ATTACHMENT_PLUNGER:
        for (unsigned int i = 0; i < RaceManager::get()->getNumLocalPlayers(); i++)
        {
            AbstractKart* kart = world->getLocalPlayerKart(i);
            kart->blockViewWithPlunger();
        }
        break;
    case DEBUG_ATTACHMENT_EXPLOSION:
        for (unsigned int i = 0; i < RaceManager::get()->getNumLocalPlayers(); i++)
        {
            AbstractKart* kart = world->getLocalPlayerKart(i);
            ExplosionAnimation::create(kart, kart->getXYZ(), true);
        }
        break;
    case DEBUG_ATTACHMENT_NOTHING:
        addAttachment(Attachment::ATTACH_NOTHING);
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
    case DEBUG_RESCUE_KART:
        if (!world) return false;
        for (unsigned int i = 0; i < RaceManager::get()->getNumLocalPlayers(); i++)
        {
            AbstractKart* kart = world->getLocalPlayerKart(i);
            RescueAnimation::create(kart);
        }
        break;
    case DEBUG_PAUSE:
        if (!world) return false;
        world->escapePressed();
        break;
    case DEBUG_GUI_CAM_TOP:
        CameraDebug::setDebugType(CameraDebug::CM_DEBUG_TOP_OF_KART);
        Camera::changeCamera(0, Camera::CM_TYPE_DEBUG);
        Camera::getActiveCamera()->setKart(World::getWorld()->getKart(kart_num));
        irr_driver->getDevice()->getCursorControl()->setVisible(true);
        break;
    case DEBUG_GUI_CAM_WHEEL:
        if (!(World::getWorld()->getKart(kart_num)->isGhostKart()))
        {
            CameraDebug::setDebugType(CameraDebug::CM_DEBUG_GROUND);
            Camera::changeCamera(0, Camera::CM_TYPE_DEBUG);
            Camera::getActiveCamera()->setKart(World::getWorld()->getKart(kart_num));
            irr_driver->getDevice()->getCursorControl()->setVisible(true);
        }
        break;
    case DEBUG_GUI_CAM_BEHIND_KART:
        CameraDebug::setDebugType(CameraDebug::CM_DEBUG_BEHIND_KART);
        Camera::changeCamera(0, Camera::CM_TYPE_DEBUG);
        Camera::getActiveCamera()->setKart(World::getWorld()->getKart(kart_num));
        irr_driver->getDevice()->getCursorControl()->setVisible(true);
        break;
    case DEBUG_GUI_CAM_SIDE_OF_KART:
        CameraDebug::setDebugType(CameraDebug::CM_DEBUG_SIDE_OF_KART);
        Camera::changeCamera(0, Camera::CM_TYPE_DEBUG);
        Camera::getActiveCamera()->setKart(World::getWorld()->getKart(kart_num));
        irr_driver->getDevice()->getCursorControl()->setVisible(true);
        break;
    case DEBUG_GUI_CAM_INV_SIDE_OF_KART:
        CameraDebug::setDebugType(CameraDebug::CM_DEBUG_INV_SIDE_OF_KART);
        Camera::changeCamera(0, Camera::CM_TYPE_DEBUG);
        Camera::getActiveCamera()->setKart(World::getWorld()->getKart(kart_num));
        irr_driver->getDevice()->getCursorControl()->setVisible(true);
        break;
    case DEBUG_GUI_CAM_FRONT_OF_KART:
        CameraDebug::setDebugType(CameraDebug::CM_DEBUG_FRONT_OF_KART);
        Camera::changeCamera(0, Camera::CM_TYPE_DEBUG);
        Camera::getActiveCamera()->setKart(World::getWorld()->getKart(kart_num));
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
        Camera::getActiveCamera()->setKart(World::getWorld()->getKart(kart_num));
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
    case DEBUG_VIEW_KART_PREVIOUS:
    {
        if (kart_num == 0)
        {
            kart_num += World::getWorld()->getNumKarts() - 1;
        }
        else
        {
            kart_num--;
        }
        Camera::getActiveCamera()->setKart(World::getWorld()->getKart(kart_num));
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
    case DEBUG_VIEW_KART_NINE:
        changeCameraTarget(9);
        break;
    case DEBUG_VIEW_KART_LAST:
        if (!world) return false;
        changeCameraTarget(World::getWorld()->getNumKarts());
        break;
    case DEBUG_VIEW_KART_NEXT:
    {
        if (kart_num == World::getWorld()->getNumKarts() - 1)
        {
            kart_num = 0;
        }
        else
        {
             kart_num++;
        }
        Camera::getActiveCamera()->setKart(World::getWorld()->getKart(kart_num));
        break;
    }
    case DEBUG_VIEW_KART_SLIDER:
    {
        if (!world) return false;
        DebugSliderDialog *dsd = new DebugSliderDialog();
        dsd->changeLabel("Red", "Kart number");
        dsd->setSliderHook("red_slider", 0, World::getWorld()->getNumKarts() - 1,
            [](){ return Camera::getActiveCamera()->getKart()->getWorldKartId(); },
            [](int new_kart_num){Camera::getActiveCamera()->
            setKart(World::getWorld()->getKart(new_kart_num)); }
        );
        dsd->changeLabel("Green", "[None]");
        dsd->toggleSlider("green_slider", false);
        dsd->changeLabel("Blue", "[None]");
        dsd->toggleSlider("blue_slider", false);
        dsd->changeLabel("SSAO radius", "[None]");
        dsd->toggleSlider("ssao_radius", false);
        dsd->changeLabel("SSAO k", "[None]");
        dsd->toggleSlider("ssao_k", false);
        dsd->changeLabel("SSAO Sigma", "[None]");
        dsd->toggleSlider("ssao_sigma", false);
        break;
    }

    case DEBUG_PRINT_START_POS:
        if (!world) return false;
        for (unsigned int i = 0; i<world->getNumKarts(); i++)
        {
            AbstractKart *kart = world->getKart(i);
            Log::info(kart->getIdent().c_str(),
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
        if (!world) return false;
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
        dsd->toggleSlider("ssao_sigma", false);
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
                RaceManager::get()->setMinorMode(RaceManager::MINOR_MODE_CUTSCENE);
                RaceManager::get()->setNumKarts(0);
                RaceManager::get()->setNumPlayers(0);
                RaceManager::get()->startSingleRace(parts.front(), 999, false);
                ((CutsceneWorld*)World::getWorld())->setParts(parts);
            });
        break;
    case DEBUG_TEXTURE_CONSOLE:
        new GeneralTextFieldDialog(
            L"Enter the texture filename(s) (separate names by ;)"
            " to be reloaded (empty to reload all)\n"
            "Press tus; for showing all mesh textures (shown in console)", []
            (const irr::core::stringw& text) {},
            [] (GUIEngine::LabelWidget* lw, GUIEngine::TextBoxWidget* tb)->bool
            {
#ifndef SERVER_ONLY
                core::stringw t = tb->getText();
                SP::SPTextureManager* sptm = SP::SPTextureManager::get();
                if (t == "tus;")
                {
                    if (CVS->isGLSL())
                        sptm->dumpAllTextures();
                    else
                    {
                        for (auto& p : STKTexManager::getInstance()->getAllTextures())
                            Log::info("STKTexManager", "%s", p.first.c_str());
                        STKTexManager::getInstance()->dumpTextureUsage();
                    }
                    return false;
                }
                if (t.empty())
                    STKTexManager::getInstance()->reloadAllTextures();
                lw->setText(sptm->reloadTexture(t), true);
#endif
                // Don't close the dialog after each run
                return false;
            });
        break;
    case DEBUG_RENDER_NW_DEBUG:
        irr_driver->toggleRenderNetworkDebug();
        break;
    case DEBUG_DUMP_RTT:
    {
        if (!world) return false;
#ifndef SERVER_ONLY
        ShaderBasedRenderer* sbr = SP::getRenderer();
        if (sbr)
        {
            sbr->dumpRTT();
        }
#endif
        break;
    }
    case DEBUG_SAVE_SCREENSHOT:
        irr_driver->requestScreenshot();
        break;
    case DEBUG_START_RECORDING:
        irr_driver->setRecording(true);
        break;
    case DEBUG_STOP_RECORDING:
        irr_driver->setRecording(false);
        break;
    case DEBUG_HELP:
        new TutorialMessageDialog(L"Debug keyboard shortcuts (can conflict with user-defined shortcuts):\n"
                            "* <~> - Show this help dialog | + <Ctrl> - Adjust lights | + <Shift> - Adjust visuals\n"
                            "* <F1> - Anvil powerup | + <Ctrl> - Normal view | + <Shift> - Bomb attachment\n"
                            "* <F2> - Basketball powerup | + <Ctrl> - First person view | + <Shift> - Anvil attachment\n"
                            "* <F3> - Bowling ball powerup | + <Ctrl> - Top view | + <Shift> - Parachute attachment\n"
                            "* <F4> - Bubblegum powerup | + <Ctrl> - Behind wheel view | + <Shift> - Flatten kart\n"
                            "* <F5> - Cake powerup | + <Ctrl> - Behind kart view | + <Shift> - Send plunger to kart front\n"
                            "* <F6> - Parachute powerup | + <Ctrl> - Right side of kart view | + <Shift> - Explode kart\n"
                            "* <F7> - Plunger powerup | + <Ctrl> - Left side of kart view | + <Shift> - Scripting console\n"
                            "* <F8> - Swatter powerup | + <Ctrl> - Front of kart view | + <Shift> - Texture console\n"
                            "* <F9> - Switch powerup | + <Ctrl> - Kart number slider | + <Shift> - Run cutscene(s)\n"
                            "* <F10> - Zipper powerup | + <Ctrl> - Powerup amount slider | + <Shift> - Toggle GUI\n"
                            "* <F11> - Save replay | + <Ctrl> - Save history | + <Shift> - Dump RTT\n"
                            "* <F12> - Show FPS | + <Ctrl> - Show other karts' powerups | + <Shift> - Show soccer player list\n"
                            "* <Insert> - Overfilled nitro\n"
                            "* <Delete> - Clear kart items\n"
                            "* <Home> - First kart\n"
                            "* <End> - Last kart\n"
                            "* <Page Up> - Previous kart\n"
                            "* <Page Down> - Next kart"
                            , World::getWorld() && World::getWorld()->isNetworkWorld() ? false : true);
        break;
    }
    return false;
}

// -----------------------------------------------------------------------------
/** Debug menu handling */
bool onEvent(const SEvent &event)
{
    // Only activated in artist debug mode
    if(!UserConfigParams::m_artist_debug_mode)
        return true;    // keep handling the events

    if (event.EventType == EET_MOUSE_INPUT_EVENT || event.EventType == EET_TOUCH_INPUT_EVENT)
    {
        if (GUIEngine::ModalDialog::isADialogActive() ||
            GUIEngine::ScreenKeyboard::isActive())
            return true;

        // Create the menu (only one menu at a time)
        #if defined(MOBILE_STK) || defined(__SWITCH__)
        #ifdef __SWITCH__
        int x = 100;
        int y = 100;
        #else
        int x = 10 * irr_driver->getActualScreenSize().Height / 480;
        int y = 30 * irr_driver->getActualScreenSize().Height / 480;
        #endif // __SWITCH__
        if ( event.EventType == EET_MOUSE_INPUT_EVENT ?
            (event.MouseInput.X < x && event.MouseInput.Y < y) :
            (event.TouchInput.X < x && event.TouchInput.Y < y) &&
        #else // MOBILE_STK
        if ( event.EventType == EET_MOUSE_INPUT_EVENT &&
            (event.MouseInput.Event == EMIE_RMOUSE_PRESSED_DOWN ||
             event.MouseInput.Event == EMIE_MMOUSE_PRESSED_DOWN) &&
        #endif
            !g_debug_menu_visible)
        {
            irr_driver->getDevice()->getCursorControl()->setVisible(true);

            // root menu
            const int mwidth = 400;
            const int mheight = 600;

            // Adapt the position boundaries of the debug menu based on the
            // current screen and font sizes
            int boundx, boundy;
            float scalex, scaley;
            switch (int(UserConfigParams::m_font_size))
            {
                case 0: // Extremely small font
                {
                    scalex = 8.5;
                    scaley = 2.62;
                    break;
                }
                case 1: // Very small font
                {
                    scalex = 6.75;
                    scaley = 2.1;
                    break;
                }
                case 2: // Small font
                {
                    scalex = 5.75;
                    scaley = 1.7;
                    break;
                }
                case 3: // Medium font
                {
                    scalex = 5.0;
                    scaley = 1.5;
                    break;
                }
                case 4: // Large font
                {
                    scalex = 4.5;
                    scaley = 1.31;
                    break;
                }
                case 5: // Very large font
                {
                    scalex = 4.0;
                    scaley = 1.14;
                    break;
                }
                case 6: // Extremely large font
                {
                    scalex = 3.6;
                    scaley = 1.01;
                    break;
                }
                default: // Medium font
                {
                    scalex = 5.0;
                    scaley = 1.5;
                    break;
                }
            }

            boundx = irr_driver->getActualScreenSize().Width -
            int(irr_driver->getActualScreenSize().Width / scalex);
            boundy = irr_driver->getActualScreenSize().Height -
            int(irr_driver->getActualScreenSize().Height / scaley);

            gui::IGUIEnvironment* guienv = irr_driver->getGUI();
            core::rect<s32> position;
            if (event.MouseInput.X > boundx && event.MouseInput.Y > boundy)
            {
                position = core::rect<s32>(boundx, boundy, mwidth, mheight);
            }
            else if (event.MouseInput.X > boundx)
            {
                position = core::rect<s32>(boundx, event.MouseInput.Y, mwidth, mheight);
            }
            else if (event.MouseInput.Y > boundy)
            {
                position = core::rect<s32>(event.MouseInput.X, boundy, mwidth, mheight);
            }
            else
            {
                position = core::rect<s32>(event.MouseInput.X, event.MouseInput.Y, mwidth, mheight);
            }
            IGUIContextMenu* mnu = guienv->addContextMenu(position, NULL);

            int graphicsMenuIndex = mnu->addItem(L"Graphics >",-1,true,true);

            IGUIContextMenu* sub = mnu->getSubMenu(graphicsMenuIndex);

            sub->addItem(L"Reload shaders", DEBUG_GRAPHICS_RELOAD_SHADERS);
            sub->addItem(L"Reload textures", DEBUG_GRAPHICS_RELOAD_TEXTURES);
            sub->addSeparator();
            sub->addItem(L"SSAO viz", DEBUG_GRAPHICS_SSAO_VIZ);
            sub->addItem(L"Shadow viz", DEBUG_GRAPHICS_SHADOW_VIZ);
            sub->addItem(L"Bounding Boxes viz", DEBUG_GRAPHICS_BOUNDING_BOXES_VIZ);
            sub->addItem(L"Physics debug", DEBUG_GRAPHICS_BULLET_1);
            sub->addItem(L"Physics debug (no kart)", DEBUG_GRAPHICS_BULLET_2);
            sub->addItem(L"Network debugging", DEBUG_RENDER_NW_DEBUG);
            sub->addSeparator();
            sub->addItem(L"Reset debug views", DEBUG_GRAPHICS_RESET);

            mnu->addItem(L"Set camera target >",-1,true, true);
            sub = mnu->getSubMenu(1);
            sub->addItem(L"Pick kart from slider (Ctrl + F9)", DEBUG_VIEW_KART_SLIDER);
            sub->addSeparator();
            sub->addItem(L"To previous kart (Page Up)", DEBUG_VIEW_KART_PREVIOUS);
            sub->addItem(L"To next kart (Page Down)", DEBUG_VIEW_KART_NEXT);
            sub->addSeparator();
            sub->addItem(L"To kart 1 (Home)", DEBUG_VIEW_KART_ONE);
            sub->addItem(L"To kart 2", DEBUG_VIEW_KART_TWO);
            sub->addItem(L"To kart 3", DEBUG_VIEW_KART_THREE);
            sub->addItem(L"To kart 4", DEBUG_VIEW_KART_FOUR);
            sub->addItem(L"To kart 5", DEBUG_VIEW_KART_FIVE);
            sub->addItem(L"To kart 6", DEBUG_VIEW_KART_SIX);
            sub->addItem(L"To kart 7", DEBUG_VIEW_KART_SEVEN);
            sub->addItem(L"To kart 8", DEBUG_VIEW_KART_EIGHT);
            sub->addItem(L"To kart 9", DEBUG_VIEW_KART_NINE);
            sub->addItem(L"To last kart (End)", DEBUG_VIEW_KART_LAST);

            mnu->addItem(L"GUI >",-1,true, true);
            sub = mnu->getSubMenu(2);
            sub->addItem(L"Toggle GUI (Shift + F10)", DEBUG_GUI_TOGGLE);
            sub->addItem(L"Hide karts", DEBUG_GUI_HIDE_KARTS);
            sub->addSeparator();
            sub->addItem(L"Normal view (Ctrl + F1)", DEBUG_GUI_CAM_NORMAL);
            sub->addItem(L"First person view (Ctrl + F2)", DEBUG_GUI_CAM_FREE);
            sub->addItem(L"Top view (Ctrl + F3)", DEBUG_GUI_CAM_TOP);
            sub->addItem(L"Behind wheel view (Ctrl + F4)", DEBUG_GUI_CAM_WHEEL);
            sub->addItem(L"Behind kart view (Ctrl + F5)", DEBUG_GUI_CAM_BEHIND_KART);
            sub->addItem(L"Right side of kart view (Ctrl + F6)", DEBUG_GUI_CAM_SIDE_OF_KART);
            sub->addItem(L"Left side of kart view (Ctrl + F7)", DEBUG_GUI_CAM_INV_SIDE_OF_KART);
            sub->addItem(L"Front of kart view (Ctrl + F8)", DEBUG_GUI_CAM_FRONT_OF_KART);

            sub->addSeparator();
            sub->addItem(L"Toggle smooth camera", DEBUG_GUI_CAM_SMOOTH);
            sub->addItem(L"Attach fps camera to kart", DEBUG_GUI_CAM_ATTACH);

            mnu->addItem(L"Items >",-1,true,true);
            sub = mnu->getSubMenu(3);
            sub->addItem(L"Anvil (F1)", DEBUG_POWERUP_ANVIL );
            sub->addItem(L"Basketball (F2)", DEBUG_POWERUP_RUBBERBALL );
            sub->addItem(L"Bowling (F3)", DEBUG_POWERUP_BOWLING );
            sub->addItem(L"Bubblegum (F4)", DEBUG_POWERUP_BUBBLEGUM );
            sub->addItem(L"Cake (F5)", DEBUG_POWERUP_CAKE );
            sub->addItem(L"Parachute (F6)", DEBUG_POWERUP_PARACHUTE );
            sub->addItem(L"Plunger (F7)", DEBUG_POWERUP_PLUNGER );
            sub->addItem(L"Swatter (F8)", DEBUG_POWERUP_SWATTER );
            sub->addItem(L"Switch (F9)", DEBUG_POWERUP_SWITCH );
            sub->addItem(L"Zipper (F10)", DEBUG_POWERUP_ZIPPER );
            sub->addItem(L"Nitro (Insert)", DEBUG_POWERUP_NITRO );

            mnu->addItem(L"Attachments >",-1,true, true);
            sub = mnu->getSubMenu(4);
            sub->addItem(L"Bomb (Shift + F1)", DEBUG_ATTACHMENT_BOMB);
            sub->addItem(L"Anvil (Shift + F2)", DEBUG_ATTACHMENT_ANVIL);
            sub->addItem(L"Parachute (Shift + F3)", DEBUG_ATTACHMENT_PARACHUTE);
            sub->addItem(L"Flatten (Shift + F4)", DEBUG_ATTACHMENT_SQUASH);
            sub->addItem(L"Plunger (Shift + F5)", DEBUG_ATTACHMENT_PLUNGER);
            sub->addItem(L"Explosion (Shift + F6)", DEBUG_ATTACHMENT_EXPLOSION);

            mnu->addItem(L"Modify kart items >",-1,true, true);
            sub = mnu->getSubMenu(5);
            sub->addItem(L"Adjust with slider (Ctrl + F10)", DEBUG_POWERUP_SLIDER);
            sub->addSeparator();
            sub->addItem(L"Clear powerup (Delete)", DEBUG_POWERUP_NOTHING );
            sub->addItem(L"Clear nitro (Delete)", DEBUG_NITRO_CLEAR );
            sub->addItem(L"Clear attachment (Delete)", DEBUG_ATTACHMENT_NOTHING);

            mnu->addItem(L"SP debug >",-1,true, true);
            sub = mnu->getSubMenu(6);
            sub->addItem(L"Reset SP debug", DEBUG_SP_RESET);
            sub->addItem(L"Toggle culling", DEBUG_SP_TOGGLE_CULLING);
            sub->addItem(L"Draw world normal in texture", DEBUG_SP_WN_VIZ);
            sub->addItem(L"Toggle normals visualization", DEBUG_SP_NORMALS_VIZ);
            sub->addItem(L"Toggle tangents visualization", DEBUG_SP_TANGENTS_VIZ);
            sub->addItem(L"Toggle bitangents visualization", DEBUG_SP_BITANGENTS_VIZ);
            sub->addItem(L"Toggle wireframe visualization", DEBUG_SP_WIREFRAME_VIZ);
            sub->addItem(L"Toggle triangle normals visualization", DEBUG_SP_TN_VIZ);

            mnu->addItem(L"Keypress actions >",-1,true, true);
            sub = mnu->getSubMenu(7);
            sub->addItem(L"Rescue", DEBUG_RESCUE_KART);
            sub->addItem(L"Pause", DEBUG_PAUSE);

            mnu->addItem(L"Output >",-1,true, true);
            sub = mnu->getSubMenu(8);
            sub->addItem(L"Print kart positions", DEBUG_PRINT_START_POS);
            sub->addSeparator();
            sub->addItem(L"Save screenshot (Print Screen)", DEBUG_SAVE_SCREENSHOT);
            sub->addItem(L"Save replay (Ctrl + F11)", DEBUG_SAVE_REPLAY);
            sub->addItem(L"Save history (F11)", DEBUG_SAVE_HISTORY);
            sub->addItem(L"Dump RTT (Shift + F11)", DEBUG_DUMP_RTT);
            sub->addSeparator();
            sub->addItem(L"Profiler", DEBUG_PROFILER);
            if (UserConfigParams::m_profiler_enabled)
            {
                sub->addItem(L"Save profiler report", DEBUG_PROFILER_WRITE_REPORT);
            }

            mnu->addItem(L"Recording >",-1,true, true);
            sub = mnu->getSubMenu(9);

#ifdef ENABLE_RECORDER
            sub->addItem(L"Start recording (Ctrl + Print Screen)", DEBUG_START_RECORDING);
            sub->addItem(L"Stop recording (Ctrl + Print Screen)", DEBUG_STOP_RECORDING);
#else
            sub->addItem(L"Recording unavailable, STK was compiled without\n"
                          "recording support.  Please re-compile STK with\n"
                          "libopenglrecorder to enable recording.  If you got\n"
                          "SuperTuxKart from your distribution's repositories,\n"
                          "please use the official binaries, or contact your\n"
                          "distributions's package mantainer.");
#endif

            mnu->addItem(L"Consoles >",-1,true, true);
            sub = mnu->getSubMenu(10);
            sub->addItem(L"Scripting console (Shift + F7)", DEBUG_SCRIPT_CONSOLE);
            sub->addItem(L"Texture console (Shift + F8)", DEBUG_TEXTURE_CONSOLE);
            sub->addItem(L"Run cutscene(s) (Shift + F9)", DEBUG_RUN_CUTSCENE);

            mnu->addItem(L"Font >",-1,true, true);
            sub = mnu->getSubMenu(11);
            sub->addItem(L"Dump glyph pages of fonts", DEBUG_FONT_DUMP_GLYPH_PAGE);
            sub->addItem(L"Reload all fonts", DEBUG_FONT_RELOAD);

            mnu->addItem(L"Lighting >",-1,true, true);
            sub = mnu->getSubMenu(12);
            sub->addItem(L"Adjust values (Shift + ~)", DEBUG_VISUAL_VALUES);
            sub->addItem(L"Adjust lights (Ctrl + ~)", DEBUG_ADJUST_LIGHTS);

            mnu->addItem(L"FPS >",-1,true, true);
            sub = mnu->getSubMenu(13);
            sub->addItem(L"Do not limit FPS", DEBUG_THROTTLE_FPS);
            sub->addItem(L"Toggle FPS (F12)", DEBUG_FPS);

            mnu->addItem(L"Debug keys (~)", DEBUG_HELP);

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

    // continue event handling if menu is not opened
    return !g_debug_menu_visible;
}   // onEvent

// ----------------------------------------------------------------------------

void handleStaticAction(int key, int value, bool control_pressed, bool shift_pressed)
{
    World* world = World::getWorld();
    CameraFPS *cam = dynamic_cast<CameraFPS*>(Camera::getActiveCamera());

    if (value)
    {
        switch (key)
        {
            case IRR_KEY_OEM_3:
            {
                if (control_pressed)
                {
                    handleContextMenuAction(DEBUG_ADJUST_LIGHTS);
                }
                else if (shift_pressed)
                {
                    handleContextMenuAction(DEBUG_VISUAL_VALUES);
                }
                else
                {
                    handleContextMenuAction(DEBUG_HELP);
                }
                break;
            }
            // Function key actions
            case IRR_KEY_F1:
            {
                if (control_pressed)
                {
                    handleContextMenuAction(DEBUG_GUI_CAM_NORMAL);
                }
                else if (shift_pressed)
                {
                    handleContextMenuAction(DEBUG_ATTACHMENT_BOMB);
                }
                else
                {
                    handleContextMenuAction(DEBUG_POWERUP_ANVIL);
                }
                break;
            }
            case IRR_KEY_F2:
            {
                if (control_pressed)
                {
                    handleContextMenuAction(DEBUG_GUI_CAM_FREE);
                }
                else if (shift_pressed)
                {
                    handleContextMenuAction(DEBUG_ATTACHMENT_ANVIL);
                }
                else
                {
                    handleContextMenuAction(DEBUG_POWERUP_RUBBERBALL);
                }
                break;
            }
            case IRR_KEY_F3:
            {
                if (control_pressed)
                {
                    handleContextMenuAction(DEBUG_GUI_CAM_TOP);
                }
                else if (shift_pressed)
                {
                   handleContextMenuAction(DEBUG_ATTACHMENT_PARACHUTE);
                }
                else
                {
                    handleContextMenuAction(DEBUG_POWERUP_BOWLING);
                }
                break;
            }
            case IRR_KEY_F4:
            {
                if (control_pressed)
                {
                    handleContextMenuAction(DEBUG_GUI_CAM_WHEEL);
                }
                else if (shift_pressed)
                {
                    handleContextMenuAction(DEBUG_ATTACHMENT_SQUASH);
                }
                else
                {
                    handleContextMenuAction(DEBUG_POWERUP_BUBBLEGUM);
                }
                break;
            }
            case IRR_KEY_F5:
            {
                if (control_pressed)
                {
                    handleContextMenuAction(DEBUG_GUI_CAM_BEHIND_KART);
                }
                else if (shift_pressed)
                {
                    handleContextMenuAction(DEBUG_ATTACHMENT_PLUNGER);
                }
                else
                {
                    handleContextMenuAction(DEBUG_POWERUP_CAKE);
                }
                break;
            }
            case IRR_KEY_F6:
            {
                if (control_pressed)
                {
                    handleContextMenuAction(DEBUG_GUI_CAM_SIDE_OF_KART);
                }
                else if (shift_pressed)
                {
                    handleContextMenuAction(DEBUG_ATTACHMENT_EXPLOSION);
                }
                else
                {
                    handleContextMenuAction(DEBUG_POWERUP_PARACHUTE);
                }
                break;
            }
            case IRR_KEY_F7:
            {
                if (control_pressed)
                {
                    handleContextMenuAction(DEBUG_GUI_CAM_INV_SIDE_OF_KART);
                }
                else if (shift_pressed)
                {
                    handleContextMenuAction(DEBUG_SCRIPT_CONSOLE);
                }
                else
                {
                    handleContextMenuAction(DEBUG_POWERUP_PLUNGER);
                }
                break;
            }
            case IRR_KEY_F8:
            {
                if (control_pressed)
                {
                    handleContextMenuAction(DEBUG_GUI_CAM_FRONT_OF_KART);
                }
                else if (shift_pressed)
                {
                    handleContextMenuAction(DEBUG_TEXTURE_CONSOLE);
                }
                else
                {
                    handleContextMenuAction(DEBUG_POWERUP_SWATTER);
                }
                break;
            }
            case IRR_KEY_F9:
            {
                if (control_pressed)
                {
                    handleContextMenuAction(DEBUG_VIEW_KART_SLIDER);
                }
                else if (shift_pressed)
                {
                    handleContextMenuAction(DEBUG_RUN_CUTSCENE);
                }
                else
                {
                    handleContextMenuAction(DEBUG_POWERUP_SWITCH);
                }
                break;
            }
            case IRR_KEY_F10:
            {
                if (control_pressed)
                {
                    handleContextMenuAction(DEBUG_POWERUP_SLIDER);
                }
                else if (shift_pressed)
                {
                    handleContextMenuAction(DEBUG_GUI_TOGGLE);
                }
                else
                {
                    handleContextMenuAction(DEBUG_POWERUP_ZIPPER);
                }
                break;
            }
            case IRR_KEY_INSERT:
            {
                handleContextMenuAction(DEBUG_POWERUP_NITRO);
                break;
            }
            case IRR_KEY_DELETE:
            {
                handleContextMenuAction(DEBUG_POWERUP_NOTHING);
                handleContextMenuAction(DEBUG_ATTACHMENT_NOTHING);
                handleContextMenuAction(DEBUG_NITRO_CLEAR);
                break;
            }
            case IRR_KEY_HOME:
            {
                handleContextMenuAction(DEBUG_VIEW_KART_ONE);
                break;
            }
            case IRR_KEY_END:
            {
                handleContextMenuAction(DEBUG_VIEW_KART_LAST);
                break;
            }
            case IRR_KEY_NEXT:
            {
                handleContextMenuAction(DEBUG_VIEW_KART_NEXT);
                break;
            }
            case IRR_KEY_PRIOR:
            {
                handleContextMenuAction(DEBUG_VIEW_KART_PREVIOUS);
                break;
            }
            default : break;
        }
    }

    switch (key)
    {
        // Flying up and down
        case IRR_KEY_I:
        {
            AbstractKart* kart = world->getLocalPlayerKart(0);
            if (kart == NULL) break;

            kart->flyUp();
            break;
        }
        case IRR_KEY_K:
        {
            AbstractKart* kart = world->getLocalPlayerKart(0);
            if (kart == NULL) break;

            kart->flyDown();
            break;
        }
        // Moving the first person camera
        case IRR_KEY_W:
        {
            if (cam)
            {
                core::vector3df vel(cam->getLinearVelocity());
                vel.Z = value ? cam->getMaximumVelocity() : 0;
                cam->setLinearVelocity(vel);
            }
            break;
        }
        case IRR_KEY_S:
        {
            if (cam)
            {
                core::vector3df vel(cam->getLinearVelocity());
                vel.Z = value ? -cam->getMaximumVelocity() : 0;
                cam->setLinearVelocity(vel);
            }
            break;
        }
        case IRR_KEY_D:
        {
            if (cam)
            {
                core::vector3df vel(cam->getLinearVelocity());
                vel.X = value ? -cam->getMaximumVelocity() : 0;
                cam->setLinearVelocity(vel);
            }
            break;
        }
        case IRR_KEY_A:
        {
            if (cam)
            {
                core::vector3df vel(cam->getLinearVelocity());
                vel.X = value ? cam->getMaximumVelocity() : 0;
                cam->setLinearVelocity(vel);
            }
            break;
        }
        case IRR_KEY_E:
        {
            if (cam)
            {
                core::vector3df vel(cam->getLinearVelocity());
                vel.Y = value ? cam->getMaximumVelocity() : 0;
                cam->setLinearVelocity(vel);
            }
            break;
        }
        case IRR_KEY_Q:
        {
            if (cam)
            {
                core::vector3df vel(cam->getLinearVelocity());
                vel.Y = value ? -cam->getMaximumVelocity() : 0;
                cam->setLinearVelocity(vel);
            }
            break;
        }
        // Rotating the first person camera
        case IRR_KEY_R:
        {
            if (cam)
            {
                cam->setAngularVelocity(value ?
                    UserConfigParams::m_fpscam_max_angular_velocity : 0.0f);
            }
            break;
        }
        case IRR_KEY_F:
        {
            if (cam)
            {
                cam->setAngularVelocity(value ?
                    -UserConfigParams::m_fpscam_max_angular_velocity : 0);
            }
            break;
        }
        default : break;
    }
}

// ----------------------------------------------------------------------------
/** Returns if the debug menu is visible.
 */
bool isOpen()
{
    return g_debug_menu_visible;
}   // isOpen

// ----------------------------------------------------------------------------
/** Close the debug menu.
 */
void closeDebugMenu()
{
    g_debug_menu_visible = false;
}   // closeDebugMenu

}  // namespace Debug
