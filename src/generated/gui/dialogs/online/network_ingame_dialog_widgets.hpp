// Auto-generated from dialogs/online/network_ingame_dialog.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"

namespace GUIEngine {

struct NetworkIngameDialogWidgets
{
    TextBoxWidget* chat = nullptr;
    ButtonWidget* team = nullptr;
    ButtonWidget* send = nullptr;
    ButtonWidget* emoji = nullptr;
    RibbonWidget* backbtnribbon = nullptr;
    IconButtonWidget* backbtn = nullptr;
    IconButtonWidget* touch_device = nullptr;
    RibbonWidget* choiceribbon = nullptr;
    IconButtonWidget* exit = nullptr;
    IconButtonWidget* newrace = nullptr;
    IconButtonWidget* restart = nullptr;
    IconButtonWidget* endrace = nullptr;
    IconButtonWidget* options = nullptr;
    IconButtonWidget* help = nullptr;

    void bind(Screen* screen)
    {
        chat = screen->getWidget<TextBoxWidget>("chat");
        team = screen->getWidget<ButtonWidget>("team");
        send = screen->getWidget<ButtonWidget>("send");
        emoji = screen->getWidget<ButtonWidget>("emoji");
        backbtnribbon = screen->getWidget<RibbonWidget>("backbtnribbon");
        backbtn = screen->getWidget<IconButtonWidget>("backbtn");
        touch_device = screen->getWidget<IconButtonWidget>("touch_device");
        choiceribbon = screen->getWidget<RibbonWidget>("choiceribbon");
        exit = screen->getWidget<IconButtonWidget>("exit");
        newrace = screen->getWidget<IconButtonWidget>("newrace");
        restart = screen->getWidget<IconButtonWidget>("restart");
        endrace = screen->getWidget<IconButtonWidget>("endrace");
        options = screen->getWidget<IconButtonWidget>("options");
        help = screen->getWidget<IconButtonWidget>("help");
    }
};

}  // namespace GUIEngine
