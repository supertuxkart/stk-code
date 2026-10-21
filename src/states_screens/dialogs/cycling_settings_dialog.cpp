//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2026 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.

#include "states_screens/dialogs/cycling_settings_dialog.hpp"

#include "config/user_config.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "input/cycling_input.hpp"
#include "states_screens/state_manager.hpp"

#include <algorithm>

using namespace GUIEngine;

// ----------------------------------------------------------------------------
CyclingSettingsDialog::CyclingSettingsDialog(float width, float height)
    : ModalDialog(width, height)
{
    loadFromFile("android/cycling_settings.stkgui");
}

// ----------------------------------------------------------------------------
void CyclingSettingsDialog::beforeAddingWidgets()
{
    getWidget<SpinnerWidget>("start_rpm")->setRange(5, 40, 1);
    getWidget<SpinnerWidget>("full_speed_rpm")->setRange(30, 140, 5);
    getWidget<SpinnerWidget>("minimum_speed")->setRange(20, 80, 5);
    getWidget<SpinnerWidget>("wheel_ratio")->setRange(1.0f, 10.0f, 0.1f);
    getWidget<SpinnerWidget>("sprint_rpm")->setRange(50, 160, 5);
    updateValues();
}

// ----------------------------------------------------------------------------
void CyclingSettingsDialog::updateEnabledState()
{
    const bool enabled =
        getWidget<CheckBoxWidget>("cycling_enabled")->getState();
    getWidget<SpinnerWidget>("start_rpm")->setActive(enabled);
    getWidget<SpinnerWidget>("full_speed_rpm")->setActive(enabled);
    getWidget<SpinnerWidget>("minimum_speed")->setActive(enabled);
    getWidget<SpinnerWidget>("wheel_ratio")->setActive(enabled);
    getWidget<CheckBoxWidget>("show_overlay")->setActive(enabled);
    getWidget<CheckBoxWidget>("auto_nitro")->setActive(enabled);
    getWidget<CheckBoxWidget>("handlebar_controls")->setActive(enabled);
    getWidget<SpinnerWidget>("sprint_rpm")->setActive(enabled &&
        getWidget<CheckBoxWidget>("auto_nitro")->getState());
}

// ----------------------------------------------------------------------------
void CyclingSettingsDialog::updateValues()
{
    getWidget<CheckBoxWidget>("cycling_enabled")->setState(
        UserConfigParams::m_cycling_enabled);
    getWidget<CheckBoxWidget>("show_overlay")->setState(
        UserConfigParams::m_cycling_show_overlay);
    getWidget<SpinnerWidget>("start_rpm")->setValue(
        UserConfigParams::m_cycling_start_rpm);
    getWidget<SpinnerWidget>("full_speed_rpm")->setValue(
        UserConfigParams::m_cycling_full_speed_rpm / 5);
    getWidget<SpinnerWidget>("minimum_speed")->setValue(
        UserConfigParams::m_cycling_min_speed_percent / 5);
    getWidget<SpinnerWidget>("wheel_ratio")->setFloatValue(
        UserConfigParams::m_cycling_wheel_ratio);
    getWidget<CheckBoxWidget>("auto_nitro")->setState(
        UserConfigParams::m_cycling_auto_nitro);
    getWidget<SpinnerWidget>("sprint_rpm")->setValue(
        UserConfigParams::m_cycling_sprint_rpm / 5);
    getWidget<CheckBoxWidget>("handlebar_controls")->setState(
        UserConfigParams::m_cycling_handlebar_controls);
    updateEnabledState();
}

// ----------------------------------------------------------------------------
GUIEngine::EventPropagation CyclingSettingsDialog::processEvent(
    const std::string& event_source)
{
    // A modal dialog receives checkbox events before CheckBoxWidget toggles
    // itself. Handle these two dependent checkboxes here and block the later
    // widget toggle, otherwise their state can never change.
    if (event_source == "cycling_enabled")
    {
        CheckBoxWidget* enabled =
            getWidget<CheckBoxWidget>("cycling_enabled");
        enabled->setState(!enabled->getState());
        updateEnabledState();
        return GUIEngine::EVENT_BLOCK;
    }

    if (event_source == "auto_nitro")
    {
        CheckBoxWidget* auto_nitro =
            getWidget<CheckBoxWidget>("auto_nitro");
        auto_nitro->setState(!auto_nitro->getState());
        updateEnabledState();
        return GUIEngine::EVENT_BLOCK;
    }

    if (event_source != "buttons")
        return GUIEngine::EVENT_LET;

    const std::string& selection = getWidget<RibbonWidget>("buttons")
        ->getSelectionIDString(PLAYER_ID_GAME_MASTER);
    if (selection == "restore")
    {
        UserConfigParams::m_cycling_enabled.revertToDefaults();
        UserConfigParams::m_cycling_show_overlay.revertToDefaults();
        UserConfigParams::m_cycling_start_rpm.revertToDefaults();
        UserConfigParams::m_cycling_full_speed_rpm.revertToDefaults();
        UserConfigParams::m_cycling_min_speed_percent.revertToDefaults();
        UserConfigParams::m_cycling_wheel_ratio.revertToDefaults();
        UserConfigParams::m_cycling_auto_nitro.revertToDefaults();
        UserConfigParams::m_cycling_sprint_rpm.revertToDefaults();
        UserConfigParams::m_cycling_handlebar_controls.revertToDefaults();
        updateValues();
        return GUIEngine::EVENT_BLOCK;
    }

    if (selection == "apply")
    {
        UserConfigParams::m_cycling_enabled =
            getWidget<CheckBoxWidget>("cycling_enabled")->getState();
        UserConfigParams::m_cycling_show_overlay =
            getWidget<CheckBoxWidget>("show_overlay")->getState();

        const int start_rpm =
            getWidget<SpinnerWidget>("start_rpm")->getValue();
        int full_speed_rpm =
            (int)getWidget<SpinnerWidget>("full_speed_rpm")->getFloatValue();
        full_speed_rpm = std::max(start_rpm + 5, full_speed_rpm);
        UserConfigParams::m_cycling_start_rpm = start_rpm;
        UserConfigParams::m_cycling_full_speed_rpm = full_speed_rpm;
        UserConfigParams::m_cycling_min_speed_percent =
            (int)getWidget<SpinnerWidget>("minimum_speed")->getFloatValue();
        UserConfigParams::m_cycling_wheel_ratio =
            getWidget<SpinnerWidget>("wheel_ratio")->getFloatValue();
        UserConfigParams::m_cycling_auto_nitro =
            getWidget<CheckBoxWidget>("auto_nitro")->getState();
        UserConfigParams::m_cycling_sprint_rpm =
            (int)getWidget<SpinnerWidget>("sprint_rpm")->getFloatValue();
        UserConfigParams::m_cycling_handlebar_controls =
            getWidget<CheckBoxWidget>("handlebar_controls")->getState();

        CyclingInput::applyConfiguration();
        user_config->saveConfig();
        ModalDialog::dismiss();
        return GUIEngine::EVENT_BLOCK;
    }

    return GUIEngine::EVENT_LET;
}
