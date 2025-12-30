// Auto-generated from screens/online/user_search.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"

namespace GUIEngine {

struct UserSearchWidgets
{
    IconButtonWidget* back = nullptr;
    TextBoxWidget* search_box = nullptr;
    ButtonWidget* search_button = nullptr;
    ListWidget* user_list = nullptr;

    void bind(Screen* screen)
    {
        back = screen->getWidget<IconButtonWidget>("back");
        search_box = screen->getWidget<TextBoxWidget>("search_box");
        search_button = screen->getWidget<ButtonWidget>("search_button");
        user_list = screen->getWidget<ListWidget>("user_list");
    }
};

}  // namespace GUIEngine
