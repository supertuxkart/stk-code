// Auto-generated from screens/online/profile_achievements.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"

namespace GUIEngine {

struct ProfileAchievementsWidgets
{
    IconButtonWidget* back = nullptr;
    LabelWidget* title = nullptr;
    ListWidget* achievements_list = nullptr;

    void bind(Screen* screen)
    {
        back = screen->getWidget<IconButtonWidget>("back");
        title = screen->getWidget<LabelWidget>("title");
        achievements_list = screen->getWidget<ListWidget>("achievements_list");
    }
};

}  // namespace GUIEngine
