// Auto-generated from screens/edit_track.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"

namespace GUIEngine {

struct EditTrackWidgets
{
    LabelWidget* selected_track = nullptr;
    TextBoxWidget* search = nullptr;
    DynamicRibbonWidget* tracks = nullptr;
    RibbonWidget* trackgroups = nullptr;
    IconButtonWidget* screenshot = nullptr;
    LabelWidget* laps_label = nullptr;
    SpinnerWidget* laps = nullptr;
    ButtonWidget* ok = nullptr;
    LabelWidget* reverse_label = nullptr;
    CheckBoxWidget* reverse = nullptr;
    ButtonWidget* cancel = nullptr;

    void bind(Screen* screen)
    {
        selected_track = screen->getWidget<LabelWidget>("selected_track");
        search = screen->getWidget<TextBoxWidget>("search");
        tracks = screen->getWidget<DynamicRibbonWidget>("tracks");
        trackgroups = screen->getWidget<RibbonWidget>("trackgroups");
        screenshot = screen->getWidget<IconButtonWidget>("screenshot");
        laps_label = screen->getWidget<LabelWidget>("laps_label");
        laps = screen->getWidget<SpinnerWidget>("laps");
        ok = screen->getWidget<ButtonWidget>("ok");
        reverse_label = screen->getWidget<LabelWidget>("reverse_label");
        reverse = screen->getWidget<CheckBoxWidget>("reverse");
        cancel = screen->getWidget<ButtonWidget>("cancel");
    }
};

}  // namespace GUIEngine
