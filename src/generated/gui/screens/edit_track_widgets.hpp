// Auto-generated from screens/edit_track.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
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

    void bind(AbstractTopLevelContainer* container)
    {
        selected_track = container->getWidget<LabelWidget>("selected_track");
        search = container->getWidget<TextBoxWidget>("search");
        tracks = container->getWidget<DynamicRibbonWidget>("tracks");
        trackgroups = container->getWidget<RibbonWidget>("trackgroups");
        screenshot = container->getWidget<IconButtonWidget>("screenshot");
        laps_label = container->getWidget<LabelWidget>("laps_label");
        laps = container->getWidget<SpinnerWidget>("laps");
        ok = container->getWidget<ButtonWidget>("ok");
        reverse_label = container->getWidget<LabelWidget>("reverse_label");
        reverse = container->getWidget<CheckBoxWidget>("reverse");
        cancel = container->getWidget<ButtonWidget>("cancel");
    }
};

}  // namespace GUIEngine
