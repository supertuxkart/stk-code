// Auto-generated from screens/tracks_and_gp.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"

namespace GUIEngine {

struct TracksAndGpWidgets
{
    IconButtonWidget* back = nullptr;
    DynamicRibbonWidget* gps = nullptr;
    TextBoxWidget* search = nullptr;
    CheckBoxWidget* favorite = nullptr;
    IconButtonWidget* rand_gp = nullptr;
    IconButtonWidget* random_track = nullptr;
    DynamicRibbonWidget* tracks = nullptr;
    RibbonWidget* trackgroups = nullptr;

    void bind(AbstractTopLevelContainer* container)
    {
        back = container->getWidget<IconButtonWidget>("back");
        gps = container->getWidget<DynamicRibbonWidget>("gps");
        search = container->getWidget<TextBoxWidget>("search");
        favorite = container->getWidget<CheckBoxWidget>("favorite");
        rand_gp = container->getWidget<IconButtonWidget>("rand-gp");
        random_track = container->getWidget<IconButtonWidget>("random_track");
        tracks = container->getWidget<DynamicRibbonWidget>("tracks");
        trackgroups = container->getWidget<RibbonWidget>("trackgroups");
    }
};

}  // namespace GUIEngine
