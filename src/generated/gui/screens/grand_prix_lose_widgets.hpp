// Auto-generated from screens/grand_prix_lose.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
#include "guiengine/widgets/button_widget.hpp"

namespace GUIEngine {

struct GrandPrixLoseWidgets
{
    ButtonWidget* save = nullptr;
    ButtonWidget* continue_ = nullptr;

    void bind(Screen* screen)
    {
        save = screen->getWidget<ButtonWidget>("save");
        continue_ = screen->getWidget<ButtonWidget>("continue");
    }
};

}  // namespace GUIEngine
