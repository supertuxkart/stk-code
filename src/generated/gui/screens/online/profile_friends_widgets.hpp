// Auto-generated from screens/online/profile_friends.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"

namespace GUIEngine {

struct ProfileFriendsWidgets
{
    IconButtonWidget* back = nullptr;
    LabelWidget* title = nullptr;
    RibbonWidget* profile_tabs = nullptr;
    IconButtonWidget* tab_achievements = nullptr;
    IconButtonWidget* tab_friends = nullptr;
    IconButtonWidget* tab_settings = nullptr;
    ListWidget* friends_list = nullptr;
    TextBoxWidget* search_box = nullptr;
    ButtonWidget* search_button = nullptr;

    void bind(AbstractTopLevelContainer* container)
    {
        back = container->getWidget<IconButtonWidget>("back");
        title = container->getWidget<LabelWidget>("title");
        profile_tabs = container->getWidget<RibbonWidget>("profile_tabs");
        tab_achievements = container->getWidget<IconButtonWidget>("tab_achievements");
        tab_friends = container->getWidget<IconButtonWidget>("tab_friends");
        tab_settings = container->getWidget<IconButtonWidget>("tab_settings");
        friends_list = container->getWidget<ListWidget>("friends_list");
        search_box = container->getWidget<TextBoxWidget>("search_box");
        search_button = container->getWidget<ButtonWidget>("search_button");
    }
};

}  // namespace GUIEngine
