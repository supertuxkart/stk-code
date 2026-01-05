// Auto-generated from screens/online/create_server.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"

namespace GUIEngine {

struct CreateServerWidgets
{
    IconButtonWidget* back = nullptr;
    LabelWidget* title = nullptr;
    TextBoxWidget* name = nullptr;
    SpinnerWidget* max_players = nullptr;
    TextBoxWidget* password = nullptr;
    RibbonWidget* difficulty = nullptr;
    IconButtonWidget* novice = nullptr;
    IconButtonWidget* intermediate = nullptr;
    IconButtonWidget* expert = nullptr;
    IconButtonWidget* best = nullptr;
    RibbonWidget* gamemode = nullptr;
    IconButtonWidget* normal = nullptr;
    IconButtonWidget* timetrial = nullptr;
    IconButtonWidget* _3strikes = nullptr;
    IconButtonWidget* soccer = nullptr;
    LabelWidget* more_options = nullptr;
    SpinnerWidget* more_options_spinner = nullptr;
    LabelWidget* info = nullptr;
    RibbonWidget* options = nullptr;
    IconButtonWidget* cancel = nullptr;
    IconButtonWidget* create = nullptr;

    void bind(AbstractTopLevelContainer* container)
    {
        back = container->getWidget<IconButtonWidget>("back");
        title = container->getWidget<LabelWidget>("title");
        name = container->getWidget<TextBoxWidget>("name");
        max_players = container->getWidget<SpinnerWidget>("max_players");
        password = container->getWidget<TextBoxWidget>("password");
        difficulty = container->getWidget<RibbonWidget>("difficulty");
        novice = container->getWidget<IconButtonWidget>("novice");
        intermediate = container->getWidget<IconButtonWidget>("intermediate");
        expert = container->getWidget<IconButtonWidget>("expert");
        best = container->getWidget<IconButtonWidget>("best");
        gamemode = container->getWidget<RibbonWidget>("gamemode");
        normal = container->getWidget<IconButtonWidget>("normal");
        timetrial = container->getWidget<IconButtonWidget>("timetrial");
        _3strikes = container->getWidget<IconButtonWidget>("3strikes");
        soccer = container->getWidget<IconButtonWidget>("soccer");
        more_options = container->getWidget<LabelWidget>("more-options");
        more_options_spinner = container->getWidget<SpinnerWidget>("more-options-spinner");
        info = container->getWidget<LabelWidget>("info");
        options = container->getWidget<RibbonWidget>("options");
        cancel = container->getWidget<IconButtonWidget>("cancel");
        create = container->getWidget<IconButtonWidget>("create");
    }
};

}  // namespace GUIEngine
