// Auto-generated from dialogs/confirm_dialog.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"

namespace GUIEngine {

struct ConfirmDialogWidgets
{
    LabelWidget* title = nullptr;
    RibbonWidget* buttons = nullptr;
    IconButtonWidget* cancel = nullptr;
    IconButtonWidget* confirm = nullptr;

    void bind(AbstractTopLevelContainer* container)
    {
        title = container->getWidget<LabelWidget>("title");
        buttons = container->getWidget<RibbonWidget>("buttons");
        cancel = container->getWidget<IconButtonWidget>("cancel");
        confirm = container->getWidget<IconButtonWidget>("confirm");
    }
};

}  // namespace GUIEngine
