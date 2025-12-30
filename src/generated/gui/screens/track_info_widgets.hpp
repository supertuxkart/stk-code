// Auto-generated from screens/track_info.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"

namespace GUIEngine {

struct TrackInfoWidgets
{
    IconButtonWidget* back = nullptr;
    LabelWidget* name = nullptr;
    LabelWidget* highscores = nullptr;
    ListWidget* highscore_entries = nullptr;
    SpinnerWidget* target_type_spinner = nullptr;
    LabelWidget* target_type_text = nullptr;
    SpinnerWidget* target_value_spinner = nullptr;
    LabelWidget* target_value_text = nullptr;
    SpinnerWidget* ai_spinner = nullptr;
    LabelWidget* ai_text = nullptr;
    SpinnerWidget* ai_blue_spinner = nullptr;
    LabelWidget* ai_blue_text = nullptr;
    CheckBoxWidget* option = nullptr;
    LabelWidget* option_text = nullptr;
    CheckBoxWidget* record = nullptr;
    LabelWidget* record_race_text = nullptr;
    IconButtonWidget* screenshot = nullptr;
    LabelWidget* author = nullptr;
    LabelWidget* max_arena_players = nullptr;
    RibbonWidget* buttons = nullptr;
    IconButtonWidget* start = nullptr;

    void bind(AbstractTopLevelContainer* container)
    {
        back = container->getWidget<IconButtonWidget>("back");
        name = container->getWidget<LabelWidget>("name");
        highscores = container->getWidget<LabelWidget>("highscores");
        highscore_entries = container->getWidget<ListWidget>("highscore_entries");
        target_type_spinner = container->getWidget<SpinnerWidget>("target-type-spinner");
        target_type_text = container->getWidget<LabelWidget>("target-type-text");
        target_value_spinner = container->getWidget<SpinnerWidget>("target-value-spinner");
        target_value_text = container->getWidget<LabelWidget>("target-value-text");
        ai_spinner = container->getWidget<SpinnerWidget>("ai-spinner");
        ai_text = container->getWidget<LabelWidget>("ai-text");
        ai_blue_spinner = container->getWidget<SpinnerWidget>("ai-blue-spinner");
        ai_blue_text = container->getWidget<LabelWidget>("ai-blue-text");
        option = container->getWidget<CheckBoxWidget>("option");
        option_text = container->getWidget<LabelWidget>("option-text");
        record = container->getWidget<CheckBoxWidget>("record");
        record_race_text = container->getWidget<LabelWidget>("record-race-text");
        screenshot = container->getWidget<IconButtonWidget>("screenshot");
        author = container->getWidget<LabelWidget>("author");
        max_arena_players = container->getWidget<LabelWidget>("max-arena-players");
        buttons = container->getWidget<RibbonWidget>("buttons");
        start = container->getWidget<IconButtonWidget>("start");
    }
};

}  // namespace GUIEngine
