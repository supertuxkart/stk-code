// Auto-generated from screens/main_menu.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"

namespace GUIEngine {

struct MainMenuWidgets
{
    IconButtonWidget* logo = nullptr;
    RibbonWidget* menu_toprow = nullptr;
    IconButtonWidget* story = nullptr;
    IconButtonWidget* new_ = nullptr;
    IconButtonWidget* multiplayer = nullptr;
    IconButtonWidget* online = nullptr;
    IconButtonWidget* addons = nullptr;
    LabelWidget* info_addons = nullptr;
    RibbonWidget* menu_bottomrow = nullptr;
    IconButtonWidget* test_gpwin = nullptr;
    IconButtonWidget* test_gplose = nullptr;
    IconButtonWidget* test_unlocked = nullptr;
    IconButtonWidget* test_unlocked2 = nullptr;
    IconButtonWidget* test_intro = nullptr;
    IconButtonWidget* test_outro = nullptr;
    IconButtonWidget* options = nullptr;
    IconButtonWidget* help = nullptr;
    IconButtonWidget* startTutorial = nullptr;
    IconButtonWidget* highscores = nullptr;
    IconButtonWidget* achievements = nullptr;
    IconButtonWidget* gpEditor = nullptr;
    IconButtonWidget* about = nullptr;
    IconButtonWidget* quit = nullptr;
    ButtonWidget* user_id = nullptr;

    void bind(Screen* screen)
    {
        logo = screen->getWidget<IconButtonWidget>("logo");
        menu_toprow = screen->getWidget<RibbonWidget>("menu_toprow");
        story = screen->getWidget<IconButtonWidget>("story");
        new_ = screen->getWidget<IconButtonWidget>("new");
        multiplayer = screen->getWidget<IconButtonWidget>("multiplayer");
        online = screen->getWidget<IconButtonWidget>("online");
        addons = screen->getWidget<IconButtonWidget>("addons");
        info_addons = screen->getWidget<LabelWidget>("info_addons");
        menu_bottomrow = screen->getWidget<RibbonWidget>("menu_bottomrow");
        test_gpwin = screen->getWidget<IconButtonWidget>("test_gpwin");
        test_gplose = screen->getWidget<IconButtonWidget>("test_gplose");
        test_unlocked = screen->getWidget<IconButtonWidget>("test_unlocked");
        test_unlocked2 = screen->getWidget<IconButtonWidget>("test_unlocked2");
        test_intro = screen->getWidget<IconButtonWidget>("test_intro");
        test_outro = screen->getWidget<IconButtonWidget>("test_outro");
        options = screen->getWidget<IconButtonWidget>("options");
        help = screen->getWidget<IconButtonWidget>("help");
        startTutorial = screen->getWidget<IconButtonWidget>("startTutorial");
        highscores = screen->getWidget<IconButtonWidget>("highscores");
        achievements = screen->getWidget<IconButtonWidget>("achievements");
        gpEditor = screen->getWidget<IconButtonWidget>("gpEditor");
        about = screen->getWidget<IconButtonWidget>("about");
        quit = screen->getWidget<IconButtonWidget>("quit");
        user_id = screen->getWidget<ButtonWidget>("user-id");
    }
};

}  // namespace GUIEngine
