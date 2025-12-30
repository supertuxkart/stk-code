// Auto-generated from dialogs/press_a_key_dialog.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"

namespace GUIEngine {

struct PressAKeyDialogWidgets
{
    LabelWidget* title = nullptr;
    RibbonWidget* buttons = nullptr;
    IconButtonWidget* cancel = nullptr;
    IconButtonWidget* assignEsc = nullptr;
    IconButtonWidget* assignNone = nullptr;

    void bind(AbstractTopLevelContainer* container)
    {
        title = container->getWidget<LabelWidget>("title");
        buttons = container->getWidget<RibbonWidget>("buttons");
        cancel = container->getWidget<IconButtonWidget>("cancel");
        assignEsc = container->getWidget<IconButtonWidget>("assignEsc");
        assignNone = container->getWidget<IconButtonWidget>("assignNone");
    }
};

}  // namespace GUIEngine
