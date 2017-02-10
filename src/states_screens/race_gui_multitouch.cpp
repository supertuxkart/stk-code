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
    const float top_margin = 0.3f * h;
    const float col_size = (btn_size + margin);
    const float small_ratio = 0.75f;

    m_minimap_bottom = (unsigned int)(h - 2 * col_size);

    m_device->addButton(BUTTON_STEERING,
                      int(0.5f * margin), int(h - 0.5f * margin - btn2_size),
                      int(btn2_size), int(btn2_size));
    m_device->addButton(BUTTON_ESCAPE,
                      int(top_margin), int(small_ratio * margin),
                      int(small_ratio * btn_size), int(small_ratio * btn_size));
    m_device->addButton(BUTTON_RESCUE,
                      int(top_margin + small_ratio * col_size),
                      int(small_ratio * margin),
                      int(small_ratio * btn_size), int(small_ratio * btn_size));
    m_device->addButton(BUTTON_NITRO,
                      int(w - 1 * col_size), int(h - 2 * col_size),
                      int(btn_size), int(btn_size));
    m_device->addButton(BUTTON_SKIDDING,
                      int(w - 1 * col_size), int(h - 1 * col_size),
                      int(btn_size), int(btn_size));
    m_device->addButton(BUTTON_FIRE,
                      int(w - 2 * col_size),  int(h - 2 * col_size),
                      int(btn_size), int(btn_size));
    m_device->addButton(BUTTON_LOOK_BACKWARDS,
                      int(w - 2 * col_size), int(h - 1 * col_size),
                      int(btn_size), int(btn_size));

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
            video::ITexture* btn_texture = irr_driver->getTexture(
                            FileManager::GUI, "android/directionnal_wheel.png");
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

            if (button->type == MultitouchButtonType::BUTTON_FIRE)
            {
                const Powerup* powerup = kart->getPowerup();
                if (powerup->getType() != PowerupManager::POWERUP_NOTHING &&
                    !kart->hasFinishedRace())
                {
                    btn_texture = powerup->getIcon()->getTexture();
                }
                else
                {
                    can_be_pressed = false;
                    btn_texture = NULL;
                }
            }
            else
            {
                std::string name = "gui_lock.png";

                switch (button->type)
                {
                case MultitouchButtonType::BUTTON_ESCAPE:
                    name = "android/pause.png";
                    break;
                case MultitouchButtonType::BUTTON_NITRO:
                    if (kart->getEnergy() > 0)
                    {
                        name = "android/nitro.png";
                    }
                    else
                    {
                        can_be_pressed = false;
                        name = "android/nitro_empty.png";
                    }
                    break;
                case MultitouchButtonType::BUTTON_LOOK_BACKWARDS:
                    name = "android/wing_mirror.png";
                    break;
                case MultitouchButtonType::BUTTON_RESCUE:
                    name = "android/thunderbird_reset.png";
                    break;
                case MultitouchButtonType::BUTTON_SKIDDING:
                    name = "android/drift.png";
                    break;
                default:
                    break;
                }

                btn_texture = irr_driver->getTexture(FileManager::GUI, name);
            }

            if (btn_texture)
            {
                std::string bg_name = (can_be_pressed && button->pressed) ?
                                       "android/blur_bg_button_focus.png" :
                                       "android/blur_bg_button.png";

                video::ITexture* btn_bg;
                btn_bg = irr_driver->getTexture(FileManager::GUI, bg_name);
                core::rect<s32> coords_bg(pos_zero, btn_bg->getSize());
                draw2DImage(btn_bg, btn_pos_bg, coords_bg, NULL, NULL, true);                

                core::rect<s32> coords(pos_zero, btn_texture->getSize());
                draw2DImage(btn_texture, btn_pos, coords, NULL, NULL, true);
            }

            if (button->type == MultitouchButtonType::BUTTON_NITRO)
            {
                float scale = UserConfigParams::m_multitouch_scale *
                    (float)(irr_driver->getActualScreenSize().Height) / 720.0f;

                if (m_race_gui != NULL)
                {
                    m_race_gui->drawEnergyMeter(button->x + button->width * 1.15f,
                                                button->y + button->height * 1.35f,
                                                kart, viewport,
                                                core::vector2df(scale, scale));
                }
            }
            else if (button->type == MultitouchButtonType::BUTTON_FIRE)
            {
                if (kart->getPowerup()->getNum() > 1)
                {
                    gui::ScalableFont* font = GUIEngine::getHighresDigitFont();
                    core::rect<s32> btn_pos((int)(button->x),
                                            (int)(button->y),
                                            (int)(button->x + button->width/2),
                                            (int)(button->y + button->height/2));
                    font->setScale(UserConfigParams::m_multitouch_scale);
                    font->draw(core::stringw(L"+"), btn_pos,
                               video::SColor(255, 255, 255, 255));
                    font->setScale(1.0f);
                }
            }
        }
    }
#endif
} // drawMultitouchSteering
