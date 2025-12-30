// Auto-generated from screens/high_score_selection.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"

namespace GUIEngine {

struct HighScoreSelectionWidgets
{
    IconButtonWidget* back = nullptr;
    IconButtonWidget* remove_all = nullptr;
    ListWidget* high_scores_list = nullptr;
    RibbonWidget* race_mode = nullptr;
    IconButtonWidget* tab_normal_race = nullptr;
    IconButtonWidget* tab_time_trial = nullptr;
    IconButtonWidget* tab_egg_hunt = nullptr;
    IconButtonWidget* tab_lap_trial = nullptr;
    IconButtonWidget* tab_grand_prix = nullptr;

    void bind(Screen* screen)
    {
        back = screen->getWidget<IconButtonWidget>("back");
        remove_all = screen->getWidget<IconButtonWidget>("remove-all");
        high_scores_list = screen->getWidget<ListWidget>("high_scores_list");
        race_mode = screen->getWidget<RibbonWidget>("race_mode");
        tab_normal_race = screen->getWidget<IconButtonWidget>("tab_normal_race");
        tab_time_trial = screen->getWidget<IconButtonWidget>("tab_time_trial");
        tab_egg_hunt = screen->getWidget<IconButtonWidget>("tab_egg_hunt");
        tab_lap_trial = screen->getWidget<IconButtonWidget>("tab_lap_trial");
        tab_grand_prix = screen->getWidget<IconButtonWidget>("tab_grand_prix");
    }
};

}  // namespace GUIEngine
