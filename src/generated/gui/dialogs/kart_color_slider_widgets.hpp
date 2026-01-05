// Auto-generated from dialogs/kart_color_slider.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/model_view_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"

namespace GUIEngine {

struct KartColorSliderWidgets
{
    ModelViewWidget* model = nullptr;
    SpinnerWidget* toggle_slider = nullptr;
    SpinnerWidget* color_slider = nullptr;
    RibbonWidget* buttons = nullptr;
    IconButtonWidget* cancel = nullptr;
    IconButtonWidget* apply = nullptr;

    void bind(AbstractTopLevelContainer* container)
    {
        model = container->getWidget<ModelViewWidget>("model");
        toggle_slider = container->getWidget<SpinnerWidget>("toggle-slider");
        color_slider = container->getWidget<SpinnerWidget>("color-slider");
        buttons = container->getWidget<RibbonWidget>("buttons");
        cancel = container->getWidget<IconButtonWidget>("cancel");
        apply = container->getWidget<IconButtonWidget>("apply");
    }
};

}  // namespace GUIEngine
