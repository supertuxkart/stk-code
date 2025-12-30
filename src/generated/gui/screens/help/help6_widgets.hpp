// Auto-generated from screens/help/help6.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"

namespace GUIEngine {

struct Help6Widgets
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

    void bind(Screen* screen)
    {
        back = screen->getWidget<IconButtonWidget>("back");
        category = screen->getWidget<RibbonWidget>("category");
        page1 = screen->getWidget<IconButtonWidget>("page1");
        page2 = screen->getWidget<IconButtonWidget>("page2");
        page3 = screen->getWidget<IconButtonWidget>("page3");
        page4 = screen->getWidget<IconButtonWidget>("page4");
        page5 = screen->getWidget<IconButtonWidget>("page5");
        page6 = screen->getWidget<IconButtonWidget>("page6");
        page7 = screen->getWidget<IconButtonWidget>("page7");
    }
};

}  // namespace GUIEngine
