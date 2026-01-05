// Auto-generated from screens/race_result.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"

namespace GUIEngine {

struct RaceResultWidgets
{
    RibbonWidget* operations = nullptr;
    IconButtonWidget* left = nullptr;
    IconButtonWidget* middle = nullptr;
    IconButtonWidget* right = nullptr;

    void bind(AbstractTopLevelContainer* container)
    {
        operations = container->getWidget<RibbonWidget>("operations");
        left = container->getWidget<IconButtonWidget>("left");
        middle = container->getWidget<IconButtonWidget>("middle");
        right = container->getWidget<IconButtonWidget>("right");
    }
};

}  // namespace GUIEngine
