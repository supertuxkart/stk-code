// Auto-generated from screens/options/options_ui.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
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

    void bind(Screen* screen)
    {
        back = screen->getWidget<IconButtonWidget>("back");
        options_choice = screen->getWidget<RibbonWidget>("options_choice");
        tab_general = screen->getWidget<IconButtonWidget>("tab_general");
        tab_display = screen->getWidget<IconButtonWidget>("tab_display");
        tab_video = screen->getWidget<IconButtonWidget>("tab_video");
        tab_audio = screen->getWidget<IconButtonWidget>("tab_audio");
        tab_ui = screen->getWidget<IconButtonWidget>("tab_ui");
        tab_players = screen->getWidget<IconButtonWidget>("tab_players");
        tab_controls = screen->getWidget<IconButtonWidget>("tab_controls");
        tab_language = screen->getWidget<IconButtonWidget>("tab_language");
        base_skinchoice = screen->getWidget<SpinnerWidget>("base_skinchoice");
        variant_skinchoice = screen->getWidget<SpinnerWidget>("variant_skinchoice");
        minimap = screen->getWidget<SpinnerWidget>("minimap");
        font_size = screen->getWidget<SpinnerWidget>("font_size");
        showfps = screen->getWidget<CheckBoxWidget>("showfps");
        karts_powerup_gui = screen->getWidget<CheckBoxWidget>("karts_powerup_gui");
        story_mode_timer = screen->getWidget<CheckBoxWidget>("story-mode-timer");
        speedrun_timer = screen->getWidget<CheckBoxWidget>("speedrun-timer");
        speedrun_timer_text = screen->getWidget<LabelWidget>("speedrun-timer-text");
    }
};

}  // namespace GUIEngine
