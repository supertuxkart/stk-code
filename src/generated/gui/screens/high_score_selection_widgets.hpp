// Auto-generated from screens/high_score_selection.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
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

    void bind(AbstractTopLevelContainer* container)
    {
        back = container->getWidget<IconButtonWidget>("back");
        remove_all = container->getWidget<IconButtonWidget>("remove-all");
        high_scores_list = container->getWidget<ListWidget>("high_scores_list");
        race_mode = container->getWidget<RibbonWidget>("race_mode");
        tab_normal_race = container->getWidget<IconButtonWidget>("tab_normal_race");
        tab_time_trial = container->getWidget<IconButtonWidget>("tab_time_trial");
        tab_egg_hunt = container->getWidget<IconButtonWidget>("tab_egg_hunt");
        tab_lap_trial = container->getWidget<IconButtonWidget>("tab_lap_trial");
        tab_grand_prix = container->getWidget<IconButtonWidget>("tab_grand_prix");
    }
};

}  // namespace GUIEngine
