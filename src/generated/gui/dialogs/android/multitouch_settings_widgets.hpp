// Auto-generated from dialogs/android/multitouch_settings.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"

namespace GUIEngine {

struct MultitouchSettingsWidgets
{
    LabelWidget* title = nullptr;
    CheckBoxWidget* buttons_enabled = nullptr;
    CheckBoxWidget* buttons_inverted = nullptr;
    SpinnerWidget* scale = nullptr;
    CheckBoxWidget* auto_acceleration = nullptr;
    CheckBoxWidget* accelerometer = nullptr;
    CheckBoxWidget* gyroscope = nullptr;
    SpinnerWidget* deadzone = nullptr;
    SpinnerWidget* sensitivity_x = nullptr;
    SpinnerWidget* sensitivity_y = nullptr;
    ButtonWidget* restore = nullptr;
    ButtonWidget* close = nullptr;

    void bind(Screen* screen)
    {
        title = screen->getWidget<LabelWidget>("title");
        buttons_enabled = screen->getWidget<CheckBoxWidget>("buttons_enabled");
        buttons_inverted = screen->getWidget<CheckBoxWidget>("buttons_inverted");
        scale = screen->getWidget<SpinnerWidget>("scale");
        auto_acceleration = screen->getWidget<CheckBoxWidget>("auto_acceleration");
        accelerometer = screen->getWidget<CheckBoxWidget>("accelerometer");
        gyroscope = screen->getWidget<CheckBoxWidget>("gyroscope");
        deadzone = screen->getWidget<SpinnerWidget>("deadzone");
        sensitivity_x = screen->getWidget<SpinnerWidget>("sensitivity_x");
        sensitivity_y = screen->getWidget<SpinnerWidget>("sensitivity_y");
        restore = screen->getWidget<ButtonWidget>("restore");
        close = screen->getWidget<ButtonWidget>("close");
    }
};

}  // namespace GUIEngine
