// Auto-generated from screens/help/help5.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"

namespace GUIEngine {

struct Help5Widgets
{
    IconButtonWidget* back = nullptr;
    RibbonWidget* category = nullptr;
    IconButtonWidget* page1 = nullptr;
    IconButtonWidget* page2 = nullptr;
    IconButtonWidget* page3 = nullptr;
    IconButtonWidget* page4 = nullptr;
    IconButtonWidget* page5 = nullptr;
    IconButtonWidget* page6 = nullptr;
    IconButtonWidget* page7 = nullptr;

    void bind(AbstractTopLevelContainer* container)
    {
        back = container->getWidget<IconButtonWidget>("back");
        category = container->getWidget<RibbonWidget>("category");
        page1 = container->getWidget<IconButtonWidget>("page1");
        page2 = container->getWidget<IconButtonWidget>("page2");
        page3 = container->getWidget<IconButtonWidget>("page3");
        page4 = container->getWidget<IconButtonWidget>("page4");
        page5 = container->getWidget<IconButtonWidget>("page5");
        page6 = container->getWidget<IconButtonWidget>("page6");
        page7 = container->getWidget<IconButtonWidget>("page7");
    }
};

}  // namespace GUIEngine
