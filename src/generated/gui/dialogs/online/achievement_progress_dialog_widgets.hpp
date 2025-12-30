// Auto-generated from dialogs/online/achievement_progress_dialog.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
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

    void bind(Screen* screen)
    {
        title = screen->getWidget<LabelWidget>("title");
        description = screen->getWidget<LabelWidget>("description");
        main_goal_description = screen->getWidget<LabelWidget>("main-goal-description");
        main_goal_progress = screen->getWidget<LabelWidget>("main-goal-progress");
        progress_tree = screen->getWidget<ListWidget>("progress-tree");
        options = screen->getWidget<RibbonWidget>("options");
        ok = screen->getWidget<IconButtonWidget>("ok");
    }
};

}  // namespace GUIEngine
