// Auto-generated from screens/tracks.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/progress_bar_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"

namespace GUIEngine {

struct TracksWidgets
{
    IconButtonWidget* back = nullptr;
    DynamicRibbonWidget* tracks = nullptr;
    RibbonWidget* trackgroups = nullptr;
    SpinnerWidget* lap_spinner = nullptr;
    LabelWidget* lap_text = nullptr;
    CheckBoxWidget* reverse = nullptr;
    LabelWidget* reverse_text = nullptr;
    ListWidget* vote_list = nullptr;
    TextBoxWidget* search_track = nullptr;
    ProgressBarWidget* timer = nullptr;

    void bind(AbstractTopLevelContainer* container)
    {
        back = container->getWidget<IconButtonWidget>("back");
        tracks = container->getWidget<DynamicRibbonWidget>("tracks");
        trackgroups = container->getWidget<RibbonWidget>("trackgroups");
        lap_spinner = container->getWidget<SpinnerWidget>("lap-spinner");
        lap_text = container->getWidget<LabelWidget>("lap-text");
        reverse = container->getWidget<CheckBoxWidget>("reverse");
        reverse_text = container->getWidget<LabelWidget>("reverse-text");
        vote_list = container->getWidget<ListWidget>("vote-list");
        search_track = container->getWidget<TextBoxWidget>("search_track");
        timer = container->getWidget<ProgressBarWidget>("timer");
    }
};

}  // namespace GUIEngine
