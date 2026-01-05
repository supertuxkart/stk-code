// Auto-generated from dialogs/online/server_configuration_dialog.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"

namespace GUIEngine {

struct ServerConfigurationDialogWidgets
{
    LabelWidget* title = nullptr;
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
    RibbonWidget* options = nullptr;
    IconButtonWidget* cancel = nullptr;
    IconButtonWidget* ok = nullptr;

    void bind(AbstractTopLevelContainer* container)
    {
        title = container->getWidget<LabelWidget>("title");
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
        options = container->getWidget<RibbonWidget>("options");
        cancel = container->getWidget<IconButtonWidget>("cancel");
        ok = container->getWidget<IconButtonWidget>("ok");
    }
};

}  // namespace GUIEngine
