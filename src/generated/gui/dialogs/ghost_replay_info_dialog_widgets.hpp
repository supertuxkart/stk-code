// Auto-generated from dialogs/ghost_replay_info_dialog.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
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

    void bind(Screen* screen)
    {
        name = screen->getWidget<LabelWidget>("name");
        current_replay_info = screen->getWidget<ListWidget>("current_replay_info");
        track_screenshot = screen->getWidget<IconButtonWidget>("track_screenshot");
        info = screen->getWidget<BubbleWidget>("info");
        record_race = screen->getWidget<CheckBoxWidget>("record-race");
        record_race_text = screen->getWidget<LabelWidget>("record-race-text");
        watch_only = screen->getWidget<CheckBoxWidget>("watch-only");
        watch_only_text = screen->getWidget<LabelWidget>("watch-only-text");
        compare_ghost = screen->getWidget<CheckBoxWidget>("compare-ghost");
        compare_ghost_text = screen->getWidget<LabelWidget>("compare-ghost-text");
        actions = screen->getWidget<RibbonWidget>("actions");
        back = screen->getWidget<IconButtonWidget>("back");
        remove = screen->getWidget<IconButtonWidget>("remove");
        add_ghost_to_compare = screen->getWidget<IconButtonWidget>("add-ghost-to-compare");
        start = screen->getWidget<IconButtonWidget>("start");
    }
};

}  // namespace GUIEngine
