// Auto-generated from dialogs/confirm_resolution_dialog.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"

namespace GUIEngine {

struct ConfirmResolutionDialogWidgets
{
    LabelWidget* title = nullptr;
    RibbonWidget* buttons = nullptr;
    IconButtonWidget* cancel = nullptr;
    IconButtonWidget* accept = nullptr;

    void bind(Screen* screen)
    {
        title = screen->getWidget<LabelWidget>("title");
        buttons = screen->getWidget<RibbonWidget>("buttons");
        cancel = screen->getWidget<IconButtonWidget>("cancel");
        accept = screen->getWidget<IconButtonWidget>("accept");
    }
};

}  // namespace GUIEngine
