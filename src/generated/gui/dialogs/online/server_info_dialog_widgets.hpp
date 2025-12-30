// Auto-generated from dialogs/online/server_info_dialog.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"

namespace GUIEngine {

struct ServerInfoDialogWidgets
{
    LabelWidget* title = nullptr;
    LabelWidget* server_info_1 = nullptr;
    LabelWidget* server_info_2 = nullptr;
    LabelWidget* server_info_3 = nullptr;
    LabelWidget* server_info_4 = nullptr;
    ListWidget* player_list = nullptr;
    LabelWidget* label_password = nullptr;
    TextBoxWidget* password = nullptr;
    RibbonWidget* options = nullptr;
    IconButtonWidget* cancel = nullptr;
    IconButtonWidget* bookmark = nullptr;
    IconButtonWidget* join = nullptr;

    void bind(Screen* screen)
    {
        title = screen->getWidget<LabelWidget>("title");
        server_info_1 = screen->getWidget<LabelWidget>("server-info-1");
        server_info_2 = screen->getWidget<LabelWidget>("server-info-2");
        server_info_3 = screen->getWidget<LabelWidget>("server-info-3");
        server_info_4 = screen->getWidget<LabelWidget>("server-info-4");
        player_list = screen->getWidget<ListWidget>("player-list");
        label_password = screen->getWidget<LabelWidget>("label_password");
        password = screen->getWidget<TextBoxWidget>("password");
        options = screen->getWidget<RibbonWidget>("options");
        cancel = screen->getWidget<IconButtonWidget>("cancel");
        bookmark = screen->getWidget<IconButtonWidget>("bookmark");
        join = screen->getWidget<IconButtonWidget>("join");
    }
};

}  // namespace GUIEngine
