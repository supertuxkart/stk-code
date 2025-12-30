// Auto-generated from screens/credits.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"

namespace GUIEngine {

struct CreditsWidgets
{
    IconButtonWidget* back = nullptr;
    ButtonWidget* stk_website = nullptr;
    IconButtonWidget* logo = nullptr;
    ButtonWidget* donate = nullptr;

    void bind(Screen* screen)
    {
        back = screen->getWidget<IconButtonWidget>("back");
        stk_website = screen->getWidget<ButtonWidget>("stk-website");
        logo = screen->getWidget<IconButtonWidget>("logo");
        donate = screen->getWidget<ButtonWidget>("donate");
    }
};

}  // namespace GUIEngine
