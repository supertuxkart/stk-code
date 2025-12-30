// Auto-generated from dialogs/online/recovery_input.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
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

    void bind(Screen* screen)
    {
        title = screen->getWidget<LabelWidget>("title");
        message = screen->getWidget<LabelWidget>("message");
        username = screen->getWidget<TextBoxWidget>("username");
        email = screen->getWidget<TextBoxWidget>("email");
        info = screen->getWidget<LabelWidget>("info");
        options = screen->getWidget<RibbonWidget>("options");
        cancel = screen->getWidget<IconButtonWidget>("cancel");
        submit = screen->getWidget<IconButtonWidget>("submit");
    }
};

}  // namespace GUIEngine
