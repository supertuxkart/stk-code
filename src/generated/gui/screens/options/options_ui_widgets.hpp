// Auto-generated from screens/options/options_ui.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"

namespace GUIEngine {

struct OptionsUiWidgets
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
    SpinnerWidget* base_skinchoice = nullptr;
    SpinnerWidget* variant_skinchoice = nullptr;
    SpinnerWidget* minimap = nullptr;
    SpinnerWidget* font_size = nullptr;
    CheckBoxWidget* showfps = nullptr;
    CheckBoxWidget* karts_powerup_gui = nullptr;
    CheckBoxWidget* story_mode_timer = nullptr;
    CheckBoxWidget* speedrun_timer = nullptr;
    LabelWidget* speedrun_timer_text = nullptr;

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
        base_skinchoice = container->getWidget<SpinnerWidget>("base_skinchoice");
        variant_skinchoice = container->getWidget<SpinnerWidget>("variant_skinchoice");
        minimap = container->getWidget<SpinnerWidget>("minimap");
        font_size = container->getWidget<SpinnerWidget>("font_size");
        showfps = container->getWidget<CheckBoxWidget>("showfps");
        karts_powerup_gui = container->getWidget<CheckBoxWidget>("karts_powerup_gui");
        story_mode_timer = container->getWidget<CheckBoxWidget>("story-mode-timer");
        speedrun_timer = container->getWidget<CheckBoxWidget>("speedrun-timer");
        speedrun_timer_text = container->getWidget<LabelWidget>("speedrun-timer-text");
    }
};

}  // namespace GUIEngine
