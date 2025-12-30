// Auto-generated from screens/gp_info.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
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

    void bind(AbstractTopLevelContainer* container)
    {
        back = container->getWidget<IconButtonWidget>("back");
        name = container->getWidget<LabelWidget>("name");
        screenshot = container->getWidget<IconButtonWidget>("screenshot");
        ai_spinner = container->getWidget<SpinnerWidget>("ai-spinner");
        ai_text = container->getWidget<LabelWidget>("ai-text");
        reverse_spinner = container->getWidget<SpinnerWidget>("reverse-spinner");
        reverse_text = container->getWidget<LabelWidget>("reverse-text");
        track_spinner = container->getWidget<SpinnerWidget>("track-spinner");
        track_text = container->getWidget<LabelWidget>("track-text");
        group_spinner = container->getWidget<SpinnerWidget>("group-spinner");
        group_text = container->getWidget<LabelWidget>("group-text");
        time_target_spinner = container->getWidget<SpinnerWidget>("time-target-spinner");
        time_target_text = container->getWidget<LabelWidget>("time-target-text");
        tracks = container->getWidget<ListWidget>("tracks");
        highscore_entries = container->getWidget<ListWidget>("highscore-entries");
        buttons = container->getWidget<RibbonWidget>("buttons");
        continue_ = container->getWidget<IconButtonWidget>("continue");
        start = container->getWidget<IconButtonWidget>("start");
    }
};

}  // namespace GUIEngine
