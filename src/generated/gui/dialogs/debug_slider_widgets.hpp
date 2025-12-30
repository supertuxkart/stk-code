// Auto-generated from dialogs/debug_slider.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"

namespace GUIEngine {

struct DebugSliderWidgets
{
    LabelWidget* Red = nullptr;
    SpinnerWidget* red_slider = nullptr;
    LabelWidget* Green = nullptr;
    SpinnerWidget* green_slider = nullptr;
    LabelWidget* Blue = nullptr;
    SpinnerWidget* blue_slider = nullptr;
    LabelWidget* SSAO radius = nullptr;
    SpinnerWidget* ssao_radius = nullptr;
    LabelWidget* SSAO k = nullptr;
    SpinnerWidget* ssao_k = nullptr;
    LabelWidget* SSAO Sigma = nullptr;
    SpinnerWidget* ssao_sigma = nullptr;
    ButtonWidget* close = nullptr;

    void bind(Screen* screen)
    {
        Red = screen->getWidget<LabelWidget>("Red");
        red_slider = screen->getWidget<SpinnerWidget>("red_slider");
        Green = screen->getWidget<LabelWidget>("Green");
        green_slider = screen->getWidget<SpinnerWidget>("green_slider");
        Blue = screen->getWidget<LabelWidget>("Blue");
        blue_slider = screen->getWidget<SpinnerWidget>("blue_slider");
        SSAO radius = screen->getWidget<LabelWidget>("SSAO radius");
        ssao_radius = screen->getWidget<SpinnerWidget>("ssao_radius");
        SSAO k = screen->getWidget<LabelWidget>("SSAO k");
        ssao_k = screen->getWidget<SpinnerWidget>("ssao_k");
        SSAO Sigma = screen->getWidget<LabelWidget>("SSAO Sigma");
        ssao_sigma = screen->getWidget<SpinnerWidget>("ssao_sigma");
        close = screen->getWidget<ButtonWidget>("close");
    }
};

}  // namespace GUIEngine
