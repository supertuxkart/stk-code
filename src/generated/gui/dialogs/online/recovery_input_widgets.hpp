// Auto-generated from dialogs/online/recovery_input.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"

namespace GUIEngine {

struct RecoveryInputWidgets
{
    LabelWidget* title = nullptr;
    LabelWidget* message = nullptr;
    TextBoxWidget* username = nullptr;
    TextBoxWidget* email = nullptr;
    LabelWidget* info = nullptr;
    RibbonWidget* options = nullptr;
    IconButtonWidget* cancel = nullptr;
    IconButtonWidget* submit = nullptr;

    void bind(AbstractTopLevelContainer* container)
    {
        title = container->getWidget<LabelWidget>("title");
        message = container->getWidget<LabelWidget>("message");
        username = container->getWidget<TextBoxWidget>("username");
        email = container->getWidget<TextBoxWidget>("email");
        info = container->getWidget<LabelWidget>("info");
        options = container->getWidget<RibbonWidget>("options");
        cancel = container->getWidget<IconButtonWidget>("cancel");
        submit = container->getWidget<IconButtonWidget>("submit");
    }
};

}  // namespace GUIEngine
