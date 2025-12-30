// Auto-generated from screens/online/profile_friends.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
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

    void bind(Screen* screen)
    {
        back = screen->getWidget<IconButtonWidget>("back");
        title = screen->getWidget<LabelWidget>("title");
        profile_tabs = screen->getWidget<RibbonWidget>("profile_tabs");
        tab_achievements = screen->getWidget<IconButtonWidget>("tab_achievements");
        tab_friends = screen->getWidget<IconButtonWidget>("tab_friends");
        tab_settings = screen->getWidget<IconButtonWidget>("tab_settings");
        friends_list = screen->getWidget<ListWidget>("friends_list");
        search_box = screen->getWidget<TextBoxWidget>("search_box");
        search_button = screen->getWidget<ButtonWidget>("search_button");
    }
};

}  // namespace GUIEngine
