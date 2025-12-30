// Auto-generated from dialogs/press_a_key_dialog.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
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

    void bind(Screen* screen)
    {
        title = screen->getWidget<LabelWidget>("title");
        buttons = screen->getWidget<RibbonWidget>("buttons");
        cancel = screen->getWidget<IconButtonWidget>("cancel");
        assignEsc = screen->getWidget<IconButtonWidget>("assignEsc");
        assignNone = screen->getWidget<IconButtonWidget>("assignNone");
    }
};

}  // namespace GUIEngine
