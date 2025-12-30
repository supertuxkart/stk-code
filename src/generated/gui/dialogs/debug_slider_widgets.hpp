// Auto-generated from dialogs/debug_slider.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
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
    LabelWidget* SSAO_radius = nullptr;
    SpinnerWidget* ssao_radius = nullptr;
    LabelWidget* SSAO_k = nullptr;
    SpinnerWidget* ssao_k = nullptr;
    LabelWidget* SSAO_Sigma = nullptr;
    SpinnerWidget* ssao_sigma = nullptr;
    ButtonWidget* close = nullptr;

    void bind(AbstractTopLevelContainer* container)
    {
        Red = container->getWidget<LabelWidget>("Red");
        red_slider = container->getWidget<SpinnerWidget>("red_slider");
        Green = container->getWidget<LabelWidget>("Green");
        green_slider = container->getWidget<SpinnerWidget>("green_slider");
        Blue = container->getWidget<LabelWidget>("Blue");
        blue_slider = container->getWidget<SpinnerWidget>("blue_slider");
        SSAO_radius = container->getWidget<LabelWidget>("SSAO radius");
        ssao_radius = container->getWidget<SpinnerWidget>("ssao_radius");
        SSAO_k = container->getWidget<LabelWidget>("SSAO k");
        ssao_k = container->getWidget<SpinnerWidget>("ssao_k");
        SSAO_Sigma = container->getWidget<LabelWidget>("SSAO Sigma");
        ssao_sigma = container->getWidget<SpinnerWidget>("ssao_sigma");
        close = container->getWidget<ButtonWidget>("close");
    }
};

}  // namespace GUIEngine
