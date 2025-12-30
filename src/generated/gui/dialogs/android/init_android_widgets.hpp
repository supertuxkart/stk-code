// Auto-generated from dialogs/android/init_android.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"

namespace GUIEngine {

struct InitAndroidWidgets
{
    RibbonWidget* control_type = nullptr;
    IconButtonWidget* accelerometer = nullptr;
    IconButtonWidget* gyroscope = nullptr;
    IconButtonWidget* steering_wheel = nullptr;
    CheckBoxWidget* auto_acceleration = nullptr;
    ButtonWidget* close = nullptr;

    void bind(Screen* screen)
    {
        control_type = screen->getWidget<RibbonWidget>("control_type");
        accelerometer = screen->getWidget<IconButtonWidget>("accelerometer");
        gyroscope = screen->getWidget<IconButtonWidget>("gyroscope");
        steering_wheel = screen->getWidget<IconButtonWidget>("steering_wheel");
        auto_acceleration = screen->getWidget<CheckBoxWidget>("auto_acceleration");
        close = screen->getWidget<ButtonWidget>("close");
    }
};

}  // namespace GUIEngine
