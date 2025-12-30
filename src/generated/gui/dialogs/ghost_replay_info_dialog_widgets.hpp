// Auto-generated from dialogs/ghost_replay_info_dialog.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/bubble_widget.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"

namespace GUIEngine {

struct GhostReplayInfoDialogWidgets
{
    LabelWidget* name = nullptr;
    ListWidget* current_replay_info = nullptr;
    IconButtonWidget* track_screenshot = nullptr;
    BubbleWidget* info = nullptr;
    CheckBoxWidget* record_race = nullptr;
    LabelWidget* record_race_text = nullptr;
    CheckBoxWidget* watch_only = nullptr;
    LabelWidget* watch_only_text = nullptr;
    CheckBoxWidget* compare_ghost = nullptr;
    LabelWidget* compare_ghost_text = nullptr;
    RibbonWidget* actions = nullptr;
    IconButtonWidget* back = nullptr;
    IconButtonWidget* remove = nullptr;
    IconButtonWidget* add_ghost_to_compare = nullptr;
    IconButtonWidget* start = nullptr;

    void bind(AbstractTopLevelContainer* container)
    {
        name = container->getWidget<LabelWidget>("name");
        current_replay_info = container->getWidget<ListWidget>("current_replay_info");
        track_screenshot = container->getWidget<IconButtonWidget>("track_screenshot");
        info = container->getWidget<BubbleWidget>("info");
        record_race = container->getWidget<CheckBoxWidget>("record-race");
        record_race_text = container->getWidget<LabelWidget>("record-race-text");
        watch_only = container->getWidget<CheckBoxWidget>("watch-only");
        watch_only_text = container->getWidget<LabelWidget>("watch-only-text");
        compare_ghost = container->getWidget<CheckBoxWidget>("compare-ghost");
        compare_ghost_text = container->getWidget<LabelWidget>("compare-ghost-text");
        actions = container->getWidget<RibbonWidget>("actions");
        back = container->getWidget<IconButtonWidget>("back");
        remove = container->getWidget<IconButtonWidget>("remove");
        add_ghost_to_compare = container->getWidget<IconButtonWidget>("add-ghost-to-compare");
        start = container->getWidget<IconButtonWidget>("start");
    }
};

}  // namespace GUIEngine
