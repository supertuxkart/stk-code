// Auto-generated from screens/online/lan.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"

namespace GUIEngine {

struct LanWidgets
{
    IconButtonWidget* back = nullptr;
    LabelWidget* title = nullptr;
    IconButtonWidget* logo = nullptr;
    RibbonWidget* lan = nullptr;
    IconButtonWidget* find_lan_server = nullptr;
    IconButtonWidget* create_lan_server = nullptr;

    void bind(AbstractTopLevelContainer* container)
    {
        back = container->getWidget<IconButtonWidget>("back");
        title = container->getWidget<LabelWidget>("title");
        logo = container->getWidget<IconButtonWidget>("logo");
        lan = container->getWidget<RibbonWidget>("lan");
        find_lan_server = container->getWidget<IconButtonWidget>("find_lan_server");
        create_lan_server = container->getWidget<IconButtonWidget>("create_lan_server");
    }
};

}  // namespace GUIEngine
