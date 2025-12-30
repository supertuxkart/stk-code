// Auto-generated from screens/ghost_replay_selection.stkgui
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

    void bind(AbstractTopLevelContainer* container)
    {
        back = container->getWidget<IconButtonWidget>("back");
        reload = container->getWidget<IconButtonWidget>("reload");
        replay_list = container->getWidget<ListWidget>("replay_list");
        race_mode = container->getWidget<RibbonWidget>("race_mode");
        tab_time_trial = container->getWidget<IconButtonWidget>("tab_time_trial");
        tab_egg_hunt = container->getWidget<IconButtonWidget>("tab_egg_hunt");
        best_times_toggle = container->getWidget<CheckBoxWidget>("best_times_toggle");
        compare_toggle = container->getWidget<CheckBoxWidget>("compare_toggle");
        compare_toggle_text = container->getWidget<LabelWidget>("compare-toggle-text");
        replay_difficulty_toggle = container->getWidget<CheckBoxWidget>("replay_difficulty_toggle");
        replay_version_toggle = container->getWidget<CheckBoxWidget>("replay_version_toggle");
        replay_multiplayer_toggle = container->getWidget<CheckBoxWidget>("replay_multiplayer_toggle");
        record_ghost = container->getWidget<ButtonWidget>("record-ghost");
    }
};

}  // namespace GUIEngine
