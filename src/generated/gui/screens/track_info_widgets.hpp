// Auto-generated from screens/track_info.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
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

    void bind(Screen* screen)
    {
        back = screen->getWidget<IconButtonWidget>("back");
        name = screen->getWidget<LabelWidget>("name");
        highscores = screen->getWidget<LabelWidget>("highscores");
        highscore_entries = screen->getWidget<ListWidget>("highscore_entries");
        target_type_spinner = screen->getWidget<SpinnerWidget>("target-type-spinner");
        target_type_text = screen->getWidget<LabelWidget>("target-type-text");
        target_value_spinner = screen->getWidget<SpinnerWidget>("target-value-spinner");
        target_value_text = screen->getWidget<LabelWidget>("target-value-text");
        ai_spinner = screen->getWidget<SpinnerWidget>("ai-spinner");
        ai_text = screen->getWidget<LabelWidget>("ai-text");
        ai_blue_spinner = screen->getWidget<SpinnerWidget>("ai-blue-spinner");
        ai_blue_text = screen->getWidget<LabelWidget>("ai-blue-text");
        option = screen->getWidget<CheckBoxWidget>("option");
        option_text = screen->getWidget<LabelWidget>("option-text");
        record = screen->getWidget<CheckBoxWidget>("record");
        record_race_text = screen->getWidget<LabelWidget>("record-race-text");
        screenshot = screen->getWidget<IconButtonWidget>("screenshot");
        author = screen->getWidget<LabelWidget>("author");
        max_arena_players = screen->getWidget<LabelWidget>("max-arena-players");
        buttons = screen->getWidget<RibbonWidget>("buttons");
        start = screen->getWidget<IconButtonWidget>("start");
    }
};

}  // namespace GUIEngine
