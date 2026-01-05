// Auto-generated from screens/race_setup.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
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

    void bind(AbstractTopLevelContainer* container)
    {
        difficulty = container->getWidget<RibbonWidget>("difficulty");
        novice = container->getWidget<IconButtonWidget>("novice");
        intermediate = container->getWidget<IconButtonWidget>("intermediate");
        expert = container->getWidget<IconButtonWidget>("expert");
        best = container->getWidget<IconButtonWidget>("best");
        gamemode = container->getWidget<DynamicRibbonWidget>("gamemode");
        back = container->getWidget<IconButtonWidget>("back");
    }
};

}  // namespace GUIEngine
