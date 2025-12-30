// Auto-generated from dialogs/general_text_field_dialog.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"

namespace GUIEngine {

struct GeneralTextFieldDialogWidgets
{
    LabelWidget* title = nullptr;
    TextBoxWidget* textfield = nullptr;
    RibbonWidget* buttons = nullptr;
    IconButtonWidget* cancel = nullptr;
    IconButtonWidget* ok = nullptr;

    void bind(Screen* screen)
    {
        title = screen->getWidget<LabelWidget>("title");
        textfield = screen->getWidget<TextBoxWidget>("textfield");
        buttons = screen->getWidget<RibbonWidget>("buttons");
        cancel = screen->getWidget<IconButtonWidget>("cancel");
        ok = screen->getWidget<IconButtonWidget>("ok");
    }
};

}  // namespace GUIEngine
