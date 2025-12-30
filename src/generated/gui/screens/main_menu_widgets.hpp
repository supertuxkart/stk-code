// Auto-generated from screens/main_menu.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
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

    void bind(AbstractTopLevelContainer* container)
    {
        logo = container->getWidget<IconButtonWidget>("logo");
        menu_toprow = container->getWidget<RibbonWidget>("menu_toprow");
        story = container->getWidget<IconButtonWidget>("story");
        new_ = container->getWidget<IconButtonWidget>("new");
        multiplayer = container->getWidget<IconButtonWidget>("multiplayer");
        online = container->getWidget<IconButtonWidget>("online");
        addons = container->getWidget<IconButtonWidget>("addons");
        info_addons = container->getWidget<LabelWidget>("info_addons");
        menu_bottomrow = container->getWidget<RibbonWidget>("menu_bottomrow");
        test_gpwin = container->getWidget<IconButtonWidget>("test_gpwin");
        test_gplose = container->getWidget<IconButtonWidget>("test_gplose");
        test_unlocked = container->getWidget<IconButtonWidget>("test_unlocked");
        test_unlocked2 = container->getWidget<IconButtonWidget>("test_unlocked2");
        test_intro = container->getWidget<IconButtonWidget>("test_intro");
        test_outro = container->getWidget<IconButtonWidget>("test_outro");
        options = container->getWidget<IconButtonWidget>("options");
        help = container->getWidget<IconButtonWidget>("help");
        startTutorial = container->getWidget<IconButtonWidget>("startTutorial");
        highscores = container->getWidget<IconButtonWidget>("highscores");
        achievements = container->getWidget<IconButtonWidget>("achievements");
        gpEditor = container->getWidget<IconButtonWidget>("gpEditor");
        about = container->getWidget<IconButtonWidget>("about");
        quit = container->getWidget<IconButtonWidget>("quit");
        user_id = container->getWidget<ButtonWidget>("user-id");
    }
};

}  // namespace GUIEngine
