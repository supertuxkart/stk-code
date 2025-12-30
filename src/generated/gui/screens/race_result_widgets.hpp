// Auto-generated from screens/race_result.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"

namespace GUIEngine {

struct RaceResultWidgets
{
    RibbonWidget* operations = nullptr;
    IconButtonWidget* left = nullptr;
    IconButtonWidget* middle = nullptr;
    IconButtonWidget* right = nullptr;

    void bind(Screen* screen)
    {
        operations = screen->getWidget<RibbonWidget>("operations");
        left = screen->getWidget<IconButtonWidget>("left");
        middle = screen->getWidget<IconButtonWidget>("middle");
        right = screen->getWidget<IconButtonWidget>("right");
    }
};

}  // namespace GUIEngine
