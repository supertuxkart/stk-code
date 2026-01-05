// Auto-generated from dialogs/high_score_info_dialog.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"

namespace GUIEngine {

struct HighScoreInfoDialogWidgets
{
    LabelWidget* name = nullptr;
    ListWidget* high_score_list = nullptr;
    LabelWidget* track_name = nullptr;
    LabelWidget* difficulty = nullptr;
    LabelWidget* num_karts = nullptr;
    LabelWidget* num_laps = nullptr;
    LabelWidget* reverse = nullptr;
    IconButtonWidget* track_screenshot = nullptr;
    RibbonWidget* actions = nullptr;
    IconButtonWidget* back = nullptr;
    IconButtonWidget* remove = nullptr;
    IconButtonWidget* start = nullptr;

    void bind(AbstractTopLevelContainer* container)
    {
        name = container->getWidget<LabelWidget>("name");
        high_score_list = container->getWidget<ListWidget>("high_score_list");
        track_name = container->getWidget<LabelWidget>("track-name");
        difficulty = container->getWidget<LabelWidget>("difficulty");
        num_karts = container->getWidget<LabelWidget>("num-karts");
        num_laps = container->getWidget<LabelWidget>("num-laps");
        reverse = container->getWidget<LabelWidget>("reverse");
        track_screenshot = container->getWidget<IconButtonWidget>("track_screenshot");
        actions = container->getWidget<RibbonWidget>("actions");
        back = container->getWidget<IconButtonWidget>("back");
        remove = container->getWidget<IconButtonWidget>("remove");
        start = container->getWidget<IconButtonWidget>("start");
    }
};

}  // namespace GUIEngine
