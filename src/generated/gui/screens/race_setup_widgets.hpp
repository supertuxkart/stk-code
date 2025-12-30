// Auto-generated from screens/race_setup.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"

namespace GUIEngine {

struct RaceSetupWidgets
{
    RibbonWidget* difficulty = nullptr;
    IconButtonWidget* novice = nullptr;
    IconButtonWidget* intermediate = nullptr;
    IconButtonWidget* expert = nullptr;
    IconButtonWidget* best = nullptr;
    DynamicRibbonWidget* gamemode = nullptr;
    IconButtonWidget* back = nullptr;

    void bind(Screen* screen)
    {
        difficulty = screen->getWidget<RibbonWidget>("difficulty");
        novice = screen->getWidget<IconButtonWidget>("novice");
        intermediate = screen->getWidget<IconButtonWidget>("intermediate");
        expert = screen->getWidget<IconButtonWidget>("expert");
        best = screen->getWidget<IconButtonWidget>("best");
        gamemode = screen->getWidget<DynamicRibbonWidget>("gamemode");
        back = screen->getWidget<IconButtonWidget>("back");
    }
};

}  // namespace GUIEngine
