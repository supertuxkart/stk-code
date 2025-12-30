// Auto-generated from screens/tracks_and_gp.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
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

    void bind(Screen* screen)
    {
        back = screen->getWidget<IconButtonWidget>("back");
        gps = screen->getWidget<DynamicRibbonWidget>("gps");
        search = screen->getWidget<TextBoxWidget>("search");
        favorite = screen->getWidget<CheckBoxWidget>("favorite");
        rand_gp = screen->getWidget<IconButtonWidget>("rand-gp");
        random_track = screen->getWidget<IconButtonWidget>("random_track");
        tracks = screen->getWidget<DynamicRibbonWidget>("tracks");
        trackgroups = screen->getWidget<RibbonWidget>("trackgroups");
    }
};

}  // namespace GUIEngine
