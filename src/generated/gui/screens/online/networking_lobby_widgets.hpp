// Auto-generated from screens/online/networking_lobby.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
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

    void bind(AbstractTopLevelContainer* container)
    {
        lobby_text = container->getWidget<LabelWidget>("lobby-text");
        text = container->getWidget<LabelWidget>("text");
        players = container->getWidget<ListWidget>("players");
        chat = container->getWidget<TextBoxWidget>("chat");
        send = container->getWidget<ButtonWidget>("send");
        emoji = container->getWidget<ButtonWidget>("emoji");
        timeout_message = container->getWidget<LabelWidget>("timeout-message");
        config = container->getWidget<IconButtonWidget>("config");
        start = container->getWidget<IconButtonWidget>("start");
        back = container->getWidget<IconButtonWidget>("back");
    }
};

}  // namespace GUIEngine
