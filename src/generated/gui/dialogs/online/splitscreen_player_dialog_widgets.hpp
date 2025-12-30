// Auto-generated from dialogs/online/splitscreen_player_dialog.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
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

    void bind(Screen* screen)
    {
        title = screen->getWidget<LabelWidget>("title");
        name_spinner = screen->getWidget<SpinnerWidget>("name-spinner");
        name_text = screen->getWidget<LabelWidget>("name-text");
        handicap = screen->getWidget<CheckBoxWidget>("handicap");
        handicap_text = screen->getWidget<LabelWidget>("handicap-text");
        message_label = screen->getWidget<LabelWidget>("message-label");
        options = screen->getWidget<RibbonWidget>("options");
        cancel = screen->getWidget<IconButtonWidget>("cancel");
        reset = screen->getWidget<IconButtonWidget>("reset");
        add = screen->getWidget<IconButtonWidget>("add");
        connect = screen->getWidget<IconButtonWidget>("connect");
    }
};

}  // namespace GUIEngine
