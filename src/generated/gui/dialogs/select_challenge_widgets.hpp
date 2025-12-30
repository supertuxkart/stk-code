// Auto-generated from dialogs/select_challenge.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"

namespace GUIEngine {

struct SelectChallengeWidgets
{
    LabelWidget* title = nullptr;
    RibbonWidget* difficulty = nullptr;
    IconButtonWidget* novice = nullptr;
    IconButtonWidget* intermediate = nullptr;
    IconButtonWidget* expert = nullptr;
    IconButtonWidget* supertux = nullptr;
    LabelWidget* challenge_info = nullptr;
    RibbonWidget* actions = nullptr;
    IconButtonWidget* back = nullptr;
    IconButtonWidget* start = nullptr;

    void bind(Screen* screen)
    {
        title = screen->getWidget<LabelWidget>("title");
        difficulty = screen->getWidget<RibbonWidget>("difficulty");
        novice = screen->getWidget<IconButtonWidget>("novice");
        intermediate = screen->getWidget<IconButtonWidget>("intermediate");
        expert = screen->getWidget<IconButtonWidget>("expert");
        supertux = screen->getWidget<IconButtonWidget>("supertux");
        challenge_info = screen->getWidget<LabelWidget>("challenge_info");
        actions = screen->getWidget<RibbonWidget>("actions");
        back = screen->getWidget<IconButtonWidget>("back");
        start = screen->getWidget<IconButtonWidget>("start");
    }
};

}  // namespace GUIEngine
