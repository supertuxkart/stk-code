// Auto-generated from screens/credits.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"

namespace GUIEngine {

struct CreditsWidgets
{
    IconButtonWidget* back = nullptr;
    ButtonWidget* stk_website = nullptr;
    IconButtonWidget* logo = nullptr;
    ButtonWidget* donate = nullptr;

    void bind(AbstractTopLevelContainer* container)
    {
        back = container->getWidget<IconButtonWidget>("back");
        stk_website = container->getWidget<ButtonWidget>("stk-website");
        logo = container->getWidget<IconButtonWidget>("logo");
        donate = container->getWidget<ButtonWidget>("donate");
    }
};

}  // namespace GUIEngine
