// Auto-generated from screens/soccer_setup.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/bubble_widget.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"

namespace GUIEngine {

struct SoccerSetupWidgets
{
    IconButtonWidget* back = nullptr;
    BubbleWidget* choose_team = nullptr;
    IconButtonWidget* red_team = nullptr;
    IconButtonWidget* blue_team = nullptr;
    ButtonWidget* continue_ = nullptr;

    void bind(AbstractTopLevelContainer* container)
    {
        back = container->getWidget<IconButtonWidget>("back");
        choose_team = container->getWidget<BubbleWidget>("choose_team");
        red_team = container->getWidget<IconButtonWidget>("red_team");
        blue_team = container->getWidget<IconButtonWidget>("blue_team");
        continue_ = container->getWidget<ButtonWidget>("continue");
    }
};

}  // namespace GUIEngine
