// Auto-generated from dialogs/online/user_info_dialog.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
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

    void bind(AbstractTopLevelContainer* container)
    {
        title = container->getWidget<LabelWidget>("title");
        desc = container->getWidget<LabelWidget>("desc");
        info = container->getWidget<LabelWidget>("info");
        options = container->getWidget<RibbonWidget>("options");
        cancel = container->getWidget<IconButtonWidget>("cancel");
        remove = container->getWidget<IconButtonWidget>("remove");
        friend_ = container->getWidget<IconButtonWidget>("friend");
        accept = container->getWidget<IconButtonWidget>("accept");
        decline = container->getWidget<IconButtonWidget>("decline");
        enter = container->getWidget<IconButtonWidget>("enter");
    }
};

}  // namespace GUIEngine
