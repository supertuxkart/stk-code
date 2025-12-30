// Auto-generated from dialogs/kart_color_slider.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
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

    void bind(Screen* screen)
    {
        model = screen->getWidget<ModelViewWidget>("model");
        toggle_slider = screen->getWidget<SpinnerWidget>("toggle-slider");
        color_slider = screen->getWidget<SpinnerWidget>("color-slider");
        buttons = screen->getWidget<RibbonWidget>("buttons");
        cancel = screen->getWidget<IconButtonWidget>("cancel");
        apply = screen->getWidget<IconButtonWidget>("apply");
    }
};

}  // namespace GUIEngine
