// Auto-generated from screens/arenas.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
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

    void bind(AbstractTopLevelContainer* container)
    {
        back = container->getWidget<IconButtonWidget>("back");
        search = container->getWidget<TextBoxWidget>("search");
        favorite = container->getWidget<CheckBoxWidget>("favorite");
        tracks = container->getWidget<DynamicRibbonWidget>("tracks");
        trackgroups = container->getWidget<RibbonWidget>("trackgroups");
        standard = container->getWidget<ButtonWidget>("standard");
        addons = container->getWidget<ButtonWidget>("addons");
        all = container->getWidget<ButtonWidget>("all");
    }
};

}  // namespace GUIEngine
