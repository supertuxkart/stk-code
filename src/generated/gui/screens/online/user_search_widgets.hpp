// Auto-generated from screens/online/user_search.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
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

    void bind(AbstractTopLevelContainer* container)
    {
        back = container->getWidget<IconButtonWidget>("back");
        search_box = container->getWidget<TextBoxWidget>("search_box");
        search_button = container->getWidget<ButtonWidget>("search_button");
        user_list = container->getWidget<ListWidget>("user_list");
    }
};

}  // namespace GUIEngine
