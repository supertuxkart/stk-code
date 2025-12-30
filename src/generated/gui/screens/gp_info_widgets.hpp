// Auto-generated from screens/gp_info.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"

namespace GUIEngine {

struct GpInfoWidgets
{
    IconButtonWidget* back = nullptr;
    LabelWidget* name = nullptr;
    IconButtonWidget* screenshot = nullptr;
    SpinnerWidget* ai_spinner = nullptr;
    LabelWidget* ai_text = nullptr;
    SpinnerWidget* reverse_spinner = nullptr;
    LabelWidget* reverse_text = nullptr;
    SpinnerWidget* track_spinner = nullptr;
    LabelWidget* track_text = nullptr;
    SpinnerWidget* group_spinner = nullptr;
    LabelWidget* group_text = nullptr;
    SpinnerWidget* time_target_spinner = nullptr;
    LabelWidget* time_target_text = nullptr;
    ListWidget* tracks = nullptr;
    ListWidget* highscore_entries = nullptr;
    RibbonWidget* buttons = nullptr;
    IconButtonWidget* continue_ = nullptr;
    IconButtonWidget* start = nullptr;

    void bind(Screen* screen)
    {
        back = screen->getWidget<IconButtonWidget>("back");
        name = screen->getWidget<LabelWidget>("name");
        screenshot = screen->getWidget<IconButtonWidget>("screenshot");
        ai_spinner = screen->getWidget<SpinnerWidget>("ai-spinner");
        ai_text = screen->getWidget<LabelWidget>("ai-text");
        reverse_spinner = screen->getWidget<SpinnerWidget>("reverse-spinner");
        reverse_text = screen->getWidget<LabelWidget>("reverse-text");
        track_spinner = screen->getWidget<SpinnerWidget>("track-spinner");
        track_text = screen->getWidget<LabelWidget>("track-text");
        group_spinner = screen->getWidget<SpinnerWidget>("group-spinner");
        group_text = screen->getWidget<LabelWidget>("group-text");
        time_target_spinner = screen->getWidget<SpinnerWidget>("time-target-spinner");
        time_target_text = screen->getWidget<LabelWidget>("time-target-text");
        tracks = screen->getWidget<ListWidget>("tracks");
        highscore_entries = screen->getWidget<ListWidget>("highscore-entries");
        buttons = screen->getWidget<RibbonWidget>("buttons");
        continue_ = screen->getWidget<IconButtonWidget>("continue");
        start = screen->getWidget<IconButtonWidget>("start");
    }
};

}  // namespace GUIEngine
