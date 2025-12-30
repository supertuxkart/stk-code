// Auto-generated from dialogs/online/user_info_dialog.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"

namespace GUIEngine {

struct UserInfoDialogWidgets
{
    LabelWidget* title = nullptr;
    LabelWidget* desc = nullptr;
    LabelWidget* info = nullptr;
    RibbonWidget* options = nullptr;
    IconButtonWidget* cancel = nullptr;
    IconButtonWidget* remove = nullptr;
    IconButtonWidget* friend_ = nullptr;
    IconButtonWidget* accept = nullptr;
    IconButtonWidget* decline = nullptr;
    IconButtonWidget* enter = nullptr;

    void bind(Screen* screen)
    {
        title = screen->getWidget<LabelWidget>("title");
        desc = screen->getWidget<LabelWidget>("desc");
        info = screen->getWidget<LabelWidget>("info");
        options = screen->getWidget<RibbonWidget>("options");
        cancel = screen->getWidget<IconButtonWidget>("cancel");
        remove = screen->getWidget<IconButtonWidget>("remove");
        friend_ = screen->getWidget<IconButtonWidget>("friend");
        accept = screen->getWidget<IconButtonWidget>("accept");
        decline = screen->getWidget<IconButtonWidget>("decline");
        enter = screen->getWidget<IconButtonWidget>("enter");
    }
};

}  // namespace GUIEngine
