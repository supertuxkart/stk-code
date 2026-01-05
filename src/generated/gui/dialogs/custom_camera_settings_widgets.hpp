// Auto-generated from dialogs/custom_camera_settings.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"

namespace GUIEngine {

struct CustomCameraSettingsWidgets
{
    LabelWidget* title = nullptr;
    LabelWidget* camera_name = nullptr;
    SpinnerWidget* fov = nullptr;
    SpinnerWidget* camera_distance = nullptr;
    SpinnerWidget* camera_angle = nullptr;
    SpinnerWidget* smooth_position = nullptr;
    SpinnerWidget* smooth_rotation = nullptr;
    SpinnerWidget* backward_camera_distance = nullptr;
    SpinnerWidget* backward_camera_angle = nullptr;
    CheckBoxWidget* use_soccer_camera = nullptr;
    RibbonWidget* buttons = nullptr;
    IconButtonWidget* reset = nullptr;
    IconButtonWidget* cancel = nullptr;
    IconButtonWidget* apply = nullptr;

    void bind(AbstractTopLevelContainer* container)
    {
        title = container->getWidget<LabelWidget>("title");
        camera_name = container->getWidget<LabelWidget>("camera_name");
        fov = container->getWidget<SpinnerWidget>("fov");
        camera_distance = container->getWidget<SpinnerWidget>("camera_distance");
        camera_angle = container->getWidget<SpinnerWidget>("camera_angle");
        smooth_position = container->getWidget<SpinnerWidget>("smooth_position");
        smooth_rotation = container->getWidget<SpinnerWidget>("smooth_rotation");
        backward_camera_distance = container->getWidget<SpinnerWidget>("backward_camera_distance");
        backward_camera_angle = container->getWidget<SpinnerWidget>("backward_camera_angle");
        use_soccer_camera = container->getWidget<CheckBoxWidget>("use_soccer_camera");
        buttons = container->getWidget<RibbonWidget>("buttons");
        reset = container->getWidget<IconButtonWidget>("reset");
        cancel = container->getWidget<IconButtonWidget>("cancel");
        apply = container->getWidget<IconButtonWidget>("apply");
    }
};

}  // namespace GUIEngine
