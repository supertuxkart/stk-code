// Auto-generated from dialogs/enter_address_dialog.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"

namespace GUIEngine {

struct EnterAddressDialogWidgets
{
    LabelWidget* title = nullptr;
    ListWidget* list_history = nullptr;
    TextBoxWidget* textfield = nullptr;
    RibbonWidget* buttons = nullptr;
    IconButtonWidget* cancel = nullptr;
    IconButtonWidget* ok = nullptr;

    void bind(AbstractTopLevelContainer* container)
    {
        title = container->getWidget<LabelWidget>("title");
        list_history = container->getWidget<ListWidget>("list_history");
        textfield = container->getWidget<TextBoxWidget>("textfield");
        buttons = container->getWidget<RibbonWidget>("buttons");
        cancel = container->getWidget<IconButtonWidget>("cancel");
        ok = container->getWidget<IconButtonWidget>("ok");
    }
};

}  // namespace GUIEngine
