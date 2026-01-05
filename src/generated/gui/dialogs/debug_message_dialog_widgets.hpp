// Auto-generated from dialogs/debug_message_dialog.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"

namespace GUIEngine {

struct DebugMessageDialogWidgets
{
    LabelWidget* title = nullptr;
    ButtonWidget* continue_ = nullptr;

    void bind(AbstractTopLevelContainer* container)
    {
        title = container->getWidget<LabelWidget>("title");
        continue_ = container->getWidget<ButtonWidget>("continue");
    }
};

}  // namespace GUIEngine
