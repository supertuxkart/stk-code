// Auto-generated from dialogs/select_challenge.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
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

    void bind(AbstractTopLevelContainer* container)
    {
        title = container->getWidget<LabelWidget>("title");
        difficulty = container->getWidget<RibbonWidget>("difficulty");
        novice = container->getWidget<IconButtonWidget>("novice");
        intermediate = container->getWidget<IconButtonWidget>("intermediate");
        expert = container->getWidget<IconButtonWidget>("expert");
        supertux = container->getWidget<IconButtonWidget>("supertux");
        challenge_info = container->getWidget<LabelWidget>("challenge_info");
        actions = container->getWidget<RibbonWidget>("actions");
        back = container->getWidget<IconButtonWidget>("back");
        start = container->getWidget<IconButtonWidget>("start");
    }
};

}  // namespace GUIEngine
