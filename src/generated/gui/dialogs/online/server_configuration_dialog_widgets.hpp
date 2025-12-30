// Auto-generated from dialogs/online/server_configuration_dialog.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
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
    IconButtonWidget* 3strikes = nullptr;
    IconButtonWidget* soccer = nullptr;
    LabelWidget* more_options = nullptr;
    SpinnerWidget* more_options_spinner = nullptr;
    RibbonWidget* options = nullptr;
    IconButtonWidget* cancel = nullptr;
    IconButtonWidget* ok = nullptr;

    void bind(Screen* screen)
    {
        title = screen->getWidget<LabelWidget>("title");
        difficulty = screen->getWidget<RibbonWidget>("difficulty");
        novice = screen->getWidget<IconButtonWidget>("novice");
        intermediate = screen->getWidget<IconButtonWidget>("intermediate");
        expert = screen->getWidget<IconButtonWidget>("expert");
        best = screen->getWidget<IconButtonWidget>("best");
        gamemode = screen->getWidget<RibbonWidget>("gamemode");
        normal = screen->getWidget<IconButtonWidget>("normal");
        timetrial = screen->getWidget<IconButtonWidget>("timetrial");
        3strikes = screen->getWidget<IconButtonWidget>("3strikes");
        soccer = screen->getWidget<IconButtonWidget>("soccer");
        more_options = screen->getWidget<LabelWidget>("more-options");
        more_options_spinner = screen->getWidget<SpinnerWidget>("more-options-spinner");
        options = screen->getWidget<RibbonWidget>("options");
        cancel = screen->getWidget<IconButtonWidget>("cancel");
        ok = screen->getWidget<IconButtonWidget>("ok");
    }
};

}  // namespace GUIEngine
