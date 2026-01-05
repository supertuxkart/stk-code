// Auto-generated from screens/online/online.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
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

    void bind(AbstractTopLevelContainer* container)
    {
        user_id = container->getWidget<ButtonWidget>("user-id");
        news_list = container->getWidget<ListWidget>("news_list");
        enable_splitscreen = container->getWidget<CheckBoxWidget>("enable-splitscreen");
        logo = container->getWidget<IconButtonWidget>("logo");
        menu_toprow = container->getWidget<RibbonWidget>("menu_toprow");
        lan = container->getWidget<IconButtonWidget>("lan");
        wan = container->getWidget<IconButtonWidget>("wan");
        enter_address = container->getWidget<IconButtonWidget>("enter-address");
        online = container->getWidget<IconButtonWidget>("online");
        back = container->getWidget<IconButtonWidget>("back");
    }
};

}  // namespace GUIEngine
