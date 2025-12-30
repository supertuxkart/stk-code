// Auto-generated from screens/online/network_karts.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
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

    void bind(AbstractTopLevelContainer* container)
    {
        kart_class = container->getWidget<SpinnerWidget>("kart_class");
        search = container->getWidget<TextBoxWidget>("search");
        favorite = container->getWidget<CheckBoxWidget>("favorite");
        karts = container->getWidget<DynamicRibbonWidget>("karts");
        continue_ = container->getWidget<IconButtonWidget>("continue");
        kartgroups = container->getWidget<RibbonWidget>("kartgroups");
        timer = container->getWidget<ProgressBarWidget>("timer");
        back = container->getWidget<IconButtonWidget>("back");
    }
};

}  // namespace GUIEngine
