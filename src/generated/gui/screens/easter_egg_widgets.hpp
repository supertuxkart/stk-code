// Auto-generated from screens/easter_egg.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"

namespace GUIEngine {

struct EasterEggWidgets
{
    IconButtonWidget* back = nullptr;
    DynamicRibbonWidget* tracks = nullptr;
    RibbonWidget* trackgroups = nullptr;
    ButtonWidget* standard = nullptr;
    ButtonWidget* addons = nullptr;
    ButtonWidget* all = nullptr;

    void bind(Screen* screen)
    {
        back = screen->getWidget<IconButtonWidget>("back");
        tracks = screen->getWidget<DynamicRibbonWidget>("tracks");
        trackgroups = screen->getWidget<RibbonWidget>("trackgroups");
        standard = screen->getWidget<ButtonWidget>("standard");
        addons = screen->getWidget<ButtonWidget>("addons");
        all = screen->getWidget<ButtonWidget>("all");
    }
};

}  // namespace GUIEngine
