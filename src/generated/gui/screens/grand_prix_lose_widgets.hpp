// Auto-generated from screens/grand_prix_lose.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/button_widget.hpp"

namespace GUIEngine {

struct GrandPrixLoseWidgets
{
    ButtonWidget* save = nullptr;
    ButtonWidget* continue_ = nullptr;

    void bind(AbstractTopLevelContainer* container)
    {
        save = container->getWidget<ButtonWidget>("save");
        continue_ = container->getWidget<ButtonWidget>("continue");
    }
};

}  // namespace GUIEngine
