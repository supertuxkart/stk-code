// Auto-generated from screens/online/server_selection.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"

namespace GUIEngine {

struct ServerSelectionWidgets
{
    IconButtonWidget* back = nullptr;
    IconButtonWidget* bookmark = nullptr;
    IconButtonWidget* reload = nullptr;
    LabelWidget* title_header = nullptr;
    ListWidget* server_list = nullptr;
    TextBoxWidget* searcher = nullptr;
    CheckBoxWidget* private_server = nullptr;
    CheckBoxWidget* ipv6 = nullptr;
    LabelWidget* ipv6_text = nullptr;

    void bind(Screen* screen)
    {
        back = screen->getWidget<IconButtonWidget>("back");
        bookmark = screen->getWidget<IconButtonWidget>("bookmark");
        reload = screen->getWidget<IconButtonWidget>("reload");
        title_header = screen->getWidget<LabelWidget>("title_header");
        server_list = screen->getWidget<ListWidget>("server_list");
        searcher = screen->getWidget<TextBoxWidget>("searcher");
        private_server = screen->getWidget<CheckBoxWidget>("private_server");
        ipv6 = screen->getWidget<CheckBoxWidget>("ipv6");
        ipv6_text = screen->getWidget<LabelWidget>("ipv6_text");
    }
};

}  // namespace GUIEngine
