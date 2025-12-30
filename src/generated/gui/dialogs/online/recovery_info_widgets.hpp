// Auto-generated from dialogs/online/recovery_info.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"

namespace GUIEngine {

struct RecoveryInfoWidgets
{
    LabelWidget* title = nullptr;
    LabelWidget* info = nullptr;
    RibbonWidget* options = nullptr;
    IconButtonWidget* cancel = nullptr;

    void bind(Screen* screen)
    {
        title = screen->getWidget<LabelWidget>("title");
        info = screen->getWidget<LabelWidget>("info");
        options = screen->getWidget<RibbonWidget>("options");
        cancel = screen->getWidget<IconButtonWidget>("cancel");
    }
};

}  // namespace GUIEngine
