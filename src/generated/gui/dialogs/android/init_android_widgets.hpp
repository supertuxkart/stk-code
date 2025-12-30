// Auto-generated from dialogs/android/init_android.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
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

    void bind(AbstractTopLevelContainer* container)
    {
        control_type = container->getWidget<RibbonWidget>("control_type");
        accelerometer = container->getWidget<IconButtonWidget>("accelerometer");
        gyroscope = container->getWidget<IconButtonWidget>("gyroscope");
        steering_wheel = container->getWidget<IconButtonWidget>("steering_wheel");
        auto_acceleration = container->getWidget<CheckBoxWidget>("auto_acceleration");
        close = container->getWidget<ButtonWidget>("close");
    }
};

}  // namespace GUIEngine
