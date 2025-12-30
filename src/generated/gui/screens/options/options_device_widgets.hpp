// Auto-generated from screens/options/options_device.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"

namespace GUIEngine {

struct OptionsDeviceWidgets
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
    LabelWidget* title = nullptr;
    ListWidget* actions = nullptr;
    ButtonWidget* delete_ = nullptr;
    ButtonWidget* disable_toggle = nullptr;
    ButtonWidget* back_to_device_list = nullptr;
    ButtonWidget* rename_config = nullptr;
    CheckBoxWidget* force_feedback = nullptr;
    LabelWidget* force_feedback_text = nullptr;

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
        title = container->getWidget<LabelWidget>("title");
        actions = container->getWidget<ListWidget>("actions");
        delete_ = container->getWidget<ButtonWidget>("delete");
        disable_toggle = container->getWidget<ButtonWidget>("disable_toggle");
        back_to_device_list = container->getWidget<ButtonWidget>("back_to_device_list");
        rename_config = container->getWidget<ButtonWidget>("rename_config");
        force_feedback = container->getWidget<CheckBoxWidget>("force_feedback");
        force_feedback_text = container->getWidget<LabelWidget>("force_feedback_text");
    }
};

}  // namespace GUIEngine
