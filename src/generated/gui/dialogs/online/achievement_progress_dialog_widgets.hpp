// Auto-generated from dialogs/online/achievement_progress_dialog.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"

namespace GUIEngine {

struct AchievementProgressDialogWidgets
{
    LabelWidget* title = nullptr;
    LabelWidget* description = nullptr;
    LabelWidget* main_goal_description = nullptr;
    LabelWidget* main_goal_progress = nullptr;
    ListWidget* progress_tree = nullptr;
    RibbonWidget* options = nullptr;
    IconButtonWidget* ok = nullptr;

    void bind(AbstractTopLevelContainer* container)
    {
        title = container->getWidget<LabelWidget>("title");
        description = container->getWidget<LabelWidget>("description");
        main_goal_description = container->getWidget<LabelWidget>("main-goal-description");
        main_goal_progress = container->getWidget<LabelWidget>("main-goal-progress");
        progress_tree = container->getWidget<ListWidget>("progress-tree");
        options = container->getWidget<RibbonWidget>("options");
        ok = container->getWidget<IconButtonWidget>("ok");
    }
};

}  // namespace GUIEngine
