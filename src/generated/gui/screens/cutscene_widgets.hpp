// Auto-generated from screens/cutscene.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/button_widget.hpp"

namespace GUIEngine {

struct CutsceneWidgets
{
    ButtonWidget* continue_ = nullptr;

    void bind(AbstractTopLevelContainer* container)
    {
        continue_ = container->getWidget<ButtonWidget>("continue");
    }
};

}  // namespace GUIEngine
