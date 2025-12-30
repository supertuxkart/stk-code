// Auto-generated from dialogs/high_score_info_dialog.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
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

    void bind(Screen* screen)
    {
        name = screen->getWidget<LabelWidget>("name");
        high_score_list = screen->getWidget<ListWidget>("high_score_list");
        track_name = screen->getWidget<LabelWidget>("track-name");
        difficulty = screen->getWidget<LabelWidget>("difficulty");
        num_karts = screen->getWidget<LabelWidget>("num-karts");
        num_laps = screen->getWidget<LabelWidget>("num-laps");
        reverse = screen->getWidget<LabelWidget>("reverse");
        track_screenshot = screen->getWidget<IconButtonWidget>("track_screenshot");
        actions = screen->getWidget<RibbonWidget>("actions");
        back = screen->getWidget<IconButtonWidget>("back");
        remove = screen->getWidget<IconButtonWidget>("remove");
        start = screen->getWidget<IconButtonWidget>("start");
    }
};

}  // namespace GUIEngine
