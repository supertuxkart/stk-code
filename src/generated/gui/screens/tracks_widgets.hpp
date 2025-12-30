// Auto-generated from screens/tracks.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
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

    void bind(Screen* screen)
    {
        back = screen->getWidget<IconButtonWidget>("back");
        tracks = screen->getWidget<DynamicRibbonWidget>("tracks");
        trackgroups = screen->getWidget<RibbonWidget>("trackgroups");
        lap_spinner = screen->getWidget<SpinnerWidget>("lap-spinner");
        lap_text = screen->getWidget<LabelWidget>("lap-text");
        reverse = screen->getWidget<CheckBoxWidget>("reverse");
        reverse_text = screen->getWidget<LabelWidget>("reverse-text");
        vote_list = screen->getWidget<ListWidget>("vote-list");
        search_track = screen->getWidget<TextBoxWidget>("search_track");
        timer = screen->getWidget<ProgressBarWidget>("timer");
    }
};

}  // namespace GUIEngine
