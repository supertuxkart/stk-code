// Auto-generated from screens/online/networking_lobby.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"

namespace GUIEngine {

struct NetworkingLobbyWidgets
{
    LabelWidget* lobby_text = nullptr;
    LabelWidget* text = nullptr;
    ListWidget* players = nullptr;
    TextBoxWidget* chat = nullptr;
    ButtonWidget* send = nullptr;
    ButtonWidget* emoji = nullptr;
    LabelWidget* timeout_message = nullptr;
    IconButtonWidget* config = nullptr;
    IconButtonWidget* start = nullptr;
    IconButtonWidget* back = nullptr;

    void bind(Screen* screen)
    {
        lobby_text = screen->getWidget<LabelWidget>("lobby-text");
        text = screen->getWidget<LabelWidget>("text");
        players = screen->getWidget<ListWidget>("players");
        chat = screen->getWidget<TextBoxWidget>("chat");
        send = screen->getWidget<ButtonWidget>("send");
        emoji = screen->getWidget<ButtonWidget>("emoji");
        timeout_message = screen->getWidget<LabelWidget>("timeout-message");
        config = screen->getWidget<IconButtonWidget>("config");
        start = screen->getWidget<IconButtonWidget>("start");
        back = screen->getWidget<IconButtonWidget>("back");
    }
};

}  // namespace GUIEngine
