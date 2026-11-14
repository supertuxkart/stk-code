//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2020 SuperTuxKart-Team
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

#include "input/sdl_controller.hpp"
#include "guiengine/message_queue.hpp"
#include "input/device_config.hpp"
#include "input/device_manager.hpp"
#include "input/gamepad_config.hpp"
#include "input/gamepad_device.hpp"
#include "input/input_manager.hpp"
#include "utils/log.hpp"
#include "utils/time.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <SDL_timer.h>
#include <SDL_version.h>
#include <algorithm>
#include <stdexcept>
#include <string>

namespace
{
SDL_HapticEffect steeringEffect(Sint16 level)
{
    SDL_HapticEffect effect = {};
    effect.type = SDL_HAPTIC_CONSTANT;
    effect.constant.direction.type = SDL_HAPTIC_CARTESIAN;
    // SDL directions tell where the force comes from, so a force from the
    // left makes a positive level push the wheel to the right.
    effect.constant.direction.dir[0] = -1;
    // The effect is stopped explicitly once the game stops feeding it, see
    // SDLController::updateForceFeedback().
    effect.constant.length = SDL_HAPTIC_INFINITY;
    effect.constant.level = level;
    return effect;
}   // steeringEffect
}

// ----------------------------------------------------------------------------
SDLController::SDLController(int device_id)
             : m_gamepad(NULL)
{
    m_last_power_level_time = StkTime::getMonoTimeMs();
    m_irr_event = {};
    m_irr_event.EventType = irr::EET_JOYSTICK_INPUT_EVENT;
    memset(m_prev_axes, 0,
        irr::SEvent::SJoystickEvent::NUMBER_OF_AXES * sizeof(int16_t));
    m_game_controller = NULL;
    m_joystick = NULL;
    m_haptic = NULL;
    m_native_rumble = false;
    m_steering_effect = -1;
    m_steering_effect_running = false;
    m_steering_spring = 0.0f;
    m_steering_damper = 0.0f;
    m_steering_offset = 0.0f;
    m_steering_force_time = 0;
    m_prev_wheel = 0.0f;
    m_prev_ffb_update = 0;
    m_uploaded_level = 0;
    m_rumble_strength = 0.0f;
    m_rumble_end_time = 0;
    m_id = -1;

    if (SDL_IsGameController(device_id))
    {
        m_game_controller = SDL_GameControllerOpen(device_id);
        if (!m_game_controller)
            throw std::runtime_error(SDL_GetError());
        m_joystick = SDL_GameControllerGetJoystick(m_game_controller);
        if (!m_joystick)
        {
            SDL_GameControllerClose(m_game_controller);
            throw std::runtime_error(SDL_GetError());
        }
    }
    else
    {
        m_joystick = SDL_JoystickOpen(device_id);
        if (!m_joystick)
            throw std::runtime_error(SDL_GetError());
    }

    m_id = SDL_JoystickInstanceID(m_joystick);
    if (m_id < 0)
    {
        if (m_game_controller)
            SDL_GameControllerClose(m_game_controller);
        else
            SDL_JoystickClose(m_joystick);
        throw std::runtime_error(SDL_GetError());
    }

    m_irr_event.JoystickEvent.Joystick = m_id;
    const char* name_cstr = SDL_JoystickName(m_joystick);
    if (name_cstr == NULL)
    {
        if (m_game_controller)
            SDL_GameControllerClose(m_game_controller);
        else
            SDL_JoystickClose(m_joystick);
        throw std::runtime_error("missing name for joystick");
    }
    std::string name = name_cstr;
#ifdef WIN32
    // SDL added #number to xinput controller which is its user id, we remove
    // it manually to allow hotplugging to get the same config each time
    // user id ranges from 0-3
    // From GetXInputName(const Uint8 userid, BYTE SubType) in
    // SDL_xinputjoystick.c
    if ((name.size() > 7 &&
        name.compare(0, 7, "XInput ") == 0 && name[name.size() - 2] == '#') ||
        (name.size() > 16 &&
        name.compare(0, 16, "X360 Controller ") == 0 && name[name.size() - 2] == '#'))
        name.erase(name.length() - 3);
#endif

    m_buttons = SDL_JoystickNumButtons(m_joystick);
    if (m_buttons < 0)
    {
        if (m_game_controller)
            SDL_GameControllerClose(m_game_controller);
        else
            SDL_JoystickClose(m_joystick);
        throw std::runtime_error(SDL_GetError());
    }

    m_axes = SDL_JoystickNumAxes(m_joystick);
    if (m_axes < 0)
    {
        if (m_game_controller)
            SDL_GameControllerClose(m_game_controller);
        else
            SDL_JoystickClose(m_joystick);
        throw std::runtime_error(SDL_GetError());
    }

    m_hats = SDL_JoystickNumHats(m_joystick);
    if (m_hats < 0)
    {
        if (m_game_controller)
            SDL_GameControllerClose(m_game_controller);
        else
            SDL_JoystickClose(m_joystick);
        throw std::runtime_error(SDL_GetError());
    }

    Log::info("SDLController",
        "%s plugged in: buttons: %d, axes: %d, hats: %d.", name.c_str(),
        m_buttons, m_axes, m_hats);
    if (m_game_controller && SDL_GameControllerName(m_game_controller))
    {
        Log::info("SDLController", "%s uses game controller mapping %s.",
            name.c_str(), SDL_GameControllerName(m_game_controller));
    }

    if (m_buttons > irr::SEvent::SJoystickEvent::NUMBER_OF_BUTTONS)
        m_buttons = irr::SEvent::SJoystickEvent::NUMBER_OF_BUTTONS;
    if (m_axes > irr::SEvent::SJoystickEvent::NUMBER_OF_AXES)
        m_axes = irr::SEvent::SJoystickEvent::NUMBER_OF_AXES;
    // We store hats event with 4 buttons
    int max_buttons_with_hats =
        irr::SEvent::SJoystickEvent::NUMBER_OF_BUTTONS - (m_hats * 4);
    if (m_buttons > max_buttons_with_hats)
        m_hats = 0;
    else
        m_buttons += m_hats * 4;
    // Save previous axes values for input sensing
    for (int i = 0; i < m_axes; i++)
        m_prev_axes[i] = SDL_JoystickGetAxis(m_joystick, i);

    DeviceManager* dm = input_manager->getDeviceManager();
    GamepadConfig* cfg = NULL;
    bool created = false;

    if (dm->getConfigForGamepad(m_id, name, &cfg) == true)
    {
        Log::info("SDLController", "creating new configuration for %s.",
            name.c_str());
        created = true;
    }

    std::string mapping_string;
    if (m_game_controller)
    {
        char* mapping = SDL_GameControllerMapping(m_game_controller);
        if (mapping)
        {
            mapping_string = mapping;
            SDL_free(mapping);
        }
    }
    cfg->initSDLController(mapping_string, m_buttons, m_axes, m_hats);
    if (created)
        cfg->initSDLMapping();
    cfg->setPlugged();

    for (int i = 0; i < dm->getGamePadAmount(); i++)
    {
        GamePadDevice* d = dm->getGamePad(i);
        if (d->getName() == name && !d->isConnected())
        {
            m_gamepad = d;
            d->setConnected(true);
            d->setIrrIndex(m_id);
            d->setConfiguration(cfg);
            if (created)
                dm->save();
            goto finish;
        }
    }

    m_gamepad = new GamePadDevice(m_id, name, m_axes, m_buttons, cfg);
    dm->addGamepad(m_gamepad);
    if (created)
        dm->save();

finish:
    m_haptic = SDL_HapticOpenFromJoystick(m_joystick);
    if (!m_haptic)
        return;

    m_native_rumble = SDL_HapticRumbleSupported(m_haptic) == SDL_TRUE &&
        SDL_HapticRumbleInit(m_haptic) == 0;
    updateAutoCenter(getGamePadDevice()->getAutoCenterStrength());

    if ((SDL_HapticQuery(m_haptic) & SDL_HAPTIC_CONSTANT) == 0)
        return;

    SDL_HapticEffect effect = steeringEffect(0);
    m_steering_effect = SDL_HapticNewEffect(m_haptic, &effect);
    if (m_steering_effect < 0)
    {
        Log::warn("SDLController", "%s: cannot create constant force: %s",
            name.c_str(), SDL_GetError());
        return;
    }
    Log::info("SDLController", "%s supports steering force feedback.",
        name.c_str());
}   // SDLController

// ----------------------------------------------------------------------------
SDLController::~SDLController()
{
    Log::info("SDLController", "%s unplugged.", SDL_JoystickName(m_joystick));
    // Closing the joystick can free the haptic device on some backends.
    if (m_haptic)
    {
        stopSteeringEffect();
        if (m_steering_effect >= 0)
            SDL_HapticDestroyEffect(m_haptic, m_steering_effect);
        SDL_HapticClose(m_haptic);
    }
    if (m_game_controller)
        SDL_GameControllerClose(m_game_controller);
    else
        SDL_JoystickClose(m_joystick);
    m_gamepad->getConfiguration()->unPlugged();
    m_gamepad->setIrrIndex(-1);
    m_gamepad->setConnected(false);
}   // ~SDLController

// ----------------------------------------------------------------------------
/** SDL only sends event when axis moves, so we need to send previously saved
 *  event for correct input sensing. */
void SDLController::handleAxisInputSense(const SDL_Event& event)
{
    int axis_idx = event.jaxis.axis;
    if (axis_idx > m_axes)
        return;
    if (event.jaxis.value == m_prev_axes[axis_idx])
        return;
    input_manager->dispatchInput(Input::IT_STICKMOTION,
        m_irr_event.JoystickEvent.Joystick, axis_idx, Input::AD_NEUTRAL,
        m_prev_axes[axis_idx]);
}   // handleAxisInputSense

// ----------------------------------------------------------------------------
void SDLController::checkPowerLevel()
{
#if SDL_VERSION_ATLEAST(2, 0, 4)
    const uint64_t time_now = StkTime::getMonoTimeMs();
    if (time_now > m_last_power_level_time + 60000)
    {
        m_last_power_level_time = time_now;
        SDL_JoystickPowerLevel pl = SDL_JoystickCurrentPowerLevel(m_joystick);
        if (pl == SDL_JOYSTICK_POWER_EMPTY || pl == SDL_JOYSTICK_POWER_LOW)
        {
            core::stringw msg =
                _("%s has low battery level.", SDL_JoystickName(m_joystick));
            MessageQueue::add(MessageQueue::MT_ERROR, msg);
            // Check 5 min later
            m_last_power_level_time += 240000;
        }
    }
#endif
}   // checkPowerLevel

void SDLController::doRumble(float strength_low, float strength_high, uint32_t duration_ms)
{
    float strength = std::min(1.0f, (strength_low + strength_high) / 2);
    if (m_native_rumble)
    {
        SDL_HapticRumblePlay(m_haptic, strength, duration_ms);
        return;
    }
    if (m_steering_effect >= 0)
    {
        m_rumble_strength = strength;
        m_rumble_end_time = StkTime::getMonoTimeMs() + duration_ms;
        return;
    }
#if SDL_VERSION_ATLEAST(2,0,9)
    if (!m_game_controller)
        return;
    uint16_t scaled_low = std::min(1.0f, strength_low) * 65535;
    uint16_t scaled_high = std::min(1.0f, strength_high) * 65535;
    SDL_GameControllerRumble(m_game_controller, scaled_low, scaled_high, duration_ms);
#endif
}

// ----------------------------------------------------------------------------
void SDLController::updateAutoCenter(int state)
{
    m_auto_center = state;
    SDL_HapticSetAutocenter(m_haptic, m_auto_center);
}

// ----------------------------------------------------------------------------
/** Sets the steering force as F = offset - spring * wheel - damper * speed,
 *  with the wheel position in [-1, 1] and its speed in full locks per second,
 *  where positive pushes the wheel to the right. It has to be called
 *  continuously, the force fades out shortly after the game stops calling it
 *  (pause, race end, menus).
 */
void SDLController::setSteeringForce(float spring, float damper, float offset)
{
    m_steering_spring = spring;
    m_steering_damper = damper;
    m_steering_offset = offset;
    m_steering_force_time = StkTime::getMonoTimeMs();
}   // setSteeringForce

// ----------------------------------------------------------------------------
/** Finds the axis bound to steering. The sign converts the raw axis value to
 *  game steering, where positive is right.
 */
bool SDLController::getSteeringAxis(int* axis, float* sign) const
{
    const DeviceConfig* cfg = m_gamepad->getConfiguration();
    const Binding& right = cfg->getBinding(PA_STEER_RIGHT);
    if (right.getType() == Input::IT_STICKMOTION && right.getId() < m_axes)
    {
        *axis = right.getId();
        *sign = right.getDirection() == Input::AD_POSITIVE ? 1.0f : -1.0f;
        return true;
    }
    const Binding& left = cfg->getBinding(PA_STEER_LEFT);
    if (left.getType() == Input::IT_STICKMOTION && left.getId() < m_axes)
    {
        *axis = left.getId();
        *sign = left.getDirection() == Input::AD_POSITIVE ? -1.0f : 1.0f;
        return true;
    }
    return false;
}   // getSteeringAxis

// ----------------------------------------------------------------------------
/** Sends the current steering force and emulated rumble to the device. The
 *  wheel position is read here instead of from the game input, so it is free
 *  of the gamepad deadzone and the wheel speed uses the real frame time
 *  instead of the physics step.
 */
void SDLController::updateForceFeedback()
{
    if (m_steering_effect < 0)
        return;

    const uint64_t now = StkTime::getMonoTimeMs();
    const bool steering = now < m_steering_force_time + 100;
    const bool rumbling = now < m_rumble_end_time;
    if (!steering && !rumbling)
    {
        stopSteeringEffect();
        return;
    }

    // On Linux every upload becomes a USB report (ff-memless replays the
    // effect), uncapped frame rates would overflow the usbhid output queue.
    const uint64_t counter = SDL_GetPerformanceCounter();
    const uint64_t frequency = SDL_GetPerformanceFrequency();
    if (m_prev_ffb_update != 0 &&
        (counter - m_prev_ffb_update) * 400 < frequency)
        return;
    const float dt = m_prev_ffb_update == 0 ? 0.0f :
        float(counter - m_prev_ffb_update) / frequency;
    m_prev_ffb_update = counter;

    int axis = 0;
    float axis_sign = 1.0f;
    const bool has_axis = getSteeringAxis(&axis, &axis_sign);
    float wheel = 0.0f;
    float wheel_speed = 0.0f;
    if (has_axis)
    {
        wheel = axis_sign * SDL_JoystickGetAxis(m_joystick, axis) / 32767.0f;
        if (dt > 0.0f)
            wheel_speed = (wheel - m_prev_wheel) / dt;
        wheel_speed = std::max(-5.0f, std::min(5.0f, wheel_speed));
        m_prev_wheel = wheel;
    }

    float force = 0.0f;
    if (steering)
    {
        force = m_steering_offset - m_steering_spring * wheel -
            m_steering_damper * wheel_speed;
    }
    if (rumbling)
    {
        // Alternate the push direction quickly, the wheel motor can't do a
        // real vibration. 50 ms per half wave stays readable at 60 fps.
        force += ((now / 50) % 2 == 0 ? 1.0f : -1.0f) * m_rumble_strength * 0.6f;
    }

    // The effect pushes towards the positive end of the axis, which is left
    // in the game if the player bound the axis the other way round.
    force *= axis_sign;
    GamepadConfig* cfg =
        static_cast<GamepadConfig*>(m_gamepad->getConfiguration());
    force *= cfg->getForceFeedbackStrength() / 100.0f;
    if (cfg->isForceFeedbackInverted())
        force = -force;
    force = std::max(-1.0f, std::min(1.0f, force));

    const Sint16 level = (Sint16)(force * 32767);
    if (m_steering_effect_running && level == m_uploaded_level)
        return;
    SDL_HapticEffect effect = steeringEffect(level);
    SDL_HapticUpdateEffect(m_haptic, m_steering_effect, &effect);
    m_uploaded_level = level;

    if (m_steering_effect_running)
        return;
    // The game provides its own centering spring while driving, the device
    // autocenter on top of it would make the wheel feel stiff and dead.
    updateAutoCenter(0);
    SDL_HapticRunEffect(m_haptic, m_steering_effect, 1);
    m_steering_effect_running = true;
}   // updateForceFeedback

// ----------------------------------------------------------------------------
void SDLController::stopSteeringEffect()
{
    if (!m_steering_effect_running)
        return;
    SDL_HapticStopEffect(m_haptic, m_steering_effect);
    m_steering_effect_running = false;
    m_prev_ffb_update = 0;
    updateAutoCenter(m_gamepad->getAutoCenterStrength());
}   // stopSteeringEffect

// ----------------------------------------------------------------------------
#ifdef ANDROID
void SDLController::handleDirectScanCode(const SDL_Event& event)
{
    // Android STK has custom changes in SDL2 to allow gamepad with unknown
    // button to use scan code directly
    input_manager->dispatchInput(Input::IT_STICKBUTTON,
        m_irr_event.JoystickEvent.Joystick, event.jbutton.button,
        Input::AD_POSITIVE,
        event.jbutton.state == SDL_PRESSED ? Input::MAX_VALUE : 0);
}   // handleDirectScanCode
#endif

#endif
