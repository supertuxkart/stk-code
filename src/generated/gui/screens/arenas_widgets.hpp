// Auto-generated from screens/arenas.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"

namespace GUIEngine {

struct ArenasWidgets
{
    IconButtonWidget* back = nullptr;
    TextBoxWidget* search = nullptr;
    CheckBoxWidget* favorite = nullptr;
    DynamicRibbonWidget* tracks = nullptr;
    RibbonWidget* trackgroups = nullptr;
    ButtonWidget* standard = nullptr;
    ButtonWidget* addons = nullptr;
    ButtonWidget* all = nullptr;

    void bind(Screen* screen)
    {
        back = screen->getWidget<IconButtonWidget>("back");
        search = screen->getWidget<TextBoxWidget>("search");
        favorite = screen->getWidget<CheckBoxWidget>("favorite");
        tracks = screen->getWidget<DynamicRibbonWidget>("tracks");
        trackgroups = screen->getWidget<RibbonWidget>("trackgroups");
        standard = screen->getWidget<ButtonWidget>("standard");
        addons = screen->getWidget<ButtonWidget>("addons");
        all = screen->getWidget<ButtonWidget>("all");
    }
};

}  // namespace GUIEngine
