// Auto-generated from screens/online/profile_achievements.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"

namespace GUIEngine {

struct ProfileAchievementsWidgets
{
    IconButtonWidget* back = nullptr;
    LabelWidget* title = nullptr;
    ListWidget* achievements_list = nullptr;

    void bind(AbstractTopLevelContainer* container)
    {
        back = container->getWidget<IconButtonWidget>("back");
        title = container->getWidget<LabelWidget>("title");
        achievements_list = container->getWidget<ListWidget>("achievements_list");
    }
};

}  // namespace GUIEngine
