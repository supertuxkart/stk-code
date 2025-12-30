// Auto-generated from screens/online/register.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
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

    void bind(Screen* screen)
    {
        create_user = screen->getWidget<LabelWidget>("create_user");
        mode_tabs = screen->getWidget<RibbonWidget>("mode_tabs");
        tab_new_online = screen->getWidget<IconButtonWidget>("tab_new_online");
        tab_existing_online = screen->getWidget<IconButtonWidget>("tab_existing_online");
        tab_offline = screen->getWidget<IconButtonWidget>("tab_offline");
        local_username = screen->getWidget<TextBoxWidget>("local_username");
        label_username = screen->getWidget<LabelWidget>("label_username");
        username = screen->getWidget<TextBoxWidget>("username");
        label_password = screen->getWidget<LabelWidget>("label_password");
        password = screen->getWidget<TextBoxWidget>("password");
        label_password_confirm = screen->getWidget<LabelWidget>("label_password_confirm");
        password_confirm = screen->getWidget<TextBoxWidget>("password_confirm");
        label_email = screen->getWidget<LabelWidget>("label_email");
        email = screen->getWidget<TextBoxWidget>("email");
        password_reset = screen->getWidget<ButtonWidget>("password_reset");
        info = screen->getWidget<LabelWidget>("info");
        options = screen->getWidget<RibbonWidget>("options");
        cancel = screen->getWidget<IconButtonWidget>("cancel");
        next = screen->getWidget<IconButtonWidget>("next");
        back = screen->getWidget<IconButtonWidget>("back");
    }
};

}  // namespace GUIEngine
