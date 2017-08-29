//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 SuperTuxKart-Team
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

#include "states_screens/race_gui_multitouch.hpp"

using namespace irr;

#include <algorithm>

#include "config/user_config.hpp"
#include "graphics/camera.hpp"
#include "graphics/2dutils.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material.hpp"
#include "guiengine/scalable_font.hpp"
#include "input/device_manager.hpp"
#include "input/multitouch_device.hpp"
#include "items/powerup.hpp"
#include "karts/abstract_kart.hpp"
#include "states_screens/race_gui_base.hpp"


//-----------------------------------------------------------------------------
/** The multitouch GUI constructor
 */
RaceGUIMultitouch::RaceGUIMultitouch(RaceGUIBase* race_gui)
{
    m_race_gui = race_gui;
    m_minimap_bottom = 0;
    m_gui_action = false;
    m_directionnal_wheel_tex = NULL;
    m_pause_tex = NULL;
    m_nitro_tex = NULL;
    m_nitro_empty_tex = NULL;
    m_wing_mirror_tex = NULL;
    m_thunderbird_reset_tex = NULL;
    m_drift_tex = NULL;
    m_bg_button_tex = NULL;
    m_bg_button_focus_tex = NULL;

    m_device = input_manager->getDeviceManager()->getMultitouchDevice();

    if (UserConfigParams::m_multitouch_scale > 1.5f)
    {
        UserConfigParams::m_multitouch_scale = 1.5f;
    }
    else if (UserConfigParams::m_multitouch_scale < 0.5f)
    {
        UserConfigParams::m_multitouch_scale = 0.5f;
    }

    initMultitouchSteering();
}   // RaceGUIMultitouch


//-----------------------------------------------------------------------------
/** The multitouch GUI destructor
 */
RaceGUIMultitouch::~RaceGUIMultitouch()
{
    closeMultitouchSteering();
}   // ~RaceGUIMultitouch


//-----------------------------------------------------------------------------
/** Sets the multitouch race gui to its initial state
 */
void RaceGUIMultitouch::reset()
{
    if (m_device != NULL)
    {
        m_device->reset();
    }
}   // reset


//-----------------------------------------------------------------------------
/** Clears all previously created buttons in the multitouch device
 */
void RaceGUIMultitouch::closeMultitouchSteering()
{
    if (m_device != NULL)
    {
        m_device->clearButtons();
    }
}   // closeMultitouchSteering


//-----------------------------------------------------------------------------
/** Makes some initializations and determines the look of multitouch steering
 *  interface
 */
void RaceGUIMultitouch::initMultitouchSteering()
{
    if (m_device == NULL)
        return;

    const float scale = UserConfigParams::m_multitouch_scale;

    const int w = irr_driver->getActualScreenSize().Width;
    const int h = irr_driver->getActualScreenSize().Height;
    const float btn_size = 0.125f * h * scale;
    const float btn2_size = 0.35f * h * scale;
    const float margin = 0.075f * h * scale;
    const float margin_top = 0.3f * h;
    const float col_size = (btn_size + margin);

    const float small_ratio = 0.75f;
    const float btn_small_size = small_ratio * btn_size;
    const float margin_small = small_ratio * margin;
    const float col_small_size = small_ratio * col_size;

    float first_column_x = w - 2 * col_size;
    float second_column_x = w - 1 * col_size;
    float steering_btn_margin = 0.6f * margin;
    float steering_btn_x = steering_btn_margin;
    float steering_btn_y = h - steering_btn_margin - btn2_size;
    
    if (UserConfigParams::m_multitouch_inverted)
    {
        first_column_x = margin + 1 * col_size;
        second_column_x = margin;
        steering_btn_x = w - btn2_size - steering_btn_margin;
    }

    m_minimap_bottom = (unsigned int)(h - 2 * col_size);

    m_device->addButton(BUTTON_STEERING,
                        int(steering_btn_x), int(steering_btn_y),
                        int(btn2_size), int(btn2_size));
    m_device->addButton(BUTTON_ESCAPE,
                        int(margin_top), int(margin_small),
                        int(btn_small_size), int(btn_small_size));
    m_device->addButton(BUTTON_RESCUE,
                        int(margin_top + col_small_size), int(margin_small),
                        int(btn_small_size), int(btn_small_size));
    m_device->addButton(BUTTON_NITRO,
                        int(second_column_x), int(h - 2 * col_size),
                        int(btn_size), int(btn_size));
    m_device->addButton(BUTTON_SKIDDING,
                        int(second_column_x), int(h - 1 * col_size),
                        int(btn_size), int(btn_size));
    m_device->addButton(BUTTON_FIRE,
                        int(first_column_x),  int(h - 2 * col_size),
                        int(btn_size), int(btn_size));
    m_device->addButton(BUTTON_LOOK_BACKWARDS,
                        int(first_column_x), int(h - 1 * col_size),
                        int(btn_size), int(btn_size));
                      
    m_directionnal_wheel_tex = irr_driver->getTexture(FileManager::GUI, 
                                              "android/directionnal_wheel.png");
    m_pause_tex = irr_driver->getTexture(FileManager::GUI, "android/pause.png");
    m_nitro_tex = irr_driver->getTexture(FileManager::GUI, "android/nitro.png");
    m_nitro_empty_tex = irr_driver->getTexture(FileManager::GUI, 
                                                     "android/nitro_empty.png");
    m_wing_mirror_tex = irr_driver->getTexture(FileManager::GUI, 
                                                     "android/wing_mirror.png");
    m_thunderbird_reset_tex = irr_driver->getTexture(FileManager::GUI, 
                                               "android/thunderbird_reset.png");
    m_drift_tex = irr_driver->getTexture(FileManager::GUI, "android/drift.png");
    m_bg_button_tex = irr_driver->getTexture(FileManager::GUI, 
                                                  "android/blur_bg_button.png");
    m_bg_button_focus_tex = irr_driver->getTexture(FileManager::GUI, 
                                            "android/blur_bg_button_focus.png");
    m_gui_action_tex = irr_driver->getTexture(FileManager::GUI,"challenge.png");

} // initMultitouchSteering

//-----------------------------------------------------------------------------
/** Draws the buttons for multitouch steering.
 *  \param kart The kart for which to show the data.
 *  \param viewport The viewport to use.
 *  \param scaling Which scaling to apply to the buttons.
 */
void RaceGUIMultitouch::drawMultitouchSteering(const AbstractKart* kart,
                                     const core::recti &viewport,
                                     const core::vector2df &scaling)
{
#ifndef SERVER_ONLY
    if (m_device == NULL)
        return;

    for (unsigned int i = 0; i < m_device->getButtonsCount(); i++)
    {
        MultitouchButton* button = m_device->getButton(i);

        core::rect<s32> btn_pos(button->x, button->y, button->x + button->width,
                                button->y + button->height);
                                
        core::rect<s32> btn_pos_bg((int)(button->x - button->width * 0.2f),
                                   (int)(button->y - button->height * 0.2f),
                                   (int)(button->x + button->width * 1.2f),
                                   (int)(button->y + button->height * 1.2f));
                                   
        const core::position2d<s32> pos_zero = core::position2d<s32>(0,0);

        if (button->type == MultitouchButtonType::BUTTON_STEERING)
        {
            video::ITexture* btn_texture = m_directionnal_wheel_tex;
            core::rect<s32> coords(pos_zero, btn_texture->getSize());
            draw2DImage(btn_texture, btn_pos, coords, NULL, NULL, true);

            // float x = (float)(button->x) + (float)(button->width) / 2.0f *
            //                                          (button->axis_x + 1.0f);
            // float y = (float)(button->y) + (float)(button->height) / 2.0f *
            //                                          (button->axis_y + 1.0f);
            // float w = (float)(button->width) / 20.0f;
            // float h = (float)(button->height) / 20.0f;

            // core::rect<s32> pos2(int(round(x - w)), int(round(y - h)),
            //                      int(round(x + w)), int(round(y + h)));

            // draw2DImage(btn_texture, pos2, coords, NULL, NULL, true);
        }
        else
        {
            bool can_be_pressed = true;
            video::ITexture* btn_texture = NULL;

            switch (button->type)
            {
            case MultitouchButtonType::BUTTON_ESCAPE:
                btn_texture = m_pause_tex;
                break;
            case MultitouchButtonType::BUTTON_FIRE:
            {
                const Powerup* powerup = kart->getPowerup();
                if (m_gui_action == true)
                {
                    btn_texture = m_gui_action_tex;
                }
                else if (powerup->getType() != PowerupManager::POWERUP_NOTHING
                         && !kart->hasFinishedRace())
                {
                    btn_texture = powerup->getIcon()->getTexture();
                }
                else
                {
                    can_be_pressed = false;
                    btn_texture = NULL;
                }
                break;
            }
            case MultitouchButtonType::BUTTON_NITRO:
            {
                if (kart->getEnergy() > 0)
                {
                    btn_texture = m_nitro_tex;
                }
                else
                {
                    can_be_pressed = false;
                    btn_texture = m_nitro_empty_tex;
                }
                break;
            }
            case MultitouchButtonType::BUTTON_LOOK_BACKWARDS:
                btn_texture = m_wing_mirror_tex;
                break;
            case MultitouchButtonType::BUTTON_RESCUE:
                btn_texture = m_thunderbird_reset_tex;
                break;
            case MultitouchButtonType::BUTTON_SKIDDING:
                btn_texture = m_drift_tex;
                break;
            default:
                break;
            }

            if (btn_texture)
            {
                video::ITexture* btn_bg = (can_be_pressed && button->pressed) ?
                                                        m_bg_button_focus_tex : 
                                                        m_bg_button_tex;
                core::rect<s32> coords_bg(pos_zero, btn_bg->getSize());
                draw2DImage(btn_bg, btn_pos_bg, coords_bg, NULL, NULL, true);                

                core::rect<s32> coords(pos_zero, btn_texture->getSize());
                draw2DImage(btn_texture, btn_pos, coords, NULL, NULL, true);
            }

            if (button->type == MultitouchButtonType::BUTTON_NITRO &&
                m_race_gui != NULL)
            {
                float scale = UserConfigParams::m_multitouch_scale *
                    (float)(irr_driver->getActualScreenSize().Height) / 720.0f;

                m_race_gui->drawEnergyMeter(int(button->x + button->width * 1.15f),
                                            int(button->y + button->height * 1.35f),
                                            kart, viewport,
                                            core::vector2df(scale, scale));
            }
            else if (button->type == MultitouchButtonType::BUTTON_FIRE &&
                     kart->getPowerup()->getNum() > 1 && 
                     m_gui_action == false)
            {
                gui::ScalableFont* font = GUIEngine::getHighresDigitFont();
                core::rect<s32> pos((int)(button->x),
                                    (int)(button->y),
                                    (int)(button->x + button->width/2),
                                    (int)(button->y + button->height/2));
                font->setScale(UserConfigParams::m_multitouch_scale);
                font->draw(core::stringw(L"+"), pos,
                           video::SColor(255, 255, 255, 255));
                font->setScale(1.0f);
            }
        }
    }
#endif
} // drawMultitouchSteering
