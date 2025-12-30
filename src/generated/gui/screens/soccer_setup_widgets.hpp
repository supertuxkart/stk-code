// Auto-generated from screens/soccer_setup.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
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

    void bind(Screen* screen)
    {
        back = screen->getWidget<IconButtonWidget>("back");
        choose_team = screen->getWidget<BubbleWidget>("choose_team");
        red_team = screen->getWidget<IconButtonWidget>("red_team");
        blue_team = screen->getWidget<IconButtonWidget>("blue_team");
        continue_ = screen->getWidget<ButtonWidget>("continue");
    }
};

}  // namespace GUIEngine
