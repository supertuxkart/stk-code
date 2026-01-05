// Auto-generated from dialogs/android/multitouch_settings.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
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

    void bind(AbstractTopLevelContainer* container)
    {
        title = container->getWidget<LabelWidget>("title");
        buttons_enabled = container->getWidget<CheckBoxWidget>("buttons_enabled");
        buttons_inverted = container->getWidget<CheckBoxWidget>("buttons_inverted");
        scale = container->getWidget<SpinnerWidget>("scale");
        auto_acceleration = container->getWidget<CheckBoxWidget>("auto_acceleration");
        accelerometer = container->getWidget<CheckBoxWidget>("accelerometer");
        gyroscope = container->getWidget<CheckBoxWidget>("gyroscope");
        deadzone = container->getWidget<SpinnerWidget>("deadzone");
        sensitivity_x = container->getWidget<SpinnerWidget>("sensitivity_x");
        sensitivity_y = container->getWidget<SpinnerWidget>("sensitivity_y");
        restore = container->getWidget<ButtonWidget>("restore");
        close = container->getWidget<ButtonWidget>("close");
    }
};

}  // namespace GUIEngine
