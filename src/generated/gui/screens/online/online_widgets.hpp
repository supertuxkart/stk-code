// Auto-generated from screens/online/online.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"

namespace GUIEngine {

struct OnlineWidgets
{
    ButtonWidget* user_id = nullptr;
    ListWidget* news_list = nullptr;
    CheckBoxWidget* enable_splitscreen = nullptr;
    IconButtonWidget* logo = nullptr;
    RibbonWidget* menu_toprow = nullptr;
    IconButtonWidget* lan = nullptr;
    IconButtonWidget* wan = nullptr;
    IconButtonWidget* enter_address = nullptr;
    IconButtonWidget* online = nullptr;
    IconButtonWidget* back = nullptr;

    void bind(Screen* screen)
    {
        user_id = screen->getWidget<ButtonWidget>("user-id");
        news_list = screen->getWidget<ListWidget>("news_list");
        enable_splitscreen = screen->getWidget<CheckBoxWidget>("enable-splitscreen");
        logo = screen->getWidget<IconButtonWidget>("logo");
        menu_toprow = screen->getWidget<RibbonWidget>("menu_toprow");
        lan = screen->getWidget<IconButtonWidget>("lan");
        wan = screen->getWidget<IconButtonWidget>("wan");
        enter_address = screen->getWidget<IconButtonWidget>("enter-address");
        online = screen->getWidget<IconButtonWidget>("online");
        back = screen->getWidget<IconButtonWidget>("back");
    }
};

}  // namespace GUIEngine
