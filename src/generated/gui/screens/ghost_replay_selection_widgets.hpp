// Auto-generated from screens/ghost_replay_selection.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"

namespace GUIEngine {

struct GhostReplaySelectionWidgets
{
    IconButtonWidget* back = nullptr;
    IconButtonWidget* reload = nullptr;
    ListWidget* replay_list = nullptr;
    RibbonWidget* race_mode = nullptr;
    IconButtonWidget* tab_time_trial = nullptr;
    IconButtonWidget* tab_egg_hunt = nullptr;
    CheckBoxWidget* best_times_toggle = nullptr;
    CheckBoxWidget* compare_toggle = nullptr;
    LabelWidget* compare_toggle_text = nullptr;
    CheckBoxWidget* replay_difficulty_toggle = nullptr;
    CheckBoxWidget* replay_version_toggle = nullptr;
    CheckBoxWidget* replay_multiplayer_toggle = nullptr;
    ButtonWidget* record_ghost = nullptr;

    void bind(Screen* screen)
    {
        back = screen->getWidget<IconButtonWidget>("back");
        reload = screen->getWidget<IconButtonWidget>("reload");
        replay_list = screen->getWidget<ListWidget>("replay_list");
        race_mode = screen->getWidget<RibbonWidget>("race_mode");
        tab_time_trial = screen->getWidget<IconButtonWidget>("tab_time_trial");
        tab_egg_hunt = screen->getWidget<IconButtonWidget>("tab_egg_hunt");
        best_times_toggle = screen->getWidget<CheckBoxWidget>("best_times_toggle");
        compare_toggle = screen->getWidget<CheckBoxWidget>("compare_toggle");
        compare_toggle_text = screen->getWidget<LabelWidget>("compare-toggle-text");
        replay_difficulty_toggle = screen->getWidget<CheckBoxWidget>("replay_difficulty_toggle");
        replay_version_toggle = screen->getWidget<CheckBoxWidget>("replay_version_toggle");
        replay_multiplayer_toggle = screen->getWidget<CheckBoxWidget>("replay_multiplayer_toggle");
        record_ghost = screen->getWidget<ButtonWidget>("record-ghost");
    }
};

}  // namespace GUIEngine
