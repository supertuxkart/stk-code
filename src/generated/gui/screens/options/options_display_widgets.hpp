// Auto-generated from screens/options/options_display.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"

namespace GUIEngine {

struct OptionsDisplayWidgets
{
    IconButtonWidget* back = nullptr;
    RibbonWidget* options_choice = nullptr;
    IconButtonWidget* tab_general = nullptr;
    IconButtonWidget* tab_display = nullptr;
    IconButtonWidget* tab_video = nullptr;
    IconButtonWidget* tab_audio = nullptr;
    IconButtonWidget* tab_ui = nullptr;
    IconButtonWidget* tab_players = nullptr;
    IconButtonWidget* tab_controls = nullptr;
    IconButtonWidget* tab_language = nullptr;
    CheckBoxWidget* fullscreen = nullptr;
    LabelWidget* fullscreenText = nullptr;
    CheckBoxWidget* rememberWinpos = nullptr;
    LabelWidget* rememberWinposText = nullptr;
    DynamicRibbonWidget* resolutions = nullptr;
    ButtonWidget* apply_resolution = nullptr;
    SpinnerWidget* camera_preset = nullptr;
    ButtonWidget* custom_camera = nullptr;
    SpinnerWidget* splitscreen_method = nullptr;

    void bind(AbstractTopLevelContainer* container)
    {
        back = container->getWidget<IconButtonWidget>("back");
        options_choice = container->getWidget<RibbonWidget>("options_choice");
        tab_general = container->getWidget<IconButtonWidget>("tab_general");
        tab_display = container->getWidget<IconButtonWidget>("tab_display");
        tab_video = container->getWidget<IconButtonWidget>("tab_video");
        tab_audio = container->getWidget<IconButtonWidget>("tab_audio");
        tab_ui = container->getWidget<IconButtonWidget>("tab_ui");
        tab_players = container->getWidget<IconButtonWidget>("tab_players");
        tab_controls = container->getWidget<IconButtonWidget>("tab_controls");
        tab_language = container->getWidget<IconButtonWidget>("tab_language");
        fullscreen = container->getWidget<CheckBoxWidget>("fullscreen");
        fullscreenText = container->getWidget<LabelWidget>("fullscreenText");
        rememberWinpos = container->getWidget<CheckBoxWidget>("rememberWinpos");
        rememberWinposText = container->getWidget<LabelWidget>("rememberWinposText");
        resolutions = container->getWidget<DynamicRibbonWidget>("resolutions");
        apply_resolution = container->getWidget<ButtonWidget>("apply_resolution");
        camera_preset = container->getWidget<SpinnerWidget>("camera_preset");
        custom_camera = container->getWidget<ButtonWidget>("custom_camera");
        splitscreen_method = container->getWidget<SpinnerWidget>("splitscreen_method");
    }
};

}  // namespace GUIEngine
