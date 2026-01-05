// Auto-generated from screens/online/profile_servers.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"

namespace GUIEngine {

struct ProfileServersWidgets
{
    IconButtonWidget* back = nullptr;
    LabelWidget* title = nullptr;
    IconButtonWidget* logo = nullptr;
    RibbonWidget* wan = nullptr;
    IconButtonWidget* find_wan_server = nullptr;
    IconButtonWidget* create_wan_server = nullptr;
    IconButtonWidget* quick_wan_play = nullptr;

    void bind(AbstractTopLevelContainer* container)
    {
        back = container->getWidget<IconButtonWidget>("back");
        title = container->getWidget<LabelWidget>("title");
        logo = container->getWidget<IconButtonWidget>("logo");
        wan = container->getWidget<RibbonWidget>("wan");
        find_wan_server = container->getWidget<IconButtonWidget>("find_wan_server");
        create_wan_server = container->getWidget<IconButtonWidget>("create_wan_server");
        quick_wan_play = container->getWidget<IconButtonWidget>("quick_wan_play");
    }
};

}  // namespace GUIEngine
