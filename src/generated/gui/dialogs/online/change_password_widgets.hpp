// Auto-generated from dialogs/online/change_password.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"

namespace GUIEngine {

struct ChangePasswordWidgets
{
    LabelWidget* title = nullptr;
    TextBoxWidget* current_password = nullptr;
    TextBoxWidget* new_password1 = nullptr;
    TextBoxWidget* new_password2 = nullptr;
    LabelWidget* info = nullptr;
    RibbonWidget* options = nullptr;
    IconButtonWidget* cancel = nullptr;
    IconButtonWidget* submit = nullptr;

    void bind(AbstractTopLevelContainer* container)
    {
        title = container->getWidget<LabelWidget>("title");
        current_password = container->getWidget<TextBoxWidget>("current_password");
        new_password1 = container->getWidget<TextBoxWidget>("new_password1");
        new_password2 = container->getWidget<TextBoxWidget>("new_password2");
        info = container->getWidget<LabelWidget>("info");
        options = container->getWidget<RibbonWidget>("options");
        cancel = container->getWidget<IconButtonWidget>("cancel");
        submit = container->getWidget<IconButtonWidget>("submit");
    }
};

}  // namespace GUIEngine
