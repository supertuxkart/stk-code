// Auto-generated from screens/online/register.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"

namespace GUIEngine {

struct RegisterWidgets
{
    LabelWidget* create_user = nullptr;
    RibbonWidget* mode_tabs = nullptr;
    IconButtonWidget* tab_new_online = nullptr;
    IconButtonWidget* tab_existing_online = nullptr;
    IconButtonWidget* tab_offline = nullptr;
    TextBoxWidget* local_username = nullptr;
    LabelWidget* label_username = nullptr;
    TextBoxWidget* username = nullptr;
    LabelWidget* label_password = nullptr;
    TextBoxWidget* password = nullptr;
    LabelWidget* label_password_confirm = nullptr;
    TextBoxWidget* password_confirm = nullptr;
    LabelWidget* label_email = nullptr;
    TextBoxWidget* email = nullptr;
    ButtonWidget* password_reset = nullptr;
    LabelWidget* info = nullptr;
    RibbonWidget* options = nullptr;
    IconButtonWidget* cancel = nullptr;
    IconButtonWidget* next = nullptr;
    IconButtonWidget* back = nullptr;

    void bind(AbstractTopLevelContainer* container)
    {
        create_user = container->getWidget<LabelWidget>("create_user");
        mode_tabs = container->getWidget<RibbonWidget>("mode_tabs");
        tab_new_online = container->getWidget<IconButtonWidget>("tab_new_online");
        tab_existing_online = container->getWidget<IconButtonWidget>("tab_existing_online");
        tab_offline = container->getWidget<IconButtonWidget>("tab_offline");
        local_username = container->getWidget<TextBoxWidget>("local_username");
        label_username = container->getWidget<LabelWidget>("label_username");
        username = container->getWidget<TextBoxWidget>("username");
        label_password = container->getWidget<LabelWidget>("label_password");
        password = container->getWidget<TextBoxWidget>("password");
        label_password_confirm = container->getWidget<LabelWidget>("label_password_confirm");
        password_confirm = container->getWidget<TextBoxWidget>("password_confirm");
        label_email = container->getWidget<LabelWidget>("label_email");
        email = container->getWidget<TextBoxWidget>("email");
        password_reset = container->getWidget<ButtonWidget>("password_reset");
        info = container->getWidget<LabelWidget>("info");
        options = container->getWidget<RibbonWidget>("options");
        cancel = container->getWidget<IconButtonWidget>("cancel");
        next = container->getWidget<IconButtonWidget>("next");
        back = container->getWidget<IconButtonWidget>("back");
    }
};

}  // namespace GUIEngine
