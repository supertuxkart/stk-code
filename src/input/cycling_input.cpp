//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2026 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.

#include "input/cycling_input.hpp"

#include "config/user_config.hpp"
#include "guiengine/modaldialog.hpp"
#include "guiengine/screen_keyboard.hpp"
#include "input/device_manager.hpp"
#include "input/input.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/controller/controller.hpp"
#include "states_screens/state_manager.hpp"

#ifdef ANDROID
#include "SDL_system.h"
#include <jni.h>
#endif

#include <algorithm>
#include <atomic>
#include <cmath>

namespace
{
    std::atomic<float> g_sensor_rpm(0.0f);
    std::atomic<float> g_cadence_rpm(0.0f);
    std::atomic<int> g_source(CyclingInput::TS_CADENCE);
    std::atomic<bool> g_connected(false);
    bool g_was_controlling = false;

    Controller* getController(DeviceManager* device_manager)
    {
        if (device_manager == NULL ||
            StateManager::get()->getGameState() != GUIEngine::GAME ||
            GUIEngine::ModalDialog::isADialogActive() ||
            GUIEngine::ScreenKeyboard::isActive())
        {
            return NULL;
        }

        StateManager::ActivePlayer* player = device_manager->getSinglePlayer();
        if (player == NULL &&
            StateManager::get()->getActivePlayers().size() > 0)
        {
            player = StateManager::get()->getActivePlayer(0);
        }
        if (player == NULL || player->getKart() == NULL)
            return NULL;
        return player->getKart()->getController();
    }
}

// ----------------------------------------------------------------------------
void CyclingInput::setTelemetry(float sensor_rpm, TelemetrySource source,
                                bool connected)
{
    g_sensor_rpm.store(std::max(0.0f, sensor_rpm));
    g_source.store(source);
    g_connected.store(connected);
}

// ----------------------------------------------------------------------------
float CyclingInput::getCadence()
{
    return g_cadence_rpm.load();
}

// ----------------------------------------------------------------------------
void CyclingInput::update(DeviceManager* device_manager)
{
    const bool enabled = UserConfigParams::m_cycling_enabled;
    const bool connected = g_connected.load();
    float cadence = 0.0f;
    if (enabled && connected)
    {
        cadence = g_sensor_rpm.load();
        if (g_source.load() == TS_WHEEL)
        {
            const float ratio = std::max(0.1f,
                (float)UserConfigParams::m_cycling_wheel_ratio);
            cadence /= ratio;
        }
    }
    g_cadence_rpm.store(cadence);

    // Stay completely out of the regular input path while cycling is off.
    // On the first frame after it is disabled, release only the actions that
    // cycling mode previously owned so no throttle or nitro state is left set.
    if (!enabled && !g_was_controlling)
        return;

    Controller* controller = getController(device_manager);
    if (controller == NULL)
        return;

    if (!enabled)
    {
        controller->action(PA_ACCEL, 0);
        controller->action(PA_NITRO, 0);
        g_was_controlling = false;
        return;
    }

    g_was_controlling = true;
    if (!connected)
    {
        controller->action(PA_ACCEL, 0);
        controller->action(PA_NITRO, 0);
        return;
    }

    const int start_rpm = std::max(1,
        (int)UserConfigParams::m_cycling_start_rpm);
    const int full_speed_rpm = std::max(start_rpm + 1,
        (int)UserConfigParams::m_cycling_full_speed_rpm);

    if (cadence < start_rpm)
    {
        controller->action(PA_ACCEL, 0);
        controller->action(PA_NITRO, 0);
        return;
    }

    // Use an ease-out curve so a modest cadence produces a useful kart speed.
    // Full acceleration is available while the kart catches up; near the
    // cadence-derived target, acceleration is tapered to avoid PWM judder.
    float progress = (cadence - start_rpm) /
        (float)(full_speed_rpm - start_rpm);
    progress = std::max(0.0f, std::min(1.0f, progress));
    progress = std::sqrt(progress);

    float minimum_fraction =
        (float)UserConfigParams::m_cycling_min_speed_percent / 100.0f;
    minimum_fraction = std::max(0.1f, std::min(1.0f, minimum_fraction));
    const float speed_fraction = minimum_fraction +
        (1.0f - minimum_fraction) * progress;

    AbstractKart* kart = controller->getKart();
    const float maximum_speed = std::max(1.0f, kart->getCurrentMaxSpeed());
    const float target_speed = maximum_speed * speed_fraction;
    const float speed_error = target_speed - std::fabs(kart->getSpeed());
    const float control_range = std::max(1.0f, maximum_speed * 0.12f);
    float acceleration = 0.35f + speed_error / control_range;
    acceleration = std::max(0.0f, std::min(1.0f, acceleration));

    controller->action(PA_ACCEL,
        (int)(acceleration * (float)Input::MAX_VALUE));

    const bool use_nitro = UserConfigParams::m_cycling_auto_nitro &&
        cadence >= (int)UserConfigParams::m_cycling_sprint_rpm;
    controller->action(PA_NITRO, use_nitro ? Input::MAX_VALUE : 0);
}

// ----------------------------------------------------------------------------
void CyclingInput::applyConfiguration()
{
    if (!UserConfigParams::m_cycling_enabled)
        setTelemetry(0.0f, TS_CADENCE, false);

#ifdef ANDROID
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    if (env == NULL || activity == NULL)
        return;

    jclass clazz = env->GetObjectClass(activity);
    if (clazz != NULL)
    {
        jmethodID method = env->GetMethodID(clazz,
            "setCyclingModeConfiguration", "(ZZZ)V");
        if (method != NULL)
        {
            env->CallVoidMethod(activity, method,
                (jboolean)((bool)UserConfigParams::m_cycling_enabled),
                (jboolean)((bool)UserConfigParams::m_cycling_handlebar_controls),
                (jboolean)((bool)UserConfigParams::m_cycling_show_overlay));
        }
        env->DeleteLocalRef(clazz);
    }
    env->DeleteLocalRef(activity);
#endif
}
