// Auto-generated from dialogs/online/splitscreen_player_dialog.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"

namespace GUIEngine {

struct SplitscreenPlayerDialogWidgets
{
    LabelWidget* title = nullptr;
    SpinnerWidget* name_spinner = nullptr;
    LabelWidget* name_text = nullptr;
    CheckBoxWidget* handicap = nullptr;
    LabelWidget* handicap_text = nullptr;
    LabelWidget* message_label = nullptr;
    RibbonWidget* options = nullptr;
    IconButtonWidget* cancel = nullptr;
    IconButtonWidget* reset = nullptr;
    IconButtonWidget* add = nullptr;
    IconButtonWidget* connect = nullptr;

    void bind(AbstractTopLevelContainer* container)
    {
        title = container->getWidget<LabelWidget>("title");
        name_spinner = container->getWidget<SpinnerWidget>("name-spinner");
        name_text = container->getWidget<LabelWidget>("name-text");
        handicap = container->getWidget<CheckBoxWidget>("handicap");
        handicap_text = container->getWidget<LabelWidget>("handicap-text");
        message_label = container->getWidget<LabelWidget>("message-label");
        options = container->getWidget<RibbonWidget>("options");
        cancel = container->getWidget<IconButtonWidget>("cancel");
        reset = container->getWidget<IconButtonWidget>("reset");
        add = container->getWidget<IconButtonWidget>("add");
        connect = container->getWidget<IconButtonWidget>("connect");
    }
};

}  // namespace GUIEngine
