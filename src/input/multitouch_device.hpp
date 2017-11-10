//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 SuperTuxKart-Team
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

#ifndef HEADER_MULTITOUCH_DEVICE_HPP
#define HEADER_MULTITOUCH_DEVICE_HPP

#include <array>
#include <vector>

#include "input/input_device.hpp"
#include "IEventReceiver.h"

#ifdef ANDROID
#include "../../../lib/irrlicht/source/Irrlicht/CIrrDeviceAndroid.h"
#endif

#define NUMBER_OF_MULTI_TOUCHES 10

enum MultitouchButtonType
{
    BUTTON_STEERING,
    BUTTON_FIRE,
    BUTTON_NITRO,
    BUTTON_SKIDDING,
    BUTTON_LOOK_BACKWARDS,
    BUTTON_RESCUE,
    BUTTON_ESCAPE,
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_LEFT,
    BUTTON_RIGHT
};

struct MultitouchEvent
{
    int id;
    bool touched;
    int x;
    int y;
};

struct MultitouchButton
{
    MultitouchButtonType type;
    PlayerAction action;
    bool pressed;
    unsigned int event_id;
    int x;
    int y;
    int width;
    int height;
    float axis_x;
    float axis_y;
};

class MultitouchDevice : public InputDevice
{
private:
    /** The list of pointers to all created buttons */
    std::vector<MultitouchButton*> m_buttons;

    /** The parameter that is used for steering button and determines dead area
     *  in a center of button */
    float m_deadzone_center;

    /** The parameter that is used for steering button and determines dead area
     *  at the edge of button */
    float m_deadzone_edge;

    /** True if accelerometer is in use */
    bool m_accelerometer_active;

#ifdef ANDROID
    /** Pointer to the Android irrlicht device */
    CIrrDeviceAndroid* m_android_device;
#endif

    float getSteeringFactor(float value);
    void updateButtonAxes(MultitouchButton* button, float x, float y);

public:
    /** The array that contains data for all multitouch input events */
    std::array<MultitouchEvent, NUMBER_OF_MULTI_TOUCHES> m_events;

    MultitouchDevice();
    virtual ~MultitouchDevice();

    /** Unused function */
    bool processAndMapInput(Input::InputType type,  const int id,
                            InputManager::InputDriverMode mode,
                            PlayerAction *action, int* value = NULL)
                            {return true;}

    unsigned int getActiveTouchesCount();

    void addButton(MultitouchButtonType type, int x, int y, int width,
                   int height);
    void clearButtons();
    void reset();

    /** Returns the number of created buttons */
    unsigned int getButtonsCount() { return (unsigned int)m_buttons.size();}

    /** Returns pointer to the selected button */
    MultitouchButton* getButton(unsigned int i) {return m_buttons.at(i);}

    /** True if accelerometer is in use */
    bool isAccelerometerActive() {return m_accelerometer_active;}

    void updateDeviceState(unsigned int event_id);
    void handleControls(MultitouchButton* button);

    void updateConfigParams();

};   // MultitouchDevice

#endif
