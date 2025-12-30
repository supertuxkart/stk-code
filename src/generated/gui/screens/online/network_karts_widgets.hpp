// Auto-generated from screens/online/network_karts.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/progress_bar_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"

namespace GUIEngine {

struct NetworkKartsWidgets
{
    SpinnerWidget* kart_class = nullptr;
    TextBoxWidget* search = nullptr;
    CheckBoxWidget* favorite = nullptr;
    DynamicRibbonWidget* karts = nullptr;
    IconButtonWidget* continue_ = nullptr;
    RibbonWidget* kartgroups = nullptr;
    ProgressBarWidget* timer = nullptr;
    IconButtonWidget* back = nullptr;

    void bind(Screen* screen)
    {
        kart_class = screen->getWidget<SpinnerWidget>("kart_class");
        search = screen->getWidget<TextBoxWidget>("search");
        favorite = screen->getWidget<CheckBoxWidget>("favorite");
        karts = screen->getWidget<DynamicRibbonWidget>("karts");
        continue_ = screen->getWidget<IconButtonWidget>("continue");
        kartgroups = screen->getWidget<RibbonWidget>("kartgroups");
        timer = screen->getWidget<ProgressBarWidget>("timer");
        back = screen->getWidget<IconButtonWidget>("back");
    }
};

}  // namespace GUIEngine
